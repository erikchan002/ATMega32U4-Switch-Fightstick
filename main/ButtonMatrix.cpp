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
  pinMode(LED_BUTTON_LED_PIN, OUTPUT);
  pinMode(SLIDER_LED_BUTTON_LED_PIN, OUTPUT);
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
  switch (matrix.key[matrix.findInList(BUTTON_UP)].kstate) {
    case PRESSED:
      color.sat += SATURATION_STEP;
      break;
    case HOLD:
      EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL) {
        color.sat += SATURATION_STEP;
      }
      break;
    case RELEASED:
    case IDLE:
    default:
      break;
  }
  switch (matrix.key[matrix.findInList(BUTTON_DOWN)].kstate) {
    case PRESSED:
      color.sat -= SATURATION_STEP;
      break;
    case HOLD:
      EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL) {
        color.sat -= SATURATION_STEP;
      }
      break;
    case RELEASED:
    case IDLE:
    default:
      break;
  }
  switch (matrix.key[matrix.findInList(BUTTON_RIGHT)].kstate) {
    case PRESSED:
      color.hue += HUE_STEP;
      break;
    case HOLD:
      EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL) {
        color.hue += HUE_STEP;
      }
      break;
    case RELEASED:
    case IDLE:
    default:
      break;
  }
  switch (matrix.key[matrix.findInList(BUTTON_LEFT)].kstate) {
    case PRESSED:
      color.hue -= HUE_STEP;
      break;
    case HOLD:
      EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL) {
        color.hue -= HUE_STEP;
      }
      break;
    case RELEASED:
    case IDLE:
    default:
      break;
  }
  switch (matrix.key[matrix.findInList(BUTTON_ZR)].kstate) {
    case PRESSED:
      color.val += VAL_STEP * 3;
      break;
    case HOLD:
      EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL) {
        color.val += VAL_STEP * 3;
      }
      break;
    case RELEASED:
    case IDLE:
    default:
      break;
  }
  switch (matrix.key[matrix.findInList(BUTTON_ZL)].kstate) {
    case PRESSED:
      color.val -= VAL_STEP * 3;
      break;
    case HOLD:
      EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL) {
        color.val -= VAL_STEP * 3;
      }
      break;
    case RELEASED:
    case IDLE:
    default:
      break;
  }
  switch (matrix.key[matrix.findInList(BUTTON_R)].kstate) {
    case PRESSED:
      color.val += VAL_STEP * 2;
      break;
    case HOLD:
      EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL) {
        color.val += VAL_STEP * 2;
      }
      break;
    case RELEASED:
    case IDLE:
    default:
      break;
  }
  switch (matrix.key[matrix.findInList(BUTTON_L)].kstate) {
    case PRESSED:
      color.val -= VAL_STEP * 2;
      break;
    case HOLD:
      EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL) {
        color.val -= VAL_STEP * 2;
      }
      break;
    case RELEASED:
    case IDLE:
    default:
      break;
  }
  switch (matrix.key[matrix.findInList(BUTTON_R_STICK)].kstate) {
    case PRESSED:
      color.val += VAL_STEP;
      break;
    case HOLD:
      EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL) {
        color.val += VAL_STEP;
      }
      break;
    case RELEASED:
    case IDLE:
    default:
      break;
  }
  switch (matrix.key[matrix.findInList(BUTTON_L_STICK)].kstate) {
    case PRESSED:
      color.val -= VAL_STEP;
      break;
    case HOLD:
      EVERY_N_MILLISECONDS(HOLD_REPEAT_INTERVAL) {
        color.val -= VAL_STEP;
      }
      break;
    case RELEASED:
    case IDLE:
    default:
      break;
  }
}

void handleLedSettingsChange() {
  if (buttons[BUTTON_LED]) {
    digitalWrite(LED_BUTTON_LED_PIN, LOW);
    if (buttons[BUTTON_A]) {
      handleColorChange(customButtonColors[0]);
    }
    if (buttons[BUTTON_B]) {
      handleColorChange(customButtonColors[1]);
    }
    if (buttons[BUTTON_Y]) {
      handleColorChange(customButtonColors[2]);
    }
    if (buttons[BUTTON_X]) {
      handleColorChange(customButtonColors[3]);
    }
    if (!buttons[BUTTON_A] && !buttons[BUTTON_B] && !buttons[BUTTON_X] &&
        !buttons[BUTTON_Y]) {
      for (uint8_t i = 0; i < 4; ++i) {
        handleColorChange(customButtonColors[i]);
      }
    }
    changeCustomButtonColors(customButtonColors);

    if (matrix.key[matrix.findInList(BUTTON_HOME)].kstate == PRESSED) {
      nextButtonLedMode();
    }
    if (matrix.key[matrix.findInList(BUTTON_CAPTURE)].kstate == PRESSED) {
      previousButtonLedMode();
    }
  } else {
    digitalWrite(LED_BUTTON_LED_PIN, HIGH);
  }
  if (buttons[BUTTON_SLIDER_LED]) {
    digitalWrite(SLIDER_LED_BUTTON_LED_PIN, LOW);
    handleColorChange(customSliderColor);
    changeCustomSliderColor(customSliderColor);
    if (matrix.key[matrix.findInList(BUTTON_HOME)].kstate == PRESSED) {
      nextSliderLedMode();
    }
    if (matrix.key[matrix.findInList(BUTTON_CAPTURE)].kstate == PRESSED) {
      previousSliderLedMode();
    }
  } else {
    digitalWrite(SLIDER_LED_BUTTON_LED_PIN, HIGH);
  }
}