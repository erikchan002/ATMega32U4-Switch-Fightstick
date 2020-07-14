#include "LED.h"

#include <EEPROM.h>

CRGB sliderLeds[NUMBER_OF_SLIDER_LEDS];
CRGB buttonLeds[NUMBER_OF_BUTTON_LEDS];

ButtonLedMode buttonLedMode;
SliderLedMode sliderLedMode;

uint8_t autoHue = 0;
uint8_t breathe = 0;

CHSV customButtonColors[4];
CHSV customSliderColor;

const uint8_t ledMaxBrightness = 255;

const uint8_t buttonLedBrightnessRatio = 255;
const uint8_t sliderLedTouchBrightnessRatio = 255;
const uint8_t sliderLedRippleBrightnessRatio = 8;
const uint8_t sliderLedBackgroundBrightnessRatio = 64;
const uint8_t sliderLedBackgroundFadeInSpeed = 1;
const uint8_t sliderLedBackgroundFadeOutSpeed = 4;
uint8_t currentSliderLedBackgroundBrightnessRatio =
    sliderLedBackgroundBrightnessRatio;

// uint8_t rippleLeft[NUMBER_OF_SLIDER_SENSORS] = {NUMBER_OF_SLIDER_SENSORS};
// uint8_t rippleRight[NUMBER_OF_SLIDER_SENSORS] = {NUMBER_OF_SLIDER_SENSORS};
// int8_t rippleLeftChange[NUMBER_OF_SLIDER_SENSORS] = {0};
// int8_t rippleRightChange[NUMBER_OF_SLIDER_SENSORS] = {0};

void setupLeds() {
  FastLED
      .addLeds<WS2812B, SLIDER_LEDS_PIN, GRB>(sliderLeds, NUMBER_OF_SLIDER_LEDS)
      .setCorrection(TypicalSMD5050);
  FastLED
      .addLeds<PL9823, BUTTON_LEDS_PIN, RGB>(buttonLeds, NUMBER_OF_BUTTON_LEDS)
      .setCorrection(Typical8mmPixel);
  FastLED.setBrightness(ledMaxBrightness);
  readColorSettings();
}
void handleSliderLeds() {
  if (sliderLedMode != SliderLedMode::arcadeSerial) {
    FastLED.setBrightness(ledMaxBrightness);
    switch (sliderLedMode) {
      case SliderLedMode::rainbow:
        fill_rainbow(sliderLeds, NUMBER_OF_SLIDER_LEDS, autoHue, 0);
        break;
      case SliderLedMode::swirl:
        fill_rainbow(sliderLeds, NUMBER_OF_SLIDER_LEDS, autoHue,
                     256 / (NUMBER_OF_SLIDER_LEDS + 1));
        break;
      case SliderLedMode::custom:
        fill_solid(sliderLeds, NUMBER_OF_SLIDER_LEDS, customSliderColor);
        break;
      case SliderLedMode::arcadeSimulation:
      default:
        fill_solid(sliderLeds, NUMBER_OF_SLIDER_LEDS, CRGB::White);
        break;
    }
    bool isTouching = false;
    uint8_t currentSliderLedBackgroundBreatheBrightnessRatio =
        currentSliderLedBackgroundBrightnessRatio / 4 * 3 +
        currentSliderLedBackgroundBrightnessRatio * quadwave8(breathe) / 255 /
            4;
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
          sliderLeds[i * LEDS_PER_SENSOR + j].maximizeBrightness(
              sliderLedTouchBrightnessRatio);
        }
        // EVERY_N_MILLISECONDS(50){
        //   if(rippleRightChange[i]==0){
        //     rippleRight[i] = i;
        //     rippleRightChange[i] = -1;
        //   } else {
        //     if(rippleRight[i]>=i){
        //       rippleRight[i] = i-1;
        //       rippleRightChange[i] = -1;
        //     } else if(rippleRight[i]<=0){
        //       rippleRight[i] = 1;
        //       rippleRightChange[i] = +1;
        //     } else {
        //       rippleRight[i]+=rippleRightChange[i];
        //     }
        //   }
        //   if(rippleLeftChange[i]==0){
        //     rippleLeft[i] = i;
        //     rippleLeftChange[i] = +1;
        //   } else {
        //     if(rippleLeft[i]<=i){
        //       rippleLeft[i] = i+1;
        //       rippleLeftChange[i] = +1;
        //     } else if(rippleLeft[i]>=NUMBER_OF_SLIDER_SENSORS){
        //       rippleLeft[i] = NUMBER_OF_SLIDER_SENSORS - 1;
        //       rippleLeftChange[i] = -1;
        //     } else {
        //       rippleLeft[i]+=rippleLeftChange[i];
        //     }
        //   }
        // }
        isTouching = true;
      } else {
        // rippleLeftChange[i] = 0;
        // rippleRightChange[i] = 0;
        // bool inRippleRange = false;
        // for(uint8_t j = 0; j < NUMBER_OF_SLIDER_SENSORS; ++j){
        //   if(i>=rippleRight[j] && i<=rippleLeft[j] && sliderTouches[j]){
        //     inRippleRange = true;
        //     break;
        //   }
        // }
        // if(inRippleRange){
        //   for(uint8_t j = 0; j < LEDS_PER_SENSOR; ++j){
        //     sliderLeds[i*LEDS_PER_SENSOR+j] =
        //     CRGB::MediumTurquoise;
        //     sliderLeds[i*LEDS_PER_SENSOR+j].maximizeBrightness(sliderLedRippleBrightnessRatio);
        //   }
        // }
        // else {
        for (uint8_t j = 0; j < LEDS_PER_SENSOR; ++j) {
          sliderLeds[i * LEDS_PER_SENSOR + j].maximizeBrightness(
              currentSliderLedBackgroundBreatheBrightnessRatio);
        }
        // }
      }
    }
    if (isTouching) {
      EVERY_N_MILLISECONDS(10) {
        breathe = 0;
        if (currentSliderLedBackgroundBrightnessRatio <
            sliderLedBackgroundFadeOutSpeed) {
          currentSliderLedBackgroundBrightnessRatio = 0;
        } else {
          currentSliderLedBackgroundBrightnessRatio -=
              sliderLedBackgroundFadeOutSpeed;
        }
      }
    } else {
      EVERY_N_MILLISECONDS(10) {
        breathe++;
        if (breathe >= 255) {
          breathe = 0;
        }
        if (currentSliderLedBackgroundBrightnessRatio >
            (sliderLedBackgroundBrightnessRatio -
             sliderLedBackgroundFadeInSpeed)) {
          currentSliderLedBackgroundBrightnessRatio =
              sliderLedBackgroundBrightnessRatio;
        } else {
          currentSliderLedBackgroundBrightnessRatio +=
              sliderLedBackgroundFadeInSpeed;
        }
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
    buttonLeds[i].maximizeBrightness(buttonLedBrightnessRatio);
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
    customButtonColors[i] = CHSV(EEPROM.read(2 + i * 3), EEPROM.read(3 + i * 3),
                                 EEPROM.read(4 + i * 3));
  }
  customSliderColor = CHSV(EEPROM.read(2 + 4 * 3), EEPROM.read(3 + 4 * 3),
                           EEPROM.read(4 + 4 * 3));
}

