#include "SerialSlider.h"

#include "LED.h"
#include "LUFASerial.h"
#include "USB.h"

LUFASerial Serial = LUFASerial(&VirtualSerial_CDC_Interface);

SerialSliderPacket::SerialSliderPacket()
    : cmd(SerialSliderCommand::none), argc(0), args(nullptr) {}
SerialSliderPacket::SerialSliderPacket(char* message)
    : cmd(SerialSliderCommand::message) {
  argc = strlen(message);
  args = new byte[argc];
  for (uint8_t i = 0; i < argc; ++i) {
    args[i] = message[i];
  }
}
SerialSliderPacket::SerialSliderPacket(SerialSliderCommand cmd)
    : cmd(cmd), argc(0), args(nullptr) {}
SerialSliderPacket::SerialSliderPacket(SerialSliderCommand cmd, uint8_t argc,
                                       byte* args)
    : cmd(cmd), argc(argc) {
  this->args = new byte[argc];
  for (uint8_t i = 0; i < argc; ++i) {
    this->args[i] = args[i];
  }
}
SerialSliderPacket::~SerialSliderPacket() {
  delete[] args;
}

static bool SerialSliderPacket::readAndDecodeByte(byte& destination) {
  int read = -1;
  if (Serial.peek() != SYNC) {
    read = Serial.read();
  }
  if (read < 0) {
    return false;
  } else if (read == ESC) {
    int escaped = -1;
    if (Serial.peek() != SYNC) {
      escaped = Serial.read();
    }
    if (escaped < 0) {
      return false;
    } else {
      destination = escaped + 1;
    }
  } else {
    destination = read;
  }
  return true;
}

static uint16_t SerialSliderPacket::encodeByte(byte* destination,
                                               uint16_t position, byte plain) {
  switch (plain) {
    case SYNC:
    case ESC:
      destination[position] = ESC;
      destination[position + 1] = plain - 1;
      return position + 2;
    default:
      destination[position] = plain;
      return position + 1;
  }
}

void SerialSliderPacket::write() {
  byte encoded[MAX_RAW_LENGTH];

  encoded[0] = SYNC;

  uint16_t position = 1;

  position = encodeByte(encoded, position, static_cast<byte>(cmd));

  position = encodeByte(encoded, position, argc);

  uint8_t checksum = -SYNC - static_cast<byte>(cmd) - argc;

  for (uint8_t i = 0; i < argc; ++i) {
    checksum -= args[i];
    position = encodeByte(encoded, position, args[i]);
  }

  position = encodeByte(encoded, position, checksum);

  // Serial1.print("Write: ");
  // for (uint16_t i = 0; i < position; ++i) {
  //   Serial1.print(encoded[i], HEX);
  //   Serial1.print(" ");
  // }
  // Serial1.println();

  // if (Serial.availableForWrite() >= position) {
  Serial.write(encoded, position);
  // }
}

static SerialSliderPacket* SerialSliderPacket::read() {
  byte cmd;
  uint8_t argc;
  byte* args;
  uint8_t checksum;
  uint8_t sum = 0;

  do {
    if (!Serial.available()) {
      return new SerialSliderPacket("no SYNC");
    }
  } while (Serial.read() != SYNC);

  sum += SYNC;

  if (!readAndDecodeByte(cmd)) {
    return new SerialSliderPacket("no cmd");
  }
  sum += cmd;
  // Serial1.print("cmd: ");
  // Serial1.println(cmd);

  if (!readAndDecodeByte(argc)) {
    return new SerialSliderPacket("no argc");
  }
  sum += argc;
  args = new byte[argc];
  // Serial1.print("argc: ");
  // Serial1.println(argc);

  for (uint8_t i = 0; i < argc; ++i) {
    if (!readAndDecodeByte(args[i])) {
      return new SerialSliderPacket("incomplete");
    }
    sum += args[i];
  }

  if (!readAndDecodeByte(checksum)) {
    return new SerialSliderPacket("no checksum");
  }
  sum += checksum;
  // Serial1.print("checksum: ");
  // Serial1.println(checksum);

  if (sum != 0) {
    return new SerialSliderPacket("incorrect checksum");
  }

  return new SerialSliderPacket(static_cast<SerialSliderCommand>(cmd), argc,
                                args);
}

static const byte HWInfoArgs[18] = {'1', '5', '2',  '7',  '5',  ' ',
                                    ' ', ' ', 0xa0, '0',  '6',  '6',
                                    '8', '7', 0xff, 0x90, 0x00, 0x64};
static const SerialSliderPacket SerialSliderPacket::HWInfo =
    SerialSliderPacket(SerialSliderCommand::getHWInfo, 18, HWInfoArgs);
static const byte WrongChecksumArgs[2] = {0xff, 0x01};
static const SerialSliderPacket SerialSliderPacket::WrongChecksum =
    SerialSliderPacket(SerialSliderCommand::exception, 2, WrongChecksumArgs);

bool serialSliderReportEnabled;
byte serialSliderReport[NUMBER_OF_REPORT_SLIDER_SENSORS] = {0};
unsigned long resetTimerStart;

void setupSerialSlider() {
  // Serial.begin(115200);
  serialSliderReportEnabled = false;
  resetSerialSliderLeds();
  resetTimerStart = millis();
}

void handleSerialSliderLeds(SerialSliderPacket* packet) {
  if (sliderLedMode == SliderLedMode::arcadeSerial &&
      packet->cmd == SerialSliderCommand::ledReport) {
    fill_solid(sliderLeds, NUMBER_OF_SLIDER_LEDS, CRGB::Black);
    for (uint8_t i = 0; i < NUMBER_OF_SLIDER_SENSORS; ++i) {
      if (3 * i + 3 >= packet->argc) break;
      fill_solid(sliderLeds + i * LEDS_PER_SENSOR, LEDS_PER_SENSOR,
                 CRGB(packet->args[3 * i + 2], packet->args[3 * i + 3],
                      packet->args[3 * i + 1]));
    }
    if (packet->argc > 0) {
      if (packet->args[0] < MAX_SLIDER_BRIGHTNESS) {
        for (uint8_t i = 0; i < NUMBER_OF_SLIDER_LEDS; ++i) {
          sliderLeds[i].nscale8(packet->args[0] * 4);
        }
      }
    }
    FastLED.show();
  }
}

void resetSerialSliderLeds() {
  if (sliderLedMode == SliderLedMode::arcadeSerial) {
    fill_solid(sliderLeds, NUMBER_OF_SLIDER_LEDS, CRGB::Black);
    FastLED.show();
  }
}

void writeSliderToSerialReport() {
  uint8_t bitPosition = 0;
  for (uint8_t i = 0; i < NUMBER_OF_SLIDER_SENSORS; ++i) {
    for (uint8_t j = 0; j < sliderReportPositionsPerSensor[i]; ++j) {
      serialSliderReport[bitPosition] =
          sliderTouches[i] ? SERIAL_SLIDER_SENSOR_ON : SERIAL_SLIDER_SENSOR_OFF;
      bitPosition++;
    }
  }
}

void handleSerialSlider() {
  if (Serial.available()) {
    SerialSliderPacket* incoming = SerialSliderPacket::read();
    switch (incoming->cmd) {
      case SerialSliderCommand::ledReport:
        handleSerialSliderLeds(incoming);
        break;
      case SerialSliderCommand::enableSliderReport:
        serialSliderReportEnabled = true;
        writeSliderToSerialReport();
        SerialSliderPacket(SerialSliderCommand::sliderReport,
                           NUMBER_OF_REPORT_SLIDER_SENSORS, serialSliderReport)
            .write();
        break;
      case SerialSliderCommand::disableSliderReport:
        serialSliderReportEnabled = false;
        SerialSliderPacket(SerialSliderCommand::disableSliderReport).write();
        break;
      case SerialSliderCommand::unknown_0x09:
      case SerialSliderCommand::unknown_0x0a:
        SerialSliderPacket(incoming->cmd).write();
        break;
      case SerialSliderCommand::reset:
        serialSliderReportEnabled = false;
        resetSerialSliderLeds();
        SerialSliderPacket(SerialSliderCommand::reset).write();
        break;
      case SerialSliderCommand::exception:
        break;
      case SerialSliderCommand::getHWInfo:
        SerialSliderPacket::HWInfo.write();
        break;
      case SerialSliderCommand::none:
        // incoming->write();
        // SerialSliderPacket::WrongChecksum.write();
        break;
      case SerialSliderCommand::message:
        // Serial1.write(incoming->args, incoming->argc);
        // Serial1.println();
        break;
      default:
        break;
    }
    delete incoming;
    incoming = nullptr;
    resetTimerStart = millis();
  } else if (millis() - resetTimerStart > 10000) {
    serialSliderReportEnabled = false;
    resetSerialSliderLeds();
    resetTimerStart = millis();
  }
  EVERY_N_MILLISECONDS(12) {
    if (serialSliderReportEnabled) {
      // Serial1.print("Report enabled: ");
      // Serial1.println(serialSliderReportEnabled);
      writeSliderToSerialReport();
      SerialSliderPacket(SerialSliderCommand::sliderReport,
                         NUMBER_OF_REPORT_SLIDER_SENSORS, serialSliderReport)
          .write();
    }
  }
}