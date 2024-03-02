#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_PCF8574.h>

#include "config.h"

// pins
#define DOOR_PIN D0

// wifi vars (from config.h)
const char* ssid = SSID;
const char* password = WIFI_PASSWORD;
const String serverName = "http://192.168.0.2:5000";
const int port = 5000;

// rfid pad
MFRC522 mfrc522(D8, D4);  // Create MFRC522 instance (SS_Pin und Reset Pin)

// keypad vars
Adafruit_PCF8574 pcf;
char input[5] = "0000";
int inputIndex = 0;
unsigned long keypadInputStartTime = 0;
const int READ_NEW_CARD_DELAY = 3000;


void setup() {
  // setup door pin
  pinMode(DOOR_PIN, OUTPUT);
  digitalWrite(DOOR_PIN, LOW);
  
  // setup serial
  Serial.begin(115200);   // Initialize serial communications with the PC

  // setup wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  WiFi.setOutputPower(19.25);
  WiFi.persistent(true);
  
  // setup rfid
	SPI.begin();			    // Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				      // Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details

  // setup keypad
  if (!pcf.begin(0x20, &Wire)) {
    Serial.println("Couldn't find PCF8574 (i2c extender)");
    while (1);
  }
  pcf.pinMode(0, OUTPUT);
  pcf.pinMode(1, OUTPUT);
  pcf.pinMode(2, OUTPUT);
  pcf.pinMode(3, OUTPUT);
  
  pcf.pinMode(4, INPUT);
  pcf.pinMode(5, INPUT);
  pcf.pinMode(6, INPUT);

}


void loop() {
  unsigned long id = rfidLoop();
  if (getInput()) {
    login(id, input);
  }
}


// waits until a card is ready and then returns uid
unsigned long rfidLoop()
{
  while(!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(10);
  }

  unsigned long hex_num;
  hex_num =  mfrc522.uid.uidByte[0] << 24;
  hex_num += mfrc522.uid.uidByte[1] << 16;
  hex_num += mfrc522.uid.uidByte[2] <<  8;
  hex_num += mfrc522.uid.uidByte[3];

  mfrc522.PICC_HaltA(); // Stop reading

  Serial.print("successfully read card with id: ");
  Serial.println(hex_num);
  beep(1);
  
  return hex_num;
}


void login(unsigned int id, char pin[]) {
  String serverPath = serverName + "/login?id=" + id + "&pin=" + pin;

  // check if wifi still connected
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("WiFi Disconnected");
    beep(4);
    return;
  }

  WiFiClient client;
  HTTPClient http;

  // try to connect to http server
  if(!http.begin(client, serverPath.c_str())) {
    Serial.println("[HTTP] Unable to connect");
    http.end();
    beep(5);
    return;
  }
    
  // Send HTTP GET request
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    Serial.print("access granted, response code: ");
    Serial.println(httpResponseCode);
    openDoor();
    beep(1);
  } else if (httpResponseCode > 0) {
    Serial.print("access NOT granted, response code: ");
    Serial.println(httpResponseCode);
    beep(2);
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
    ESP.reset();
    beep(3);
  }
  // Free resources
  http.end();
}


void openDoor() {
  // open the door here
  Serial.println("door opening");
  digitalWrite(DOOR_PIN, HIGH);
  delay(100);
  digitalWrite(DOOR_PIN, LOW);

}


void beep(int id) {
  // place holder for beeping
  // 1 (ok) 2 (wrong login data) 3 (problem with wifi)
  Serial.print("beep ");
  Serial.println(id);
}


bool getInput() {
  bool wasKeyPressed = false;
  inputIndex = 0;
  keypadInputStartTime = millis();

  //reset input
  for(int i=0; i<4; i++) {
    input[i] = '0';
  }


  bool running = true;
  while (millis() - keypadInputStartTime < READ_NEW_CARD_DELAY || !mfrc522.PICC_IsNewCardPresent()) {
    char c = getPressedChar();
    if(c == 0) {
      wasKeyPressed = false;
    } else if (!wasKeyPressed) {
      wasKeyPressed = true;

      Serial.print("keypad eingabe: ");
      Serial.println(c);

      if(c == '#' || c == '*') {
        Serial.print("Code eingabe abgeschlossen: ");
        Serial.println(input);
        return true;
      } else {
        beep(1);
      }

      if (inputIndex < 4) {
        input[inputIndex] = c;
        inputIndex++;
      }
    }

    //  entprellen
    delay(10);
  }
  return false;
}


char getPressedChar() {
  char keys[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
  };

  bool pressed = false;
  int row = 0;
  int col = 0;
  for(int y=0; y<4; y++) {
    // set all rows to high
    for(int i=0; i<4; i++) {
      pcf.digitalWrite(i, HIGH);
    }
    // set y row to low
    pcf.digitalWrite(y, LOW);

    for(int x=0; x<3; x++) {
      if (!pcf.digitalRead(x+4)) {
        pressed = true;
        col = x;
        row = y;
        break;
      }
    }
  }
  if(pressed) {
    return keys[row][col];
  }
  return 0;
}
