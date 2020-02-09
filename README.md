
# teensy-joystick

Teensy/Arduino sketch for managing a 2 axis analog joystick. This sketch will auto calibrate the stick's center zero for both X and Y axes, and will also store axes outer bounds values.


# Files

 - joy.ino - Actual sketch code.
 - name.c - Specifies the HID/USB name the device assumes. This is the name the device will appear to the OS and applications as.

# Boot up sequence

When the Teensy is first powered on it goes through a 10 second boot process with different LED indicators. The first 5 seconds of that boot process the orange LED will be solid, that is the center auto calibration. DO NOT TOUCH THE STICK DURING THIS PERIOD. Just let the stick rest in the center where it wants to. The next 5 seconds the orange LED will blink. That is the mode selection window. Pressing the stick in (button click) or moving up or down will put the stick into one of the three digital/keyboard modes. Moving the stick to the right will put the stick into outer bounds calibration mode. Moving the stick to the left will delete the saved EEPROM outbounds values. The outer bounds calibration mode is simple. You just peg the stick as far as it will go in full slow steady circles. Those values are saved to EEPROM and then retrieved on each subsequent boot.
