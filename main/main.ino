#include "LUFAConfig.h"
#include <LUFA.h>
#include "Joystick.h"
#include <Keypad.h>
#include <Wire.h>
#include <FastLED.h>
#include <EEPROM.h>

/* ------------------------ Button matrix code - begins ------------------------ */

#define DEBOUNCE_INTERVAL 3 //ms

#define DPAD_UP_MASK        0x00
#define DPAD_UPRIGHT_MASK   0x01
#define DPAD_RIGHT_MASK     0x02
#define DPAD_DOWNRIGHT_MASK 0x03
#define DPAD_DOWN_MASK      0x04
#define DPAD_DOWNLEFT_MASK  0x05
#define DPAD_LEFT_MASK      0x06
#define DPAD_UPLEFT_MASK    0x07
#define DPAD_NONE_MASK    0x08

#define Y_MASK        0x0001
#define B_MASK        0x0002
#define A_MASK        0x0004
#define X_MASK        0x0008
#define L_MASK        0x0010
#define R_MASK        0x0020
#define ZL_MASK       0x0040
#define ZR_MASK       0x0080
#define MINUS_MASK    0x0100
#define PLUS_MASK     0x0200
#define L_STICK_MASK  0x0400
#define R_STICK_MASK  0x0800
#define HOME_MASK     0x1000
#define CAPTURE_MASK  0x2000

#define NUMBER_OF_BUTTONS 22
#define NUMBER_OF_ROWS 5
#define NUMBER_OF_COLUMNS 5

#define NONE NUMBER_OF_BUTTONS
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define CIRCLE 4
#define CROSS 5
#define SQUARE 6
#define TRIANGLE 7
#define A 8
#define B 9
#define X 10
#define Y 11
#define L 12
#define R 13
#define ZL 14
#define ZR 15
#define L_STICK 16
#define R_STICK 17
#define PLUS 18
#define MINUS 19
#define HOME 20
#define CAPTURE 21

bool buttons[NUMBER_OF_BUTTONS] = {false};
uint8_t keymap[NUMBER_OF_ROWS][NUMBER_OF_COLUMNS] = {
  {UP, L_STICK, NONE, R_STICK, X},
  {LEFT, L, NONE, R, A},
  {DOWN, ZL, HOME, ZR, Y},
  {RIGHT, MINUS, CAPTURE, PLUS, B},
  {TRIANGLE, SQUARE, NONE, CROSS, CIRCLE}
};

uint8_t rowPins[NUMBER_OF_ROWS] = {4,5,6,7,8};
uint8_t colPins[NUMBER_OF_COLUMNS] = {9,10,16,14,15};
Keypad matrix = Keypad( makeKeymap(keymap), rowPins, colPins, NUMBER_OF_ROWS, NUMBER_OF_COLUMNS );

