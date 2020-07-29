#include <Arduino.h>
#include <stdint.h>

#include "Slider.h"

#define SYNC 0xff
#define ESC 0xfd
#define MAX_ARGC NUMBER_OF_REPORT_SLIDER_SENSORS * 3 + 1
#define MAX_RAW_LENGTH (MAX_ARGC + 3) * 2 + 1
#define SERIAL_SLIDER_SENSOR_OFF 0x00
#define SERIAL_SLIDER_SENSOR_ON 0xfe
#define MAX_SLIDER_BRIGHTNESS 0x3f
#define RESET_TIMEOUT 10  // seconds

enum class SerialSliderCommand {
  none = 0x00,
  sliderReport = 0x01,
  ledReport = 0x02,
  enableSliderReport = 0x03,
  disableSliderReport = 0x04,
  unknown_0x09 = 0x09,
  unknown_0x0a = 0x0a,
  reset = 0x10,
  exception = 0xee,
  getHWInfo = 0xf0,
  message = 0xfe,
};

class SerialSliderPacket {
 private:
  static bool readAndDecodeByte(byte& destination);
  static uint16_t encodeByte(byte* destination, uint16_t position, byte plain);

 public:
  SerialSliderCommand cmd;
  uint8_t argc;
  byte* args;

  SerialSliderPacket();
  SerialSliderPacket(char* message);
  SerialSliderPacket(SerialSliderCommand cmd);
  SerialSliderPacket(SerialSliderCommand cmd, uint8_t argc, byte* args);
  ~SerialSliderPacket();

  static const SerialSliderPacket HWInfo;
  static const SerialSliderPacket WrongChecksum;

  void write();
  static SerialSliderPacket* read();
};

void setupSerialSlider();
void handleSerialSliderLeds(SerialSliderPacket* packet);
void resetSerialSliderLeds();
void writeSliderToSerialReport();
void handleSerialSlider();