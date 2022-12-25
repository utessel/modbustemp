#pragma once

#include <stdint.h>

struct DataBuffer {
  uint8_t whatever[8];
};

struct OneWireData {
  uint8_t addr[8]; // 64 Bit ROM ID -> 48 Bit are "serial number"
  uint16_t count;  // "timestamp" of measurement
  float temp;     // measured value in degree Celsius
};

struct InputBuffer {
  uint16_t displayTick;
  uint16_t modbusTick;
  float temp;
  float pressure;
  float hum;
  OneWireData ow[8];
};

extern DataBuffer dataBuffer;
extern InputBuffer inputBuffer;

//#define MODBUS_RE_PIN 3
//#define MODBUS_DE_PIN 4
#define MODBUS_BAUD 19200
#define modbusSerial Serial

#define ONE_WIRE_PIN 2
