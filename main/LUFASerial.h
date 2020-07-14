#include <LUFA.h>

#define READ_BUFFER_SIZE 256

class LUFASerial {
 private:
  const USB_ClassInfo_CDC_Device_t* cdcInterfaceInfo;
  byte buffer[READ_BUFFER_SIZE];
  int16_t front;
  int16_t rear;
  void accept();
  uint16_t bufferLength();

 public:
  LUFASerial(USB_ClassInfo_CDC_Device_t*);
  int available();
  int peek();
  int read();
  size_t write(byte single);
  size_t write(byte buffer[], uint16_t length);
  size_t write(char string[]);
};