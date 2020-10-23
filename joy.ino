#include <Bounce.h>
#include <EEPROM.h>

/**
   Start - PIN defines
*/
/** Analog pin # for the joystick X axis */
#define STICK_X 9

/** Analog pin # for the joystick Y axis */
#define STICK_Y 8

/** Digital pin # for the joystick button */
#define JOYSTICK_1_BUTTON_PIN 0

/** Pin # for the extra signal LED. Set to a valid pin # if you have an extra LED wired to said pin. (HINT: -1 isn't a valid pin #) */
#define ALT_LED_PIN -1
/**
   Stop - PIN defines
*/

/**
  Start - Binding defines
*/
#define BUTTON_JOYSTICK_1_KEY KEY_LEFT_SHIFT
#define KEYBOARD_MODE_STICK_UP_KEY KEY_W
#define KEYBOARD_MODE_STICK_DOWN_KEY KEY_S
#define KEYBOARD_MODE_STICK_LEFT_KEY KEY_A
#define KEYBOARD_MODE_STICK_RIGHT_KEY KEY_D
/**
  Stop - Binding defines
*/

/**
  Start - Keyboard mode defines
*/
#define KEYBOARD_MODE_X_START_OFFSET 250
#define KEYBOARD_MODE_Y_START_OFFSET 250
/**
  Stop - Keyboard mode defines
*/

short Xstick;
short Ystick;
short deadzone;
short edgeAdjust;
short upperBound;
short lowerBound;
short xLow;
short xHigh;
short yLow;
short yHigh;
bool isKeyboardMode;

// up, down, left, right
bool keyboardModeKeyStatus[4] = {false, false, false, false};
Bounce joystickButton1 = Bounce(JOYSTICK_1_BUTTON_PIN, 10);

void setup() {
  Joystick.useManualSend(true);
  pinMode(13, OUTPUT);

  if (ALT_LED_PIN > 0) {
    pinMode(ALT_LED_PIN, OUTPUT);
  }

  setDeadzone();
  setBounds();
  pinMode(JOYSTICK_1_BUTTON_PIN, INPUT_PULLUP);

  Xstick = 512;
  Ystick = 512;

  isKeyboardMode = false;
  detectStartupFlags();

  /**
     Uncomment these two function when troubleshooting or setting up the teensy + joystick. Have a text
     editor open and plug in/power on the device. It will do some calcuations and print out information.
     Once you see it print out the first set of data, immediately begin rotating the stick making sure to
     hit the max outer bounds on each rotation. Once you see the second chunk of data printed, you may stop
     the rotations. Use the printed data to compare to what's been stored in EEPROM. Once that is complete you should
     comment these functions back out so they do not execute.
  */
  //outputInitialState();
  //doMinMaxAccumulationAndOutput();

  Xstick = 512;
  Ystick = 512;
}

void setLedState(int state) {
  digitalWrite(13, state);

  if (ALT_LED_PIN > 0) {
    digitalWrite(ALT_LED_PIN, state);
  }
}

void doStickCalculations(bool constrainDeadzone = false) {
  Xstick = analogRead(STICK_X);
  Ystick = analogRead(STICK_Y);

  if (constrainDeadzone) {
    if (isInsideDeadzone(Xstick)) {
      Xstick = 512;
    } else {
      if (Xstick > 512) {
        Xstick = constrain(map((Xstick - deadzone), 513, (xHigh - edgeAdjust), 513, 1023), 513, 1023);
      } else if (Xstick < 512) {
        Xstick = constrain(map((Xstick + deadzone), (xLow + edgeAdjust), 511, 0, 511), 0, 511);
      }
    }

    if (isInsideDeadzone(Ystick)) {
      Ystick = 512;
    } else {
      if (Ystick > 512) {
        Ystick = constrain(map((Ystick - deadzone), 513, (yHigh - edgeAdjust), 513, 1023), 513, 1023);
      } else if (Ystick < 512) {
        Ystick = constrain(map((Ystick + deadzone), (yLow + edgeAdjust), 511, 0, 511), 0, 511);
      }
    }
  }
}

