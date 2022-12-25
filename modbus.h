#pragma once

#include "utilities.h"

class Modbus {
  public:
    Modbus(uint8_t id): OwnId{id} {}
    void setup();
    void tick();

  private:  
    const uint8_t ILLEGAL_FUNCTION = 0x01;
    const uint8_t ILLEGAL_DATA_ADDRESS = 0x02;

    Ticker ticker;
    uint8_t rxBuf[32];

    uint16_t LEValue(uint8_t index)
    {
      return rxBuf[index] | rxBuf[index + 1] << 8;
    }

    uint16_t BEValue(uint8_t index)
    {
      return rxBuf[index + 1] | rxBuf[index] << 8;
    }

    char len { 0 };

    uint8_t OwnId { 1 };


    void Data();
    void Timeout();

    void CheckMessage();

    void ReadHoldingRegisters(uint16_t offset, uint8_t count);
    void ReadInputRegisters(uint16_t offset, uint8_t count);
    void WriteSingleRegister(uint16_t offset, uint16_t data);
    void Diagnostics();
    void Exception(uint8_t code);

    void SendRaw(const void * raw, uint8_t count);

    void Send(const void * header, uint8_t headerSize, const void * data = nullptr, uint8_t dataSize = 0);

    template <typename T>
    void Send(const T & msg)
    {
      Send( &msg, sizeof(msg) );
    }
};
