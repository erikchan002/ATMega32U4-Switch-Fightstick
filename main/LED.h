#include <FastLED.h>
#include <stdint.h>

#include "ButtonMatrix.h"
#include "Slider.h"

#define SLIDER_LED_COLOR_CORRECTION 0xFFC0E0
#define BUTTON_LED_COLOR_CORRECTION 0xFFB0E0

#define BUTTON_LEDS_PIN 21
#define SLIDER_LEDS_PIN 20
#define LEDS_PER_SENSOR 1
#define LEDS_PER_BUTTON 1
#define NUMBER_OF_SLIDER_LEDS NUMBER_OF_SLIDER_SENSORS* LEDS_PER_SENSOR
#define NUMBER_OF_BUTTON_LEDS 4 * LEDS_PER_BUTTON
#define NUMBER_OF_SLIDER_LED_MODES 5
#define NUMBER_OF_BUTTON_LED_MODES 6
#define RAINBOW_INTERVAL 10  // seconds per cycle
#define FRAME_RATE 120       //  per second
extern CRGB sliderLeds[NUMBER_OF_SLIDER_LEDS];
extern CRGB buttonLeds[NUMBER_OF_BUTTON_LEDS];

enum class ButtonLedMode {
  arcadeSimulation,
  nintendo,
  sony,
  rainbow,
  swirl,
  custom
};
extern ButtonLedMode buttonLedMode;

enum class SliderLedMode {
  arcadeSerial,
  arcadeSimulation,
  rainbow,
  swirl,
  custom
};
extern SliderLedMode sliderLedMode;

extern CHSV customButtonColors[4];
extern CHSV customSliderColor;

extern uint8_t buttonLedBrightnessRatios[4];
extern uint8_t sliderLedBrightnessRatio;

void setupLeds();
void handleSliderLeds();
void handleButtonLeds();
void readColorSettings();
void setButtonLedMode(ButtonLedMode mode);
void nextButtonLedMode();
void previousButtonLedMode();
void setSliderLedMode(SliderLedMode mode);
void nextSliderLedMode();
void previousSliderLedMode();
void changeCustomButtonColors(CHSV newCustomButtonColors[4]);
void changeCustomSliderColor(CHSV newCustomSliderColor);
void handleLeds();