void setupMatrix(){
  matrix.setDebounceTime(DEBOUNCE_INTERVAL);
}
void readMatrix(){
  if (matrix.getKeys()){
    for (int i=0; i<LIST_MAX; i++){
      if ( matrix.key[i].stateChanged && matrix.key[i].kchar != NONE ){
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
void writeMatrixToReport(){
  ReportData.HAT = DPAD_NONE_MASK;
  if(buttons[RIGHT]) ReportData.HAT = DPAD_RIGHT_MASK;
  if(buttons[DOWN]) ReportData.HAT = DPAD_DOWN_MASK;
  if(buttons[LEFT]) ReportData.HAT = DPAD_LEFT_MASK;
  if(buttons[UP]) ReportData.HAT = DPAD_UP_MASK;
  if(buttons[DOWN] && buttons[RIGHT]) ReportData.HAT = DPAD_DOWNRIGHT_MASK;
  if(buttons[DOWN] && buttons[LEFT]) ReportData.HAT = DPAD_DOWNLEFT_MASK;
  if(buttons[UP] && buttons[RIGHT]) ReportData.HAT = DPAD_UPRIGHT_MASK;
  if(buttons[UP] && buttons[LEFT]) ReportData.HAT = DPAD_UPLEFT_MASK;

  if(buttons[A] || buttons[CIRCLE]) ReportData.Button |= A_MASK;
  if(buttons[B] || buttons[CROSS]) ReportData.Button |= B_MASK;
  if(buttons[X] || buttons[TRIANGLE]) ReportData.Button |= X_MASK;
  if(buttons[Y] || buttons[SQUARE]) ReportData.Button |= Y_MASK;
  if(buttons[L]) ReportData.Button |= L_MASK;
  if(buttons[R]) ReportData.Button |= R_MASK;
  if(buttons[ZL]) ReportData.Button |= ZL_MASK;
  if(buttons[ZR]) ReportData.Button |= ZR_MASK;
  if(buttons[L_STICK]) ReportData.Button |= L_STICK_MASK;
  if(buttons[R_STICK]) ReportData.Button |= R_STICK_MASK;
  if(buttons[PLUS]) ReportData.Button |= PLUS_MASK;
  if(buttons[MINUS]) ReportData.Button |= MINUS_MASK;
  if(buttons[HOME]) ReportData.Button |= HOME_MASK;
  if(buttons[CAPTURE]) ReportData.Button |= CAPTURE_MASK;
}

/* ------------------------- Button matrix code - ends ------------------------- */

/* --------------------------- Slider code - begins ---------------------------- */

#define SLIDER_I2C_ADDRESS 0x08
#define NUMBER_OF_SLIDER_SENSORS 32
#define BUFFER_SIZE (NUMBER_OF_SLIDER_SENSORS+7)/8
bool sliderTouches[NUMBER_OF_SLIDER_SENSORS] = {false};
void setupSlider(){
  Wire.setClock(400000L);
  Wire.begin();
}
void readSlider() {
  Wire.requestFrom(SLIDER_I2C_ADDRESS, BUFFER_SIZE);
  uint8_t numberOfSensorsRead = 0;
  for(uint8_t i=0;i<BUFFER_SIZE;++i){
    if(!Wire.available()) break;
    uint8_t incoming = Wire.read();
    for(uint8_t j=0;j<8;++j){
        if(numberOfSensorsRead>=NUMBER_OF_SLIDER_SENSORS) break;
        sliderTouches[numberOfSensorsRead] = (incoming >> j) & 1;
        ++numberOfSensorsRead;
    }
  }
}
void writeSliderToReport(){
  uint8_t sticks[4] = {0};
  for(uint8_t i=0; i<NUMBER_OF_SLIDER_SENSORS;++i){
    if(sliderTouches[i]) sticks[i/8] |= (1 << (i%8));
  }
  ReportData.LX = sticks[0] ^ 0x80;
  ReportData.LY = sticks[1] ^ 0x80;
  ReportData.RX = sticks[2] ^ 0x80;
  ReportData.RY = sticks[3] ^ 0x80;
}

/* ---------------------------- Slider code - ends ----------------------------- */

/* ---------------------------- LED code - begins ------------------------------ */

#define LED_PIN 18
#define LEDS_PER_SENSOR 1
#define LEDS_PER_BUTTON 1
CRGB leds[NUMBER_OF_SLIDER_SENSORS*LEDS_PER_SENSOR+4*LEDS_PER_BUTTON];
enum class ButtonLedMode { arcade, nintendo, sony, rainbow, swirl, custom } buttonLedMode;
enum class SliderLedMode { arcade, rainbow, swirl, custom } sliderLedMode;
uint8_t autoHue = 0;
uint8_t breathe = 0;
CRGB customColors[5];
const uint8_t ledMaxBrightness = 32;
const uint8_t buttonLedBrightnessRatio = 255;
const uint8_t sliderLedTouchBrightnessRatio = 255;
const uint8_t sliderLedRippleBrightnessRatio = 8;
const uint8_t sliderLedBackgroundBrightnessRatio = 64;
const uint8_t sliderLedBackgroundFadeInSpeed = 1;
const uint8_t sliderLedBackgroundFadeOutSpeed = 4;
uint8_t currentSliderLedBackgroundBrightnessRatio = sliderLedBackgroundBrightnessRatio;
uint8_t rippleLeft[NUMBER_OF_SLIDER_SENSORS] = {NUMBER_OF_SLIDER_SENSORS};
uint8_t rippleRight[NUMBER_OF_SLIDER_SENSORS] = {NUMBER_OF_SLIDER_SENSORS};
int8_t rippleLeftChange[NUMBER_OF_SLIDER_SENSORS] = {0};
int8_t rippleRightChange[NUMBER_OF_SLIDER_SENSORS] = {0};
void setupLeds() {
  FastLED.addLeds<WS2812B,LED_PIN,GRB>(leds, NUMBER_OF_SLIDER_SENSORS*LEDS_PER_SENSOR+4*LEDS_PER_BUTTON).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(ledMaxBrightness);
  // readColorSettings();
}
void handleButtonLeds() {
  uint8_t i;
  switch(buttonLedMode){
    case ButtonLedMode::nintendo:
      for(i = 0;i<LEDS_PER_BUTTON;++i){
        leds[i] = CRGB::Red;
      }
      for(;i<LEDS_PER_BUTTON;++i){
        leds[i] = CRGB::Yellow;
      }
      for(;i<LEDS_PER_BUTTON;++i){
        leds[i] = CRGB::Green;
      }
      for(;i<LEDS_PER_BUTTON;++i){
        leds[i] = CRGB::Blue;
      }
      break;
    case ButtonLedMode::sony:
      for(i = 0;i<LEDS_PER_BUTTON;++i){
        leds[i] = CRGB::Red;
      }
      for(;i<LEDS_PER_BUTTON;++i){
        leds[i] = CRGB::Blue;
      }
      for(;i<LEDS_PER_BUTTON;++i){
        leds[i] = CRGB::Purple;
      }
      for(;i<LEDS_PER_BUTTON;++i){
        leds[i] = CRGB::Green;
      }
      break;
    case ButtonLedMode::rainbow:
      fill_rainbow(leds,4*LEDS_PER_BUTTON,autoHue,0);
      break;
    case ButtonLedMode::swirl:
      fill_rainbow(leds,4*LEDS_PER_BUTTON,autoHue,256/5);
      break;
    case ButtonLedMode::custom:
      for(i = 0; i < 4; ++i){
        for(uint8_t j = 0;j<LEDS_PER_BUTTON;++j){
          leds[i*LEDS_PER_BUTTON+j] = customColors[i];
        }
      }
      break;
    case ButtonLedMode::arcade:
    default:
      fill_solid(leds,4*LEDS_PER_BUTTON,CRGB::White);
      break;
  }
  for(i = 0; i < 4*LEDS_PER_BUTTON; ++i){
    leds[i].maximizeBrightness(buttonLedBrightnessRatio);
  }
  if(buttons[CIRCLE]){
    for(i = 0;i<LEDS_PER_BUTTON;++i){
      leds[0*LEDS_PER_BUTTON+i] = CRGB::Black;
    }
  }
  if(buttons[CROSS]){
    for(i = 0;i<LEDS_PER_BUTTON;++i){
      leds[1*LEDS_PER_BUTTON+i] = CRGB::Black;
    }
  }
  if(buttons[SQUARE]){
    for(i = 0;i<LEDS_PER_BUTTON;++i){
      leds[2*LEDS_PER_BUTTON+i] = CRGB::Black;
    }
  }
  if(buttons[TRIANGLE]){
    for(i = 0;i<LEDS_PER_BUTTON;++i){
      leds[3*LEDS_PER_BUTTON+i] = CRGB::Black;
    }
  }
}
void handleSliderLeds(){
  switch(sliderLedMode){
    case SliderLedMode::rainbow:
      fill_rainbow(leds+4*LEDS_PER_BUTTON,NUMBER_OF_SLIDER_SENSORS*LEDS_PER_SENSOR,autoHue,0);
      break;
    case SliderLedMode::swirl:
      fill_rainbow(leds+4*LEDS_PER_BUTTON,NUMBER_OF_SLIDER_SENSORS*LEDS_PER_SENSOR,autoHue,256/(NUMBER_OF_SLIDER_SENSORS*LEDS_PER_SENSOR+1));
      break;
    case SliderLedMode::custom:
      fill_solid(leds+4*LEDS_PER_BUTTON,NUMBER_OF_SLIDER_SENSORS*LEDS_PER_SENSOR,customColors[4]);
      break;
    case SliderLedMode::arcade:
    default:
      fill_solid(leds+4*LEDS_PER_BUTTON,NUMBER_OF_SLIDER_SENSORS*LEDS_PER_SENSOR,CRGB::White);
      break;
  }
  bool isTouching = false;
  uint8_t currentSliderLedBackgroundBreatheBrightnessRatio = currentSliderLedBackgroundBrightnessRatio/4*3+currentSliderLedBackgroundBrightnessRatio*quadwave8(breathe)/255/4;
  for(uint8_t i = 0; i < NUMBER_OF_SLIDER_SENSORS; ++i){    
    if(sliderTouches[i]) {
      switch(sliderLedMode){
        case SliderLedMode::rainbow:
        case SliderLedMode::swirl:
        case SliderLedMode::custom:
          break;
        case SliderLedMode::arcade:
        default:
          for(uint8_t j = 0; j < LEDS_PER_SENSOR; ++j){
            leds[i*LEDS_PER_SENSOR+j+4*LEDS_PER_BUTTON] = CRGB::MediumTurquoise;
          }
          break;
      }
      for(uint8_t j = 0; j < LEDS_PER_SENSOR; ++j){
        leds[i*LEDS_PER_SENSOR+j+4*LEDS_PER_BUTTON].maximizeBrightness(sliderLedTouchBrightnessRatio);
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
    }
    else {
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
      //     leds[i*LEDS_PER_SENSOR+j+4*LEDS_PER_BUTTON] = CRGB::MediumTurquoise;
      //     leds[i*LEDS_PER_SENSOR+j+4*LEDS_PER_BUTTON].maximizeBrightness(sliderLedRippleBrightnessRatio);
      //   }
      // } 
      // else {
        for(uint8_t j = 0; j < LEDS_PER_SENSOR; ++j){
          leds[i*LEDS_PER_SENSOR+j+4*LEDS_PER_BUTTON].maximizeBrightness(currentSliderLedBackgroundBreatheBrightnessRatio);
        }
      // }
    }
  }
  if(isTouching){
    EVERY_N_MILLISECONDS(10){ 
      breathe = 0;
      if(currentSliderLedBackgroundBrightnessRatio < sliderLedBackgroundFadeOutSpeed){
        currentSliderLedBackgroundBrightnessRatio = 0;
      }
      else {
        currentSliderLedBackgroundBrightnessRatio -= sliderLedBackgroundFadeOutSpeed;
      }
    }
  } else {
    EVERY_N_MILLISECONDS(10){
      breathe++; 
      if(breathe>=255){
        breathe=0;
      }
      if(currentSliderLedBackgroundBrightnessRatio > (sliderLedBackgroundBrightnessRatio-sliderLedBackgroundFadeInSpeed)){
        currentSliderLedBackgroundBrightnessRatio = sliderLedBackgroundBrightnessRatio;
      }
      else {
        currentSliderLedBackgroundBrightnessRatio += sliderLedBackgroundFadeInSpeed;
      }
    }
  }
  
}
void readColorSettings(){
  buttonLedMode = ButtonLedMode(EEPROM.read(0));
  sliderLedMode = SliderLedMode(EEPROM.read(1));
  for(uint8_t i = 0; i<5; ++i){
    customColors[i] = CRGB(EEPROM.read(2+i*3),EEPROM.read(3+i*3),EEPROM.read(4+i*3));
  }
}
void nextButtonLedMode(){
  uint8_t newButtonLedModeInt = (static_cast<uint8_t>(buttonLedMode)+1)%(static_cast<uint8_t>(ButtonLedMode::custom)+1);
  buttonLedMode = ButtonLedMode(newButtonLedModeInt);
  EEPROM.write(0,newButtonLedModeInt);
}
void previousButtonLedMode(){
  uint8_t newButtonLedModeInt = (static_cast<uint8_t>(buttonLedMode)-1)%(static_cast<uint8_t>(ButtonLedMode::custom)+1);
  buttonLedMode = ButtonLedMode(newButtonLedModeInt);
  EEPROM.write(0,newButtonLedModeInt);
}
void nextSliderLedMode(){
  uint8_t newSliderLedModeInt = (static_cast<uint8_t>(sliderLedMode)+1)%(static_cast<uint8_t>(SliderLedMode::custom)+1);
  sliderLedMode = SliderLedMode(newSliderLedModeInt);
  EEPROM.write(1,newSliderLedModeInt);
}
void previousSliderLedMode(){
  uint8_t newSliderLedModeInt = (static_cast<uint8_t>(sliderLedMode)-1)%(static_cast<uint8_t>(SliderLedMode::custom)+1);
  sliderLedMode = SliderLedMode(newSliderLedModeInt);
  EEPROM.write(1,newSliderLedModeInt);
}
void changeCustomColors(CRGB newCustomColors[5]){
  for(uint8_t i = 0; i<5; ++i){
    customColors[i] = newCustomColors[i];
    EEPROM.write(2+i*3,customColors[i].red);
    EEPROM.write(3+i*3,customColors[i].green);
    EEPROM.write(4+i*3,customColors[i].blue);
  }
}
void handleLeds() {
  handleButtonLeds();
  handleSliderLeds();
  
  EVERY_N_MILLISECONDS( 20 ) { 
    autoHue++; 
  }
  // EVERY_N_SECONDS( 5 ) { nextButtonLedMode(); nextSliderLedMode(); }
  FastLED.show();
  FastLED.delay(1000/120);
}

/* ----------------------------- LED code - ends ------------------------------- */

void setup() {
  setupLeds();
  setupMatrix();
  setupSlider();

  SetupHardware();
  GlobalInterruptEnable();
}

void loop() {
  readMatrix();
  readSlider();
  handleLeds();
  writeMatrixToReport();
  writeSliderToReport();

  HID_Task();
  USB_USBTask();
}
