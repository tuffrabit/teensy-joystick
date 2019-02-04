# teensy-joystick

Teensy/Arduino sketch for managing a 2 axis analog joystick. This sketch will auto calibrate the stick's center zero for both X and Y axes. There are currently two commented out functions that need to be un-commented and used for initial setup, and then re-commented or removed for actual regular use. I plan on perhaps replacing that with an auto learning feature using the EEPROM.


# Files

 - joy.ino - Actual sketch code.
 - name.c - Specifies the HID/USB name the device assumes. This is the name the device will appear to the OS and applications as.
