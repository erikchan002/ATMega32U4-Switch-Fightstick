#include "ButtonMatrix.h"

#include "LED.h"
#include "USB.h"

bool buttons[NUMBER_OF_BUTTONS] = {false};
const uint8_t keymap[NUMBER_OF_ROWS][NUMBER_OF_COLUMNS] = {
    {BUTTON_UP, BUTTON_L_STICK, BUTTON_LED, BUTTON_R_STICK, BUTTON_X},
    {BUTTON_LEFT, BUTTON_L, BUTTON_SLIDER_LED, BUTTON_R, BUTTON_A},
    {BUTTON_DOWN, BUTTON_ZL, BUTTON_HOME, BUTTON_ZR, BUTTON_Y},
    {BUTTON_RIGHT, BUTTON_MINUS, BUTTON_CAPTURE, BUTTON_PLUS, BUTTON_B},
    {BUTTON_TRIANGLE, BUTTON_SQUARE, BUTTON_NONE, BUTTON_CROSS, BUTTON_CIRCLE}};

const uint8_t rowPins[NUMBER_OF_ROWS] = {ROW_0_PIN, ROW_1_PIN, ROW_2_PIN,
                                         ROW_3_PIN, ROW_4_PIN};
const uint8_t colPins[NUMBER_OF_COLUMNS] = {COL_0_PIN, COL_1_PIN, COL_2_PIN,
                                            COL_3_PIN, COL_4_PIN};
Keypad matrix = Keypad(makeKeymap(keymap), rowPins, colPins, NUMBER_OF_ROWS,
                       NUMBER_OF_COLUMNS);

void setupMatrix() {
  matrix.setHoldTime(HOLD_TIME);
  matrix.setDebounceTime(DEBOUNCE_INTERVAL);
}
void readMatrix() {
  if (matrix.getKeys()) {
    for (int i = 0; i < LIST_MAX; ++i) {
      if (matrix.key[i].stateChanged && matrix.key[i].kchar != BUTTON_NONE) {
        switch (matrix.key[i].kstate) {
          case PRESSED:
          case HOLD:
            buttons[matrix.key[i].kchar] = true;
            break;
          case RELEASED:
          case IDLE:
          default:
            buttons[matrix.key[i].kchar] = false;
            break;
        }
      }
    }
  }
}
void writeMatrixToReport() {
  ReportData.HAT = DPAD_NONE_MASK;

  if (!buttons[BUTTON_LED] && !buttons[BUTTON_SLIDER_LED]) {
    if (buttons[BUTTON_RIGHT]) ReportData.HAT = DPAD_RIGHT_MASK;
    if (buttons[BUTTON_DOWN]) ReportData.HAT = DPAD_DOWN_MASK;
    if (buttons[BUTTON_LEFT]) ReportData.HAT = DPAD_LEFT_MASK;
    if (buttons[BUTTON_UP]) ReportData.HAT = DPAD_UP_MASK;
    if (buttons[BUTTON_DOWN] && buttons[BUTTON_RIGHT])
      ReportData.HAT = DPAD_DOWNRIGHT_MASK;
    if (buttons[BUTTON_DOWN] && buttons[BUTTON_LEFT])
      ReportData.HAT = DPAD_DOWNLEFT_MASK;
    if (buttons[BUTTON_UP] && buttons[BUTTON_RIGHT])
      ReportData.HAT = DPAD_UPRIGHT_MASK;
    if (buttons[BUTTON_UP] && buttons[BUTTON_LEFT])
      ReportData.HAT = DPAD_UPLEFT_MASK;

    if (buttons[BUTTON_A]) ReportData.Button |= A_MASK;
    if (buttons[BUTTON_B]) ReportData.Button |= B_MASK;
    if (buttons[BUTTON_X]) ReportData.Button |= X_MASK;
    if (buttons[BUTTON_Y]) ReportData.Button |= Y_MASK;

    if (buttons[BUTTON_L]) ReportData.Button |= L_MASK;
    if (buttons[BUTTON_R]) ReportData.Button |= R_MASK;
    if (buttons[BUTTON_ZL]) ReportData.Button |= ZL_MASK;
    if (buttons[BUTTON_ZR]) ReportData.Button |= ZR_MASK;
    if (buttons[BUTTON_L_STICK]) ReportData.Button |= L_STICK_MASK;
    if (buttons[BUTTON_R_STICK]) ReportData.Button |= R_STICK_MASK;

    if (buttons[BUTTON_HOME]) ReportData.Button |= HOME_MASK;
    if (buttons[BUTTON_CAPTURE]) ReportData.Button |= CAPTURE_MASK;
  }

  if (buttons[BUTTON_CIRCLE]) ReportData.Button |= A_MASK;
  if (buttons[BUTTON_CROSS]) ReportData.Button |= B_MASK;
  if (buttons[BUTTON_TRIANGLE]) ReportData.Button |= X_MASK;
  if (buttons[BUTTON_SQUARE]) ReportData.Button |= Y_MASK;

  if (buttons[BUTTON_PLUS]) ReportData.Button |= PLUS_MASK;
  if (buttons[BUTTON_MINUS]) ReportData.Button |= MINUS_MASK;
}

