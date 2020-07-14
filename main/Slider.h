#include <stdint.h>

#define SLIDER_I2C_ADDRESS 0x08
#define NUMBER_OF_SLIDER_SENSORS 32
#define NUMBER_OF_REPORT_SLIDER_SENSORS 32
#define BUFFER_SIZE (NUMBER_OF_SLIDER_SENSORS + 7) / 8

extern bool sliderTouches[NUMBER_OF_SLIDER_SENSORS];
extern uint8_t sliderReportPositionsPerSensor[NUMBER_OF_SLIDER_SENSORS];

void setupSlider();
void readSlider();
void writeSliderToReport();