#include <Bounce.h>

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
#define KEYBOARD_MODE_MODIFIER_KEY KEY_LEFT_SHIFT

/**
  Stop - Binding defines
*/

/**
   High and low values for both of the joystick axes. Get these values from the output of the
   doMinMaxAccumulationAndOutput() function.
*/
#define X_FROM_LOW 281
#define X_FROM_HIGH 765
#define Y_FROM_LOW 263
#define Y_FROM_HIGH 744

#define X_LOW_LIMIT 175
#define X_HIGH_LIMIT 848
#define Y_LOW_LIMIT 150
#define Y_HIGH_LIMIT 838

/**
  Start - Keyboard mode defines
*/

#define KEYBOARD_MODE_X_START_OFFSET 15
#define KEYBOARD_MODE_Y_START_OFFSET 15
#define KEYBOARD_MODE_X_MODIFIER_SCALE .6
#define KEYBOARD_MODE_Y_MODIFIER_SCALE .6

/**
  Stop - Keyboard mode defines
*/

int Xstick;
int Ystick;
int deadzone;
int upperBound;
int lowerBound;
bool isKeyboardMode;
bool isHoldToWalk;
bool isHoldToRun;
int keyboardModeXUpperModifierOffset;
int keyboardModeXLowerModifierOffset;
int keyboardModeYUpperModifierOffset;
int keyboardModeYLowerModifierOffset;

