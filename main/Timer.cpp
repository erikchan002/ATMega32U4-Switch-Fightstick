#include "Timer.h"

#include <Arduino.h>

unsigned long last;
unsigned long count;
unsigned long min;
unsigned long max;
unsigned long total;

void setupTimer() {
  Serial1.begin(115200);
  last = micros();
  count = 0;
  min = 1000000;
  max = 0;
  total = 0;
}

void timer() {
  ++count;
  unsigned long now = micros();
  unsigned long single = now - last;
  last = now;
  if (single < min) min = single;
  if (single > max) max = single;
  total += single;
  if (total >= REPORT_EVERY * 1000000) {
    unsigned long meanTime = total / count;
    unsigned long meanRate = count * 1000000 / total;
    char message[256];
    sprintf(
        message,
        "Min cycle time %lu microseconds\nMax cycle time %lu "
        "microseconds\nMean cycle time %lu microseconds\nMean rate %luHz\n\n",
        min, max, meanTime, meanRate);
    Serial1.print(message);
    count = 0;
    min = 1000000;
    max = 0;
    total = 0;
  }
}