void loop() {
  doStickCalculations(true);
  joystickButton1.update();

  if (isKeyboardMode) {
    // up, down, left, right, modifier
    bool keyboardModeKeyPress[5] = {false, false, false, false, false};

    if (Xstick > (512 + KEYBOARD_MODE_X_START_OFFSET)) {
      keyboardModeKeyPress[3] = true;
    } else if (Xstick < (512 - KEYBOARD_MODE_X_START_OFFSET)) {
      keyboardModeKeyPress[2] = true;
    }

    if (Ystick < (512 - KEYBOARD_MODE_Y_START_OFFSET)) {
      keyboardModeKeyPress[0] = true;
    } else if (Ystick > (512 + KEYBOARD_MODE_Y_START_OFFSET)) {
      keyboardModeKeyPress[1] = true;
    }

    handleKeyboundModeKey(KEYBOARD_MODE_STICK_UP_KEY, keyboardModeKeyPress[0]);
    handleKeyboundModeKey(KEYBOARD_MODE_STICK_DOWN_KEY, keyboardModeKeyPress[1]);
    handleKeyboundModeKey(KEYBOARD_MODE_STICK_LEFT_KEY, keyboardModeKeyPress[2]);
    handleKeyboundModeKey(KEYBOARD_MODE_STICK_RIGHT_KEY, keyboardModeKeyPress[3]);

    if (joystickButton1.fallingEdge()) {
      Keyboard.press(BUTTON_JOYSTICK_1_KEY);
    }

    if (joystickButton1.risingEdge()) {
      Keyboard.release(BUTTON_JOYSTICK_1_KEY);
    }
  } else {
    Joystick.X(Xstick);
    Joystick.Y(Ystick);

    if (joystickButton1.fallingEdge()) {
      Joystick.button(1, 1);
    }

    if (joystickButton1.risingEdge()) {
      Joystick.button(1, 0);
    }

    Joystick.Z(512);
    Joystick.Zrotate(512);
    Joystick.sliderLeft(0);
    Joystick.sliderRight(0);
    Joystick.hat(-1);
    Joystick.send_now();
  }
}

void handleKeyboundModeKey(int key, bool isPress) {
  int keyIndex = -1;

  switch (key) {
    case KEYBOARD_MODE_STICK_UP_KEY:
      keyIndex = 0;
      break;
    case KEYBOARD_MODE_STICK_DOWN_KEY:
      keyIndex = 1;
      break;
    case KEYBOARD_MODE_STICK_LEFT_KEY:
      keyIndex = 2;
      break;
    case KEYBOARD_MODE_STICK_RIGHT_KEY:
      keyIndex = 3;
      break;
  }

  if (keyIndex > -1) {
    if (isPress) {
      if (keyboardModeKeyStatus[keyIndex] == false) {
        Keyboard.press(key);
        keyboardModeKeyStatus[keyIndex] = true;
      }
    } else {
      if (keyboardModeKeyStatus[keyIndex] == true) {
        Keyboard.release(key);
        keyboardModeKeyStatus[keyIndex] = false;
      }
    }
  }
}

bool isInsideDeadzone(int rawStickValue) {
  bool returnValue = false;

  if ((rawStickValue > 512 && rawStickValue <= upperBound) ||
      (rawStickValue < 512 && rawStickValue >= lowerBound)
     ) {
    returnValue = true;
  }

  return returnValue;
}

int getDeadzoneAdjustedValue(int value) {
  if (value > 512) {
    value = value - deadzone;
  }

  if (value < 512) {
    value = value + deadzone;
  }

  return value;
}

void setDeadzone() {
  setLedState(HIGH);
  deadzone = 0;

  unsigned long startTime = millis();
  unsigned long highValue = 0;

  while ((millis() - startTime) < 5000) {
    doStickCalculations();

    int localXstick = abs(Xstick);
    int localYstick = abs(Ystick);
    int diffX = 0;
    int diffY = 0;

    if (localXstick < 512) {
      diffX = 512 - localXstick;
    }

    if (localXstick > 512) {
      diffX = localXstick - 512;
    }

    if (localYstick < 512) {
      diffY = 512 - localYstick;
    }

    if (localYstick > 512) {
      diffY = localYstick - 512;
    }

    if (diffX >= diffY) {
      highValue = diffX;
    } else {
      highValue = diffY;
    }
  }

  deadzone = highValue;
}

void setBounds() {
  deadzone = deadzone + 7;
  edgeAdjust = deadzone + 15;
  upperBound = 512 + deadzone;
  lowerBound = 512 - deadzone;
  xLow = getLowestXFromEEPROM();
  xHigh = getHighestXFromEEPROM();
  yLow = getHighestYFromEEPROM();
  yHigh = getLowestYFromEEPROM();

  if (xLow == -1) {
    xLow = 250;
  }

  if (xHigh == -1) {
    xHigh = 850;
  }

  if (yLow == -1) {
    yLow = 250;
  } else {
    yLow = 1023 - yLow;
  }

  if (yHigh == -1) {
    yHigh = 850;
  } else {
    yHigh = 1023 - yHigh;
  }
  
  setLedState(LOW);
}

