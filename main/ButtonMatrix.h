#include <FastLED.h>
#include <Keypad.h>

#define DEBOUNCE_INTERVAL 3       // ms
#define HOLD_TIME 1000            // ms
#define HOLD_REPEAT_INTERVAL 200  // ms
#define SATURATION_STEP 8
#define HUE_STEP 8
#define VAL_STEP 8

#define DPAD_UP_MASK 0x00
#define DPAD_UPRIGHT_MASK 0x01
#define DPAD_RIGHT_MASK 0x02
#define DPAD_DOWNRIGHT_MASK 0x03
#define DPAD_DOWN_MASK 0x04
#define DPAD_DOWNLEFT_MASK 0x05
#define DPAD_LEFT_MASK 0x06
#define DPAD_UPLEFT_MASK 0x07
#define DPAD_NONE_MASK 0x08

#define Y_MASK 0x0001
#define B_MASK 0x0002
#define A_MASK 0x0004
#define X_MASK 0x0008
#define L_MASK 0x0010
#define R_MASK 0x0020
#define ZL_MASK 0x0040
#define ZR_MASK 0x0080
#define MINUS_MASK 0x0100
#define PLUS_MASK 0x0200
#define L_STICK_MASK 0x0400
#define R_STICK_MASK 0x0800
#define HOME_MASK 0x1000
#define CAPTURE_MASK 0x2000

#define NUMBER_OF_BUTTONS 24
#define NUMBER_OF_ROWS 5
#define NUMBER_OF_COLUMNS 5

#define ROW_0_PIN 4
#define ROW_1_PIN 5
#define ROW_2_PIN 6
#define ROW_3_PIN 7
#define ROW_4_PIN 8
#define COL_0_PIN 9
#define COL_1_PIN 15
#define COL_2_PIN 14
#define COL_3_PIN 16
#define COL_4_PIN 10
#define LED_BUTTON_LED_PIN 18
#define SLIDER_LED_BUTTON_LED_PIN 19

#define BUTTON_NONE NUMBER_OF_BUTTONS
#define BUTTON_UP 0
#define BUTTON_DOWN 1
#define BUTTON_LEFT 2
#define BUTTON_RIGHT 3
#define BUTTON_CIRCLE 4
#define BUTTON_CROSS 5
#define BUTTON_SQUARE 6
#define BUTTON_TRIANGLE 7
#define BUTTON_A 8
#define BUTTON_B 9
#define BUTTON_X 10
#define BUTTON_Y 11
#define BUTTON_L 12
#define BUTTON_R 13
#define BUTTON_ZL 14
#define BUTTON_ZR 15
#define BUTTON_L_STICK 16
#define BUTTON_R_STICK 17
#define BUTTON_PLUS 18
#define BUTTON_MINUS 19
#define BUTTON_HOME 20
#define BUTTON_CAPTURE 21
#define BUTTON_LED 22
#define BUTTON_SLIDER_LED 23

extern bool buttons[NUMBER_OF_BUTTONS];

void setupMatrix();
void readMatrix();
void writeMatrixToReport();
void handleColorChange(CHSV &color);
void handleLedSettingsChange();