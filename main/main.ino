#include <LUFA.h>

#include "ButtonMatrix.h"
#include "LED.h"
#include "LUFAConfig.h"
#include "SerialSlider.h"
#include "Slider.h"
//#include "Timer.h"
#include "USB.h"

void setup() {
  setupLeds();
  setupMatrix();
  setupSlider();
  setupSerialSlider();
  // setupTimer();

  SetupHardware();
  GlobalInterruptEnable();
}

void loop() {
  readMatrix();
  readSlider();
  handleLedSettingsChange();
  handleLeds();
  handleSerialSlider();
  writeMatrixToReport();
  writeSliderToReport();

  HID_Task();
  CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
  USB_USBTask();

  // timer();
}