void detectStartupFlags() {
  unsigned long startTime = millis();
  unsigned long lastBlink = 0;
  int ledState = LOW;

  while ((millis() - startTime) < 5000) {
    unsigned long now = millis();

    joystickButton1.update();

    if (now - lastBlink >= 500) {
      lastBlink = now;

      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }

      setLedState(ledState);
    }

    if (joystickButton1.fallingEdge()) {
      setLedState(HIGH);
      isKeyboardMode = true;

      break;
    }

    doStickCalculations();

    if (Xstick > 700) {
      saveBoundsToEEPROM();
      break;
    }

    if (Xstick < 324) {
      clearBoundsFromEEPROM();
      break;
    }
  }

  if (!isKeyboardMode) {
    setLedState(LOW);
  }
}

void updateBoundsToEEPROM(short lowestX, short highestX, short lowestY, short highestY) {
  EEPROM.update(0, highByte(lowestX));
  EEPROM.update(1, lowByte(lowestX));
  EEPROM.update(2, highByte(highestX));
  EEPROM.update(3, lowByte(highestX));
  EEPROM.update(4, highByte(lowestY));
  EEPROM.update(5, lowByte(lowestY));
  EEPROM.update(6, highByte(highestY));
  EEPROM.update(7, lowByte(highestY));
}

short getLowestXFromEEPROM() {
  return EEPROM.read(0) << 8 | EEPROM.read(1);
}

short getHighestXFromEEPROM() {
  return EEPROM.read(2) << 8 | EEPROM.read(3);
}

short getLowestYFromEEPROM() {
  return EEPROM.read(4) << 8 | EEPROM.read(5);
}

short getHighestYFromEEPROM() {
  return EEPROM.read(6) << 8 | EEPROM.read(7);
}

void saveBoundsToEEPROM() {
  setLedState(HIGH);
  
  unsigned long startTime = millis();
  short highestX = 512;
  short lowestX = 512;
  short highestY = 512;
  short lowestY = 512;

  while ((millis() - startTime) < 5000) {
    doStickCalculations();

    if (Xstick < lowestX) {
      lowestX = Xstick;
    }

    if (Xstick > highestX) {
      highestX = Xstick;
    }

    if (Ystick < lowestY) {
      lowestY = Ystick;
    }

    if (Ystick > highestY) {
      highestY = Ystick;
    }
  }

  updateBoundsToEEPROM(lowestX, highestX, lowestY, highestY);
  setLedState(LOW);
}

void clearBoundsFromEEPROM() {
  setLedState(HIGH);
  updateBoundsToEEPROM(-1, -1, -1, -1);
  setLedState(LOW);
}

void outputInitialState() {
  doStickCalculations();

  Keyboard.print(F("Deadzone: "));
  Keyboard.println(deadzone);
  Keyboard.print(F("Upper Bound: "));
  Keyboard.println(upperBound);
  Keyboard.print(F("Lower Bound: "));
  Keyboard.println(lowerBound);
  Keyboard.print(F("X Rest: "));
  Keyboard.println(Xstick);
  Keyboard.print(F("Y Rest: "));
  Keyboard.println(Ystick);

  if (isInsideDeadzone(Xstick)) {
    Xstick = 512;
  }

  if (isInsideDeadzone(Ystick)) {
    Ystick = 512;
  }

  Keyboard.print(F("Adjusted X Rest: "));
  Keyboard.println(Xstick);
  Keyboard.print(F("Adjusted Y Rest: "));
  Keyboard.println(Ystick);
  Keyboard.print(F("EEPROM Lowest X: "));
  Keyboard.println(getLowestXFromEEPROM());
  Keyboard.print(F("EEPROM Highest X: "));
  Keyboard.println(getHighestXFromEEPROM());
  Keyboard.print(F("EEPROM Lowest Y: "));
  Keyboard.println(getLowestYFromEEPROM());
  Keyboard.print(F("EEPROM Highest Y: "));
  Keyboard.println(getHighestYFromEEPROM());
}

void doMinMaxAccumulationAndOutput() {
  unsigned long startTime = millis();
  int highestX = 512;
  int lowestX = 512;
  int highestY = 512;
  int lowestY = 512;

  while ((millis() - startTime) < 5000) {
    doStickCalculations();

    if (Xstick < lowestX) {
      lowestX = Xstick;
    }

    if (Xstick > highestX) {
      highestX = Xstick;
    }

    if (Ystick < lowestY) {
      lowestY = Ystick;
    }

    if (Ystick > highestY) {
      highestY = Ystick;
    }
  }

  Keyboard.print(F("Lowest X: "));
  Keyboard.println(lowestX);
  Keyboard.print(F("Highest X: "));
  Keyboard.println(highestX);
  Keyboard.print(F("Lowest Y: "));
  Keyboard.println(lowestY);
  Keyboard.print(F("Highest Y: "));
  Keyboard.println(highestY);
  Keyboard.println(F(""));
}
