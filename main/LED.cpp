#include "LED.h"

#include <EEPROM.h>

CRGB sliderLeds[NUMBER_OF_SLIDER_LEDS];
CRGB buttonLeds[NUMBER_OF_BUTTON_LEDS];

ButtonLedMode buttonLedMode;
SliderLedMode sliderLedMode;

uint8_t autoHue = 0;
uint8_t breathe = 0;
uint8_t fade = 255;

CHSV customButtonColors[4];
CHSV customSliderColor;

const uint8_t ledMaxBrightness = 255;

uint8_t buttonLedBrightnessRatios[4] = {255, 255, 255, 255};
uint8_t sliderLedBrightnessRatio = 255;
const uint8_t sliderLedTouchBrightnessRatio = 255;
const uint8_t sliderLedBackgroundBrightnessRatio = 64;
const uint8_t sliderLedBackgroundFadeInSpeed = 4;
const uint8_t sliderLedBackgroundFadeOutSpeed = 16;

void setupLeds() {
  FastLED
      .addLeds<WS2812B, SLIDER_LEDS_PIN, GRB>(sliderLeds, NUMBER_OF_SLIDER_LEDS)
      .setCorrection(SLIDER_LED_COLOR_CORRECTION);
  FastLED
      .addLeds<PL9823, BUTTON_LEDS_PIN, RGB>(buttonLeds, NUMBER_OF_BUTTON_LEDS)
      .setCorrection(BUTTON_LED_COLOR_CORRECTION);
  FastLED.setBrightness(ledMaxBrightness);
  readColorSettings();
}
void handleSliderLeds() {
  if (sliderLedMode != SliderLedMode::arcadeSerial) {
    switch (sliderLedMode) {
      case SliderLedMode::rainbow:
        fill_rainbow(sliderLeds, NUMBER_OF_SLIDER_LEDS, autoHue, 0);
        break;
      case SliderLedMode::swirl:
        fill_rainbow(sliderLeds, NUMBER_OF_SLIDER_LEDS, autoHue,
                     256 / (NUMBER_OF_SLIDER_LEDS + 1));
        break;
      case SliderLedMode::custom:
        fill_solid(sliderLeds, NUMBER_OF_SLIDER_LEDS, CRGB(customSliderColor));
        break;
      case SliderLedMode::arcadeSimulation:
      default:
        fill_solid(sliderLeds, NUMBER_OF_SLIDER_LEDS, CRGB::White);
        break;
    }
    bool isTouching = false;
    uint8_t sliderLedBackgroundBreatheBrightnessRatio =
        sliderLedBackgroundBrightnessRatio / 3 * 2 +
        sliderLedBackgroundBrightnessRatio * quadwave8(breathe) / 255 / 3;
    for (uint8_t i = 0; i < NUMBER_OF_SLIDER_SENSORS; ++i) {
      if (sliderTouches[i]) {
        switch (sliderLedMode) {
          case SliderLedMode::rainbow:
          case SliderLedMode::swirl:
          case SliderLedMode::custom:
            break;
          case SliderLedMode::arcadeSimulation:
          default:
            fill_solid(sliderLeds + i * LEDS_PER_SENSOR, LEDS_PER_SENSOR,
                       CRGB::MediumTurquoise);
            break;
        }
        for (uint8_t j = 0; j < LEDS_PER_SENSOR; ++j) {
          sliderLeds[i * LEDS_PER_SENSOR + j] %= sliderLedTouchBrightnessRatio;
        }
        isTouching = true;
      } else {
        for (uint8_t j = 0; j < LEDS_PER_SENSOR; ++j) {
          sliderLeds[i * LEDS_PER_SENSOR + j] %=
              sliderLedBackgroundBreatheBrightnessRatio;
          sliderLeds[i * LEDS_PER_SENSOR + j] %= fade;
        }
      }
      for (uint8_t j = 0; j < LEDS_PER_SENSOR; ++j) {
        sliderLeds[i * LEDS_PER_SENSOR + j] %= sliderLedBrightnessRatio;
      }
    }
    if (isTouching) {
      EVERY_N_MILLISECONDS(10) {
        breathe = 0;
        fade = qsub8(fade, sliderLedBackgroundFadeOutSpeed);
      }
    } else {
      EVERY_N_MILLISECONDS(10) {
        breathe++;
        fade = qadd8(fade, sliderLedBackgroundFadeInSpeed);
      }
    }
  }
}
void handleButtonLeds() {
  uint8_t i;
  switch (buttonLedMode) {
    case ButtonLedMode::nintendo:
      fill_solid(buttonLeds + 0 * LEDS_PER_BUTTON, LEDS_PER_BUTTON, CRGB::Red);
      fill_solid(buttonLeds + 1 * LEDS_PER_BUTTON, LEDS_PER_BUTTON,
                 CRGB::Yellow);
      fill_solid(buttonLeds + 2 * LEDS_PER_BUTTON, LEDS_PER_BUTTON,
                 CRGB::Green);
      fill_solid(buttonLeds + 3 * LEDS_PER_BUTTON, LEDS_PER_BUTTON, CRGB::Blue);
      break;
    case ButtonLedMode::sony:
      fill_solid(buttonLeds + 0 * LEDS_PER_BUTTON, LEDS_PER_BUTTON, CRGB::Red);
      fill_solid(buttonLeds + 1 * LEDS_PER_BUTTON, LEDS_PER_BUTTON, CRGB::Blue);
      fill_solid(buttonLeds + 2 * LEDS_PER_BUTTON, LEDS_PER_BUTTON,
                 CRGB::Purple);
      fill_solid(buttonLeds + 3 * LEDS_PER_BUTTON, LEDS_PER_BUTTON,
                 CRGB::Green);
      break;
    case ButtonLedMode::rainbow:
      fill_rainbow(buttonLeds, NUMBER_OF_BUTTON_LEDS, autoHue, 0);
      break;
    case ButtonLedMode::swirl:
      fill_rainbow(buttonLeds, NUMBER_OF_BUTTON_LEDS, autoHue, 256 / 5);
      break;
    case ButtonLedMode::custom:
      for (i = 0; i < 4; ++i) {
        fill_solid(buttonLeds + i * LEDS_PER_BUTTON, LEDS_PER_BUTTON,
                   customButtonColors[i]);
      }
      break;
    case ButtonLedMode::arcadeSimulation:
    default:
      fill_solid(buttonLeds, NUMBER_OF_BUTTON_LEDS, CRGB::White);
      break;
  }
  for (i = 0; i < NUMBER_OF_BUTTON_LEDS; ++i) {
    buttonLeds[i] %= buttonLedBrightnessRatios[i];
  }
  if (buttons[BUTTON_CIRCLE]) {
    fill_solid(buttonLeds + 0 * LEDS_PER_BUTTON, LEDS_PER_BUTTON, CRGB::Black);
  }
  if (buttons[BUTTON_CROSS]) {
    fill_solid(buttonLeds + 1 * LEDS_PER_BUTTON, LEDS_PER_BUTTON, CRGB::Black);
  }
  if (buttons[BUTTON_SQUARE]) {
    fill_solid(buttonLeds + 2 * LEDS_PER_BUTTON, LEDS_PER_BUTTON, CRGB::Black);
  }
  if (buttons[BUTTON_TRIANGLE]) {
    fill_solid(buttonLeds + 3 * LEDS_PER_BUTTON, LEDS_PER_BUTTON, CRGB::Black);
  }
}