// up, down, left, right, modifier
bool keyboardModeKeyStatus[5] = {false, false, false, false, false};
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
  isHoldToWalk = false;
  isHoldToRun = false;
  detectStartupFlags();
  calculateKeyboardModeOffsets();

  /**
     Uncomment these two function when initially setting up the teensy + joystick. Have a text
     editor open and plug in/power on the device. It will do some calcuations and print out information.
     Once you see it print out the first set of data, immediately begin rotating the stick making sure to
     hit the max outer bounds on each rotation. Once you see the second chunk of data printed, you may stop
     the rotations. Use the printed data to set the #define values above. Once that is complete you should
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
        Xstick = constrain(map((Xstick - deadzone), 513, X_FROM_HIGH, 513, 1023), 513, 1023);

        if (Xstick > X_HIGH_LIMIT) {
          Xstick = 1023;
        }
      } else if (Xstick < 512) {
        Xstick = constrain(map((Xstick + deadzone), X_FROM_LOW, 511, 0, 511), 0, 511);

        if (Xstick < X_LOW_LIMIT) {
          Xstick = 0;
        }
      }
    }

    if (isInsideDeadzone(Ystick)) {
      Ystick = 512;
    } else {
      if (Ystick > 512) {
        Ystick = constrain(map((Ystick - deadzone), 513, Y_FROM_HIGH, 513, 1023), 513, 1023);

        if (Ystick > Y_HIGH_LIMIT) {
          Ystick = 1023;
        }
      } else if (Ystick < 512) {
        Ystick = constrain(map((Ystick + deadzone), Y_FROM_LOW, 511, 0, 511), 0, 511);

        if (Ystick < Y_LOW_LIMIT) {
          Ystick = 0;
        }
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

      if (Xstick < (512 + keyboardModeXUpperModifierOffset)) {
        if (isHoldToWalk) {
          keyboardModeKeyPress[4] = true;
        }
      } else {
        if (isHoldToRun) {
          keyboardModeKeyPress[4] = true;
        }
      }
    } else if (Xstick < (512 - KEYBOARD_MODE_X_START_OFFSET)) {
      keyboardModeKeyPress[2] = true;

      if (Xstick > (512 - keyboardModeXLowerModifierOffset)) {
        if (isHoldToWalk) {
          keyboardModeKeyPress[4] = true;
        }
      } else {
        if (isHoldToRun) {
          keyboardModeKeyPress[4] = true;
        }
      }
    }

    if (Ystick < (512 - KEYBOARD_MODE_Y_START_OFFSET)) {
      keyboardModeKeyPress[0] = true;

      if (Ystick > (512 - keyboardModeYLowerModifierOffset)) {
        if (isHoldToWalk) {
          keyboardModeKeyPress[4] = true;
        }
      } else {
        if (isHoldToRun) {
          keyboardModeKeyPress[4] = true;
        }
      }
    } else if (Ystick > (512 + KEYBOARD_MODE_Y_START_OFFSET)) {
      keyboardModeKeyPress[1] = true;

      if (Ystick < (512 + keyboardModeYUpperModifierOffset)) {
        if (isHoldToWalk) {
          keyboardModeKeyPress[4] = true;
        }
      } else {
        if (isHoldToRun) {
          keyboardModeKeyPress[4] = true;
        }
      }
    }

    handleKeyboundModeKey(KEYBOARD_MODE_STICK_UP_KEY, keyboardModeKeyPress[0]);
    handleKeyboundModeKey(KEYBOARD_MODE_STICK_DOWN_KEY, keyboardModeKeyPress[1]);
    handleKeyboundModeKey(KEYBOARD_MODE_STICK_LEFT_KEY, keyboardModeKeyPress[2]);
    handleKeyboundModeKey(KEYBOARD_MODE_STICK_RIGHT_KEY, keyboardModeKeyPress[3]);
    handleKeyboundModeKey(KEYBOARD_MODE_MODIFIER_KEY, keyboardModeKeyPress[4]);

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
    case KEYBOARD_MODE_MODIFIER_KEY:
      keyIndex = 4;
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
  upperBound = 512 + deadzone;
  lowerBound = 512 - deadzone;

  setLedState(LOW);
}

void detectStartupFlags() {
  unsigned long startTime = millis();
  unsigned long lastBlink = 0;
  int ledState = LOW;
  int yUpperBounds = ((Y_FROM_HIGH - 512) * .8) + 512;
  int yLowerBounds = 512 - ((512 - Y_FROM_LOW) * .8);

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
      isHoldToWalk = false;
      isHoldToRun = false;

      break;
    }

    doStickCalculations();

    if (Ystick > yUpperBounds) {
      setLedState(HIGH);
      isKeyboardMode = true;
      isHoldToWalk = false;
      isHoldToRun = true;

      break;
    }

    if (Ystick < yLowerBounds) {
      setLedState(HIGH);
      isKeyboardMode = true;
      isHoldToWalk = true;
      isHoldToRun = false;

      break;
    }
  }

  if (!isKeyboardMode) {
    setLedState(LOW);
  }
}

void calculateKeyboardModeOffsets() {
  keyboardModeXUpperModifierOffset = abs((map(X_FROM_HIGH, 513, X_FROM_HIGH, 513, 1023) - (513 + KEYBOARD_MODE_X_START_OFFSET)) * KEYBOARD_MODE_X_MODIFIER_SCALE);
  keyboardModeXLowerModifierOffset = abs((map(X_FROM_LOW, X_FROM_LOW, 511, 0, 511) - (511 - KEYBOARD_MODE_X_START_OFFSET)) * KEYBOARD_MODE_X_MODIFIER_SCALE);
  keyboardModeYUpperModifierOffset = abs((map(Y_FROM_HIGH, 513, Y_FROM_HIGH, 513, 1023) - (513 + KEYBOARD_MODE_Y_START_OFFSET)) * KEYBOARD_MODE_Y_MODIFIER_SCALE);
  keyboardModeYLowerModifierOffset = abs((map(Y_FROM_LOW, Y_FROM_LOW, 511, 0, 511) - (511 - KEYBOARD_MODE_Y_START_OFFSET)) * KEYBOARD_MODE_Y_MODIFIER_SCALE);
}

void outputInitialState() {
  doStickCalculations();

  Keyboard.print("Deadzone: ");
  Keyboard.println(deadzone);
  Keyboard.print("Upper Bound: ");
  Keyboard.println(upperBound);
  Keyboard.print("Lower Bound: ");
  Keyboard.println(lowerBound);
  Keyboard.print("X Rest: ");
  Keyboard.println(Xstick);
  Keyboard.print("Y Rest: ");
  Keyboard.println(Ystick);

  if (isInsideDeadzone(Xstick)) {
    Xstick = 512;
  }

  if (isInsideDeadzone(Ystick)) {
    Ystick = 512;
  }

  Keyboard.print("Adjusted X Rest: ");
  Keyboard.println(Xstick);
  Keyboard.print("Adjusted Y Rest: ");
  Keyboard.println(Ystick);
  Keyboard.print("Keyboard Mode X Upper Modifier Offset: ");
  Keyboard.println(keyboardModeXUpperModifierOffset);
  Keyboard.print("Keyboard Mode X Lower Modifier Offset: ");
  Keyboard.println(keyboardModeXLowerModifierOffset);
  Keyboard.print("Keyboard Mode Y Upper Modifier Offset: ");
  Keyboard.println(keyboardModeYUpperModifierOffset);
  Keyboard.print("Keyboard Mode Y Lower Modifier Offset: ");
  Keyboard.println(keyboardModeYLowerModifierOffset);
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

  Keyboard.print("Lowest X: ");
  Keyboard.println(lowestX);
  Keyboard.print("Highest X: ");
  Keyboard.println(highestX);
  Keyboard.print("Lowest Y: ");
  Keyboard.println(lowestY);
  Keyboard.print("Highest Y: ");
  Keyboard.println(highestY);
}