void handleColorChange(CHSV &color) {
  const int up = matrix.findInList(static_cast<char>(BUTTON_UP));
  const int down = matrix.findInList(static_cast<char>(BUTTON_DOWN));
  const int right = matrix.findInList(static_cast<char>(BUTTON_RIGHT));
  const int left = matrix.findInList(static_cast<char>(BUTTON_LEFT));
  const int zr = matrix.findInList(static_cast<char>(BUTTON_ZR));
  const int zl = matrix.findInList(static_cast<char>(BUTTON_ZL));
  const int r = matrix.findInList(static_cast<char>(BUTTON_R));
  const int l = matrix.findInList(static_cast<char>(BUTTON_L));
  const int rStick = matrix.findInList(static_cast<char>(BUTTON_R_STICK));
  const int lStick = matrix.findInList(static_cast<char>(BUTTON_L_STICK));
  if (up != -1) {
    switch (matrix.key[up].kstate) {
      case PRESSED:
        if (matrix.key[up].stateChanged) {
          color.sat = qadd8(color.sat, SATURATION_STEP);
        }
        break;
      case HOLD:
        EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL / SATURATION_STEP) {
          color.sat = qadd8(color.sat, 1);
        }
        break;
      case RELEASED:
      case IDLE:
      default:
        break;
    }
  }
  if (down != -1) {
    switch (matrix.key[down].kstate) {
      case PRESSED:
        if (matrix.key[down].stateChanged) {
          color.sat = qsub8(color.sat, SATURATION_STEP);
        }
        break;
      case HOLD:
        EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL / SATURATION_STEP) {
          color.sat = qsub8(color.sat, 1);
        }
        break;
      case RELEASED:
      case IDLE:
      default:
        break;
    }
  }
  if (right != -1) {
    switch (matrix.key[right].kstate) {
      case PRESSED:
        if (matrix.key[right].stateChanged) {
          color.hue += HUE_STEP;
        }
        break;
      case HOLD:
        EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL / HUE_STEP) {
          color.hue++;
        }
        break;
      case RELEASED:
      case IDLE:
      default:
        break;
    }
  }
  if (left != -1) {
    switch (matrix.key[left].kstate) {
      case PRESSED:
        if (matrix.key[left].stateChanged) {
          color.hue -= HUE_STEP;
        }
        break;
      case HOLD:
        EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL / HUE_STEP) {
          color.hue--;
        }
        break;
      case RELEASED:
      case IDLE:
      default:
        break;
    }
  }
  if (zr != -1) {
    switch (matrix.key[zr].kstate) {
      case PRESSED:
        if (matrix.key[zr].stateChanged) {
          color.val = qadd8(color.val, qmul8(VALUE_STEP, 3));
        }
        break;
      case HOLD:
        EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL / VALUE_STEP / 3) {
          color.val = qadd8(color.val, 1);
        }
        break;
      case RELEASED:
      case IDLE:
      default:
        break;
    }
  }
  if (zl != -1) {
    switch (matrix.key[zl].kstate) {
      case PRESSED:
        if (matrix.key[zl].stateChanged) {
          color.val = qsub8(color.val, qmul8(VALUE_STEP, 3));
        }
        break;
      case HOLD:
        EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL / VALUE_STEP / 3) {
          color.val = qsub8(color.val, 1);
        }
        break;
      case RELEASED:
      case IDLE:
      default:
        break;
    }
  }
  if (r != -1) {
    switch (matrix.key[r].kstate) {
      case PRESSED:
        if (matrix.key[r].stateChanged) {
          color.val = qadd8(color.val, qmul8(VALUE_STEP, 2));
        }
        break;
      case HOLD:
        EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL / VALUE_STEP / 2) {
          color.val = qadd8(color.val, 1);
        }
        break;
      case RELEASED:
      case IDLE:
      default:
        break;
    }
  }
  if (l != -1) {
    switch (matrix.key[l].kstate) {
      case PRESSED:
        if (matrix.key[l].stateChanged) {
          color.val = qsub8(color.val, qmul8(VALUE_STEP, 2));
        }
        break;
      case HOLD:
        EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL / VALUE_STEP / 2) {
          color.val = qsub8(color.val, 1);
        }
        break;
      case RELEASED:
      case IDLE:
      default:
        break;
    }
  }
  if (rStick != -1) {
    switch (matrix.key[rStick].kstate) {
      case PRESSED:
        if (matrix.key[rStick].stateChanged) {
          color.val = qadd8(color.val, VALUE_STEP);
        }
        break;
      case HOLD:
        EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL / VALUE_STEP) {
          color.val = qadd8(color.val, 1);
        }
        break;
      case RELEASED:
      case IDLE:
      default:
        break;
    }
  }
  if (lStick != -1) {
    switch (matrix.key[lStick].kstate) {
      case PRESSED:
        if (matrix.key[lStick].stateChanged) {
          color.val = qsub8(color.val, VALUE_STEP);
        }
        break;
      case HOLD:
        EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL / VALUE_STEP) {
          color.val = qsub8(color.val, 1);
        }
        break;
      case RELEASED:
      case IDLE:
      default:
        break;
    }
  }
}