void readColorSettings() {
  buttonLedMode = ButtonLedMode(EEPROM.read(0));
  sliderLedMode = SliderLedMode(EEPROM.read(1));
  for (uint8_t i = 0; i < 4; ++i) {
    customButtonColors[i] =
        CHSV(EEPROM.read(2 + i * 3), EEPROM.read(3 + i * 3), 255);
    buttonLedBrightnessRatios[i] = EEPROM.read(4 + i * 3);
  }
  customSliderColor = CHSV(EEPROM.read(2 + 4 * 3), EEPROM.read(3 + 4 * 3), 255);
  sliderLedBrightnessRatio = EEPROM.read(4 + 4 * 3);
}

void setButtonLedMode(ButtonLedMode mode) {
  buttonLedMode = mode;
  EEPROM.write(0, static_cast<uint8_t>(mode));
}

void nextButtonLedMode() {
  setButtonLedMode(static_cast<ButtonLedMode>(addmod8(
      static_cast<uint8_t>(buttonLedMode), 1, NUMBER_OF_BUTTON_LED_MODES)));
}

void previousButtonLedMode() {
  setButtonLedMode(static_cast<ButtonLedMode>(
      addmod8(static_cast<uint8_t>(buttonLedMode),
              NUMBER_OF_BUTTON_LED_MODES - 1, NUMBER_OF_BUTTON_LED_MODES)));
}

void setSliderLedMode(SliderLedMode mode) {
  sliderLedMode = mode;
  EEPROM.write(1, static_cast<uint8_t>(mode));
  if (sliderLedMode == SliderLedMode::arcadeSerial) {
    fill_solid(sliderLeds, NUMBER_OF_SLIDER_LEDS, CRGB::Black);
  }
}

void nextSliderLedMode() {
  setSliderLedMode(static_cast<SliderLedMode>(addmod8(
      static_cast<uint8_t>(sliderLedMode), 1, NUMBER_OF_SLIDER_LED_MODES)));
}

void previousSliderLedMode() {
  setSliderLedMode(static_cast<SliderLedMode>(
      addmod8(static_cast<uint8_t>(sliderLedMode),
              NUMBER_OF_SLIDER_LED_MODES - 1, NUMBER_OF_SLIDER_LED_MODES)));
}

void changeCustomButtonColors(CHSV newCustomButtonColors[4]) {
  for (uint8_t i = 0; i < 4; ++i) {
    customButtonColors[i] =
        CHSV(newCustomButtonColors[i].hue, newCustomButtonColors[i].sat, 255);
    buttonLedBrightnessRatios[i] = newCustomButtonColors[i].val;
    EEPROM.write(2 + i * 3, newCustomButtonColors[i].hue);
    EEPROM.write(3 + i * 3, newCustomButtonColors[i].sat);
    EEPROM.write(4 + i * 3, newCustomButtonColors[i].val);
  }
}

void changeCustomSliderColor(CHSV newCustomSliderColor) {
  customSliderColor =
      CHSV(newCustomSliderColor.hue, newCustomSliderColor.sat, 255);
  sliderLedBrightnessRatio = newCustomSliderColor.val;
  EEPROM.write(2 + 4 * 3, newCustomSliderColor.hue);
  EEPROM.write(3 + 4 * 3, newCustomSliderColor.sat);
  EEPROM.write(4 + 4 * 3, newCustomSliderColor.val);
}

void handleLeds() {
  EVERY_N_MILLISECONDS(RAINBOW_INTERVAL * 1000 / 256) {
    autoHue++;
  }
  EVERY_N_MILLISECONDS(1000 / FRAME_RATE) {
    handleButtonLeds();
    handleSliderLeds();
    FastLED.show();
  }
}