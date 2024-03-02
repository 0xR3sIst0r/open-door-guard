# web interface

show overview of all users in the system
add new user
delete user

# todo

- arduino introduce exit routine
- put on raspberry pi
- add sound

# Raspberry Pi setup

- install raspberry pi os
- apt update + upgrade
- setup ssh
- setup service (/etc/systemd/system/doorlock.service)
- enable & start service

# Router setup

- setup router (strong password)
- set raspberry pi ip as static (192.168.0.2)

# Arduino setup

- change wifi name and password in code
- connect all the stuff

## Schematics

keyboard: (r1, r2, r3, r4, c1, c2, c3)

d5 - r1
d6 - r2
d7 - r3
d8 - r4
d1 - c1
d2 - c2
d3 - c3


d5 - SDA
d6 - SCK
d7 - MOSI
d8 - MISO
d0 - RST
3.3v - 3.3v
GND - GND
