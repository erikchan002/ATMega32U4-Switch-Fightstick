#include "Slider.h"

#include <Wire.h>

#include "USB.h"

bool sliderTouches[NUMBER_OF_SLIDER_SENSORS] = {0};
uint8_t sliderReportPositionsPerSensor[NUMBER_OF_SLIDER_SENSORS];
void setupSlider() {
  for (uint8_t i = 0; i < NUMBER_OF_SLIDER_SENSORS; ++i) {
    sliderReportPositionsPerSensor[i] =
        NUMBER_OF_REPORT_SLIDER_SENSORS / NUMBER_OF_SLIDER_SENSORS;
  }
  uint8_t extraPositions =
      NUMBER_OF_REPORT_SLIDER_SENSORS % NUMBER_OF_SLIDER_SENSORS;
  for (uint8_t i = 0;
       i < NUMBER_OF_REPORT_SLIDER_SENSORS / 2 && extraPositions > 0; ++i) {
    sliderReportPositionsPerSensor[i]++;
    extraPositions--;
    if (extraPositions == 0) break;
    sliderReportPositionsPerSensor[NUMBER_OF_SLIDER_SENSORS - i]++;
    extraPositions--;
  }
  Wire.setClock(400000L);
  Wire.begin();
}
void readSlider() {
  Wire.requestFrom(SLIDER_I2C_ADDRESS, BUFFER_SIZE);
  uint8_t numberOfSensorsRead = 0;
  for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
    if (!Wire.available()) break;
    uint8_t incoming = Wire.read();
    for (uint8_t j = 0; j < 8; ++j) {
      if (numberOfSensorsRead >= NUMBER_OF_SLIDER_SENSORS) break;
      sliderTouches[numberOfSensorsRead] = (incoming >> j) & 1;
      ++numberOfSensorsRead;
    }
  }
}
void writeSliderToReport() {
  uint8_t sticks[4] = {0};
  uint8_t bitPosition = NUMBER_OF_REPORT_SLIDER_SENSORS - 1;
  for (uint8_t i = 0; i < NUMBER_OF_SLIDER_SENSORS; ++i) {
    for (uint8_t j = 0; j < sliderReportPositionsPerSensor[i]; ++j) {
      if (sliderTouches[i]) {
        sticks[bitPosition / 8] |= (1 << (bitPosition % 8));
      }
      bitPosition--;
    }
  }
  ReportData.LX = sticks[0] ^ 0x80;
  ReportData.LY = sticks[1] ^ 0x80;
  ReportData.RX = sticks[2] ^ 0x80;
  ReportData.RY = sticks[3] ^ 0x80;
}