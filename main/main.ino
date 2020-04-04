#include "LUFAConfig.h"
#include <LUFA.h>
#include "Joystick.h"
#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h>
#include <Wire.h>
#include <FastLED.h>
#include <EEPROM.h>

#define DEBOUNCE_INTERVAL 1 //ms

class Button {
  private:
    const static uint16_t debounceInterval = DEBOUNCE_INTERVAL;
    uint16_t mask;
    Bounce bounce;
    bool attached;
  public:
    bool status;
    Button(uint16_t mask=0):mask(mask),bounce(Bounce()),attached(false){
      this->bounce.interval(Button::debounceInterval);
    }
    void attach(uint8_t pin){
      this->attached = true;
      this->bounce.attach(pin, INPUT_PULLUP);
    }
    void disable(){
      this->attached = false;
    }
    void read(){
      if (this->attached && this->bounce.update()) this->status = this->bounce.fell();
    }
    USB_JoystickReport_Input_t write(USB_JoystickReport_Input_t &report){
      if(this->status) report.Button |= this->mask;
      return report;
    }
};

#define DPAD_UP_MASK        0x00
#define DPAD_UPRIGHT_MASK   0x01
#define DPAD_RIGHT_MASK     0x02
#define DPAD_DOWNRIGHT_MASK 0x03
#define DPAD_DOWN_MASK      0x04
#define DPAD_DOWNLEFT_MASK  0x05
#define DPAD_LEFT_MASK      0x06
#define DPAD_UPLEFT_MASK    0x07
#define DPAD_NONE_MASK    0x08
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
class Dpad {
  private:
    const static uint16_t debounceInterval = DEBOUNCE_INTERVAL;
    Bounce bounce[4];
    bool attached;
  public:
    bool status[4];
    Dpad():bounce{Bounce(),Bounce(),Bounce(),Bounce()},attached(false){
    }
    void attach(uint8_t pinUp, uint8_t pinDown, uint8_t pinLeft, uint8_t pinRight){
      this->attached = true;
      this->bounce[UP].attach(pinUp);
      this->bounce[DOWN].attach(pinDown);
      this->bounce[LEFT].attach(pinLeft);
      this->bounce[RIGHT].attach(pinRight);
    }
    void disable(){
      this->attached = false;
    }
    void read(){
      for (uint8_t i=0;i<4;++i){
        if (this->attached && this->bounce[i].update()) this->status[i] = this->bounce[i].fell();
      }
    }
    USB_JoystickReport_Input_t write(USB_JoystickReport_Input_t &report){
      report.HAT = DPAD_NONE_MASK;
      if(this->status[RIGHT]) report.HAT = DPAD_RIGHT_MASK;
      if(this->status[DOWN]) report.HAT = DPAD_DOWN_MASK;
      if(this->status[LEFT]) report.HAT = DPAD_LEFT_MASK;
      if(this->status[UP]) report.HAT = DPAD_UP_MASK;
      if(this->status[DOWN] && this->status[RIGHT]) report.HAT = DPAD_DOWNRIGHT_MASK;
      if(this->status[DOWN] && this->status[LEFT]) report.HAT = DPAD_DOWNLEFT_MASK;
      if(this->status[UP] && this->status[RIGHT]) report.HAT = DPAD_UPRIGHT_MASK;
      if(this->status[UP] && this->status[LEFT]) report.HAT = DPAD_UPLEFT_MASK;
      return report;
    }
};

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

#define NUMBER_OF_BUTTONS 14
#define A 0
#define B 1
#define X 2
#define Y 3
#define L 4
#define R 5
#define ZL 6
#define ZR 7
#define L_STICK 8
#define R_STICK 9
#define PLUS 10
#define MINUS 11
#define HOME 12
#define CAPTURE 13
Button buttons[NUMBER_OF_BUTTONS]{
  A_MASK,
  B_MASK,
  X_MASK,
  Y_MASK,
  L_MASK,
  R_MASK,
  ZL_MASK,
  ZR_MASK,
  L_STICK_MASK,
  R_STICK_MASK,
  PLUS_MASK,
  MINUS_MASK,
  HOME_MASK,
  CAPTURE_MASK
};
Dpad dpad;

#define SLIDER_I2C_ADDRESS 0x08
#define NUMBER_OF_SLIDER_SENSORS 30
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

#define LEDS_PER_SENSOR 1
#define LEDS_PER_BUTTON 1
CRGB leds[NUMBER_OF_SLIDER_SENSORS*LEDS_PER_SENSOR+4*LEDS_PER_BUTTON];
enum class ButtonLedMode { arcade, nintendo, sony, rainbow, swirl, custom } buttonLedMode;
enum class SliderLedMode { arcade, rainbow, swirl, custom } sliderLedMode;
uint8_t autoHue = 0;
uint16_t breatheAngle = 0;
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

void setup() {
  setupLeds();
  setupButtons();
  setupDpad();
  setupSlider();

  SetupHardware();
  GlobalInterruptEnable();
}

void readAll(){
  for (uint8_t i=0;i<NUMBER_OF_BUTTONS;++i) buttons[i].read();
  dpad.read();
  readSlider();
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
  if(buttons[A].status){
    for(i = 0;i<LEDS_PER_BUTTON;++i){
      leds[0*LEDS_PER_BUTTON+i] = CRGB::Black;
    }
  }
  if(buttons[B].status){
    for(i = 0;i<LEDS_PER_BUTTON;++i){
      leds[1*LEDS_PER_BUTTON+i] = CRGB::Black;
    }
  }
  if(buttons[Y].status){
    for(i = 0;i<LEDS_PER_BUTTON;++i){
      leds[2*LEDS_PER_BUTTON+i] = CRGB::Black;
    }
  }
  if(buttons[X].status){
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
          leds[i*LEDS_PER_SENSOR+j+4*LEDS_PER_BUTTON].maximizeBrightness(currentSliderLedBackgroundBrightnessRatio/4*3+currentSliderLedBackgroundBrightnessRatio*sin(radians(breatheAngle))/4);
        }
      // }
    }
  }
  if(isTouching){
    EVERY_N_MILLISECONDS(10){ 
      breatheAngle = 0;
      if(currentSliderLedBackgroundBrightnessRatio < sliderLedBackgroundFadeOutSpeed){
        currentSliderLedBackgroundBrightnessRatio = 0;
      }
      else {
        currentSliderLedBackgroundBrightnessRatio -= sliderLedBackgroundFadeOutSpeed;
      }
    }
  } else {
    EVERY_N_MILLISECONDS(10){
      breatheAngle++; 
      if(breatheAngle>=360){
        breatheAngle-=360;
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

void writeReport(){
  for (uint8_t i=0;i<NUMBER_OF_BUTTONS;++i) buttons[i].write(ReportData);
  dpad.write(ReportData);
  writeSliderToReport();
}

/*------------------------------------------------------------*/
/* setup your pins here */

void setupButtons(){
  buttons[A].attach(4);
  buttons[B].attach(5);
  buttons[Y].attach(6);
  buttons[X].attach(7);
}

void setupDpad(){

}

#define LED_PIN 18

void setupLeds() {
  FastLED.addLeds<WS2812B,LED_PIN,GRB>(leds, NUMBER_OF_SLIDER_SENSORS*LEDS_PER_SENSOR+4*LEDS_PER_BUTTON).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(ledMaxBrightness);
  // readColorSettings();
}

/*------------------------------------------------------------*/

void loop() {
  readAll();
  handleLeds();
  writeReport();

  HID_Task();
  USB_USBTask();
}