void handleButtonLedModeChange() {
  const int home = matrix.findInList(static_cast<char>(BUTTON_HOME));
  const int capture = matrix.findInList(static_cast<char>(BUTTON_CAPTURE));
  if (home != -1) {
    if (matrix.key[home].kstate == PRESSED && matrix.key[home].stateChanged) {
      nextButtonLedMode();
    }
  }
  if (capture != -1) {
    if (matrix.key[capture].kstate == PRESSED &&
        matrix.key[capture].stateChanged) {
      previousButtonLedMode();
    }
  }
}

void handleSliderLedModeChange() {
  const int home = matrix.findInList(static_cast<char>(BUTTON_HOME));
  const int capture = matrix.findInList(static_cast<char>(BUTTON_CAPTURE));
  if (home != -1) {
    if (matrix.key[home].kstate == PRESSED && matrix.key[home].stateChanged) {
      nextSliderLedMode();
    }
  }
  if (capture != -1) {
    if (matrix.key[capture].kstate == PRESSED &&
        matrix.key[capture].stateChanged) {
      previousSliderLedMode();
    }
  }
}

void handleLedSettingsChange() {
  if (buttons[BUTTON_LED]) {
    CHSV newCustomButtonColors[4];
    for (uint8_t i = 0; i < 4; ++i) {
      newCustomButtonColors[0] =
          CHSV(customButtonColors[i].hue, customButtonColors[i].sat,
               buttonLedBrightnessRatios[i]);
    }
    if (buttons[BUTTON_A]) {
      handleColorChange(newCustomButtonColors[0]);
    }
    if (buttons[BUTTON_B]) {
      handleColorChange(newCustomButtonColors[1]);
    }
    if (buttons[BUTTON_Y]) {
      handleColorChange(newCustomButtonColors[2]);
    }
    if (buttons[BUTTON_X]) {
      handleColorChange(newCustomButtonColors[3]);
    }
    if (!buttons[BUTTON_A] && !buttons[BUTTON_B] && !buttons[BUTTON_X] &&
        !buttons[BUTTON_Y]) {
      for (uint8_t i = 0; i < 4; ++i) {
        handleColorChange(newCustomButtonColors[i]);
      }
    }
    changeCustomButtonColors(newCustomButtonColors);
    handleButtonLedModeChange();
  }
  if (buttons[BUTTON_SLIDER_LED]) {
    CHSV newCustomSliderColor(customSliderColor.hue, customSliderColor.sat,
                              sliderLedBrightnessRatio);
    handleColorChange(newCustomSliderColor);
    changeCustomSliderColor(newCustomSliderColor);
    handleSliderLedModeChange();
  }
}