#include "LUFAConfig.h"
#include <LUFA.h>
#include "Joystick.h"
#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h>

#define DEBOUNCE_INTERVAL 1 //ms

class Button {
  private:
    const static uint16_t debounceInterval = DEBOUNCE_INTERVAL;
    uint16_t mask;
    Bounce bounce;
    bool attached;
  public:
    byte status;
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
    byte status[4];
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
      for (int i=0;i<4;++i){
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

#define NUMBER_OF_SLIDER_ELEMENTS 31
uint32_t slider = 0;

void readAllEnabled(){
  for (int i=0;i<NUMBER_OF_BUTTONS;++i) buttons[i].read();
  dpad.read();
}

void readSlider() {
  while(Serial1.read()!=0xFF);
  uint32_t positions = 0;

  byte incoming[NUMBER_OF_SLIDER_ELEMENTS/7+1] = {0};
  Serial1.readBytes(incoming, NUMBER_OF_SLIDER_ELEMENTS/7+1);
  
  for(int i=0; i <= NUMBER_OF_SLIDER_ELEMENTS/7; ++i){
    positions |= ((uint32_t)incoming[i] << ((NUMBER_OF_SLIDER_ELEMENTS/7-i)*7));
  }

  slider = positions;
}

void writeReport(){
  for (int i=0;i<NUMBER_OF_BUTTONS;++i) buttons[i].write(ReportData);
  dpad.write(ReportData);

  ReportData.LX = (slider & 0xFF) ^ 0x80;
  ReportData.LY = ((slider>>8) & 0xFF) ^ 0x80;
  ReportData.RX = ((slider>>15) & 0xFF) ^ 0x80;
  ReportData.RY = ((slider>>23) & 0xFF) ^ 0x80;
}

void setup() {
  buttons[A].attach(4);
  buttons[B].attach(5);
  buttons[Y].attach(6);
  buttons[X].attach(7);

  Serial1.begin(115200);

  SetupHardware();
  GlobalInterruptEnable();
}

void loop() {
  readAllEnabled();
  readSlider();
  writeReport();

  HID_Task();
  USB_USBTask();
}
