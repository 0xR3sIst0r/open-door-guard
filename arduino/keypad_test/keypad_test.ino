#include <Adafruit_PCF8574.h>

// pins
#define DOOR_PIN D0

// keypad vars
Adafruit_PCF8574 pcf;
char input[5] = "0000";
int inputIndex = 0;
unsigned long keypadInputStartTime = 0;
const int READ_NEW_CARD_DELAY = 3000;

const int i = 7;

void setup() {
  // setup serial
  Serial.begin(115200);   // Initialize serial communications with the PC
  // setup keypad
  if (!pcf.begin(0x20, &Wire)) {
    Serial.println("Couldn't find PCF8574 (i2c extender)");
    while (1);
  }

  pcf.pinMode(i, OUTPUT);
  pcf.digitalWrite(i, LOW);
  

}


void loop() {
  //getInput();
  pcf.digitalWrite(i, LOW);
  Serial.println("loop");
  delay(1000);
  pcf.digitalWrite(i, HIGH);
  delay(1000);
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
