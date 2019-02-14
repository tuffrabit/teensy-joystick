/** Analog pin # for the joystick X axis */
#define STICK_X 9

/** Analog pin # for the joystick Y axis */
#define STICK_Y 8

/** Digital pin # for the joystick button */
#define BUTTON_PIN 0

/**
 * High and low values for both of the joystick axes. Get these values from the output of the
 * doMinMaxAccumulationAndOutput() function.
*/
#define X_FROM_LOW 289
#define X_FROM_HIGH 750
#define Y_FROM_LOW 245
#define Y_FROM_HIGH 720

int deadzone;
int upperBound;
int lowerBound;
bool isKeyboardMode;
bool lastKeyboardModeKeys[5] = {false, false, false, false, false}; // up, down, left, right, select

void setup() {
  Joystick.useManualSend(true);
  pinMode(13, OUTPUT);
  setDeadzone();
  setBounds();
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  /**
   * Uncomment these two function when initially setting up the teensy + joystick. Have a text
   * editor open and plug in/power on the device. It will do some calcuations and print out information.
   * Once you see it print out the first set of data, immediately begin rotating the stick making sure to
   * hit the max outer bounds on each rotation. Once you see the second chunk of data printed, you may stop
   * the rotations. Use the printed data to set the #define values above. Once that is complete you should
   * comment these functions back out so they do not execute.
   */
  //outputInitialState();
  //doMinMaxAccumulationAndOutput();

  isKeyboardMode = false;
  detectStartupFlags();
}

void loop() {
  int Xstick = analogRead(STICK_X);
  int Ystick = analogRead(STICK_Y);
    
  if (isInsideDeadzone(Xstick)) {
    Xstick = 512;
  } else {
    Xstick = constrain(map(Xstick, X_FROM_LOW, X_FROM_HIGH, 0, 1023), 0, 1023);
  }
  
  if (isInsideDeadzone(Ystick)) {
    Ystick = 512;
  } else {
    Ystick = constrain(map(Ystick, Y_FROM_LOW, Y_FROM_HIGH, 0, 1023), 0, 1023);
  }

  if (isKeyboardMode) {
    bool keyboardModeKeys[5] = {false, false, false, false, false}; // up, down, left, right, select
    
    if (Xstick < 612 && Xstick > 412) {
      keyboardModeKeys[2] = false;
      keyboardModeKeys[3] = false;
    } else if (Xstick > 612) {
      keyboardModeKeys[2] = false;
      keyboardModeKeys[3] = true;
    } else {
      keyboardModeKeys[2] = true;
      keyboardModeKeys[3] = false;
    }

    if (Ystick < 612 && Ystick > 412) {
      keyboardModeKeys[0] = false;
      keyboardModeKeys[1] = false;
    } else if (Ystick < 412) {
      keyboardModeKeys[0] = true;
      keyboardModeKeys[1] = false;
    } else {
      keyboardModeKeys[0] = false;
      keyboardModeKeys[1] = true;
    }

    if (!digitalRead(BUTTON_PIN)) {
      keyboardModeKeys[5] = true;
    } else {
      keyboardModeKeys[5] = false;
    }

    bool isDifferent = false;

    for (int i = 0; i < 5; i++) {
      bool last = lastKeyboardModeKeys[i];
      bool current = keyboardModeKeys[i];

      if (current != last) {
        lastKeyboardModeKeys[i] = current;
        isDifferent = true;

        switch (i) {
          case 0:
            if (current) {
              Keyboard.set_key1(KEY_W);
            } else {
              Keyboard.set_key1(0);
            }

            break;
          case 1:
            if (current) {
              Keyboard.set_key2(KEY_S);
            } else {
              Keyboard.set_key2(0);
            }

            break;
          case 2:
            if (current) {
              Keyboard.set_key3(KEY_A);
            } else {
              Keyboard.set_key3(0);
            }

            break;
          case 3:
            if (current) {
              Keyboard.set_key4(KEY_D);
            } else {
              Keyboard.set_key4(0);
            }

            break;
          case 4:
            if (current) {
              Keyboard.set_key5(KEY_P);
            } else {
              Keyboard.set_key5(0);
            }

            break;
        }
      }
    }

    if (isDifferent) {
      Keyboard.send_now();
    }
  } else {
    Joystick.X(Xstick);
    Joystick.Y(Ystick);
    Joystick.button(1, !digitalRead(BUTTON_PIN));
    Joystick.Z(512);
    Joystick.Zrotate(512);
    Joystick.sliderLeft(0);
    Joystick.sliderRight(0);
    Joystick.hat(-1);
    Joystick.send_now();
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

void setDeadzone() {
  digitalWrite(13, HIGH);
  deadzone = 0;
  
  unsigned long startTime = millis();
  unsigned long highValue = 0;

  while ((millis() - startTime) < 5000) {
    int Xstick = abs(analogRead(STICK_X));
    int Ystick = abs(analogRead(STICK_Y));
    int diffX = 0;
    int diffY = 0;

    if (Xstick < 512) {
      diffX = 512 - Xstick;
    }

    if (Xstick > 512) {
      diffX = Xstick - 512;
    }

    if (Ystick < 512) {
      diffY = 512 - Ystick;
    }

    if (Ystick > 512) {
      diffY = Ystick - 512;
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

  digitalWrite(13, LOW);
}

void detectStartupFlags() {
  unsigned long startTime = millis();

  while ((millis() - startTime) < 5000) {
    if (!digitalRead(BUTTON_PIN)) {
      digitalWrite(13, HIGH);
      isKeyboardMode = true;
      
      break;
    }
  }
}

void outputInitialState() {
  int Xstick = analogRead(STICK_X);
  int Ystick = analogRead(STICK_Y);
  
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
}

void doMinMaxAccumulationAndOutput() {
  unsigned long startTime = millis();
  int highestX = 512;
  int lowestX = 512;
  int highestY = 512;
  int lowestY = 512;

  while ((millis() - startTime) < 5000) {
    int Xstick = analogRead(STICK_X);
    int Ystick = analogRead(STICK_Y);
    
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

  Keyboard.print("Highest X: ");
  Keyboard.println(highestX);
  Keyboard.print("Lowest X: ");
  Keyboard.println(lowestX);
  Keyboard.print("Highest Y: ");
  Keyboard.println(highestY);
  Keyboard.print("Lowest Y: ");
  Keyboard.println(lowestY);
}