void setButtonLedMode(ButtonLedMode mode) {
  buttonLedMode = mode;
  EEPROM.write(0, static_cast<uint8_t>(mode));
}

void nextButtonLedMode() {
  if (buttonLedMode == ButtonLedMode::custom) {
    setButtonLedMode(ButtonLedMode::arcadeSimulation);
  } else {
    setButtonLedMode(
        static_cast<ButtonLedMode>(static_cast<uint8_t>(buttonLedMode) + 1));
  }
}

void previousButtonLedMode() {
  if (buttonLedMode == ButtonLedMode::arcadeSimulation) {
    setButtonLedMode(ButtonLedMode::custom);
  } else {
    setButtonLedMode(
        static_cast<ButtonLedMode>(static_cast<uint8_t>(buttonLedMode) - 1));
  }
}

void setSliderLedMode(SliderLedMode mode) {
  sliderLedMode = mode;
  EEPROM.write(1, static_cast<uint8_t>(mode));
}

void nextSliderLedMode() {
  if (sliderLedMode == SliderLedMode::custom) {
    setSliderLedMode(SliderLedMode::arcadeSerial);
  } else {
    setSliderLedMode(
        static_cast<SliderLedMode>(static_cast<uint8_t>(sliderLedMode) + 1));
  }
}

void previousSliderLedMode() {
  if (sliderLedMode == SliderLedMode::arcadeSerial) {
    setSliderLedMode(SliderLedMode::custom);
  } else {
    setSliderLedMode(
        static_cast<SliderLedMode>(static_cast<uint8_t>(sliderLedMode) - 1));
  }
}

void changeCustomButtonColors(CHSV newCustomButtonColors[4]) {
  for (uint8_t i = 0; i < 4; ++i) {
    customButtonColors[i] = newCustomButtonColors[i];
    EEPROM.write(2 + i * 3, customButtonColors[i].hue);
    EEPROM.write(3 + i * 3, customButtonColors[i].sat);
    EEPROM.write(4 + i * 3, customButtonColors[i].val);
  }
}

void changeCustomSliderColor(CHSV newCustomSliderColor) {
  customSliderColor = newCustomSliderColor;
  EEPROM.write(2 + 4 * 3, customSliderColor.hue);
  EEPROM.write(3 + 4 * 3, customSliderColor.sat);
  EEPROM.write(4 + 4 * 3, customSliderColor.val);
}

void handleLeds() {
  EVERY_N_MILLISECONDS(20) {
    autoHue++;
  }
  EVERY_N_MILLISECONDS(1000 / 120) {
    handleButtonLeds();
    handleSliderLeds();
    FastLED.show();
  }
}