#include "LUFASerial.h"

LUFASerial::LUFASerial(USB_ClassInfo_CDC_Device_t* cdcInterfaceInfo)
    : cdcInterfaceInfo(cdcInterfaceInfo), front(-1), rear(0) {}

void LUFASerial::accept() {
  for (uint16_t i = 0; i < READ_BUFFER_SIZE - bufferLength(); ++i) {
    int16_t incoming = CDC_Device_ReceiveByte(cdcInterfaceInfo);
    if (incoming < 0) break;
    buffer[rear] = incoming;
    if (front == -1) front = rear;
    // Serial1.print("Accept: ");
    // Serial1.println(incoming, HEX);
    rear = static_cast<int16_t>(rear + 1) % READ_BUFFER_SIZE;
  }
}

uint16_t LUFASerial::bufferLength() {
  if (front == -1) return 0;
  if (rear == front) return READ_BUFFER_SIZE;
  return static_cast<int16_t>(rear - front + READ_BUFFER_SIZE) %
         READ_BUFFER_SIZE;
}

int LUFASerial::available() {
  accept();
  return bufferLength();
}

int LUFASerial::peek() {
  if (!available()) return -1;
  return buffer[front];
}

int LUFASerial::read() {
  int next = peek();
  if (next != -1) {
    front = static_cast<int16_t>(front + 1) % READ_BUFFER_SIZE;
    if (front == rear) {
      front = -1;
    }
  }
  // Serial1.print("Read from buffer: ");
  // Serial1.println(next, HEX);
  return next;
}

size_t LUFASerial::write(byte datum) {
  if (cdcInterfaceInfo->State.ControlLineStates.HostToDevice > 0) {
    uint8_t result = CDC_Device_SendByte(cdcInterfaceInfo, datum);
    if (result == ENDPOINT_READYWAIT_NoError) {
      return 1;
    }
  }
  return 0;
}

size_t LUFASerial::write(byte buffer[], uint16_t length) {
  // Serial1.print("CDC Write: ");
  // Serial1.write(buffer, length);
  // Serial1.println();

  uint16_t count = 0;
  for (uint16_t count = 0; count < length; ++count) {
    if (!write(buffer[count])) {
      return count;
    }
  }
  return count;

  // if (cdcInterfaceInfo->State.ControlLineStates.HostToDevice > 0) {
  //   uint8_t result = CDC_Device_SendData(cdcInterfaceInfo, buffer, length);
  //   if (result == ENDPOINT_RWSTREAM_NoError) {
  //     return length;
  //   }
  // }
  // return 0;
}

size_t LUFASerial::write(char string[]) {
  return write(string, strlen(string));
}
