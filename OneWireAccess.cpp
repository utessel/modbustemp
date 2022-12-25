#include <OneWire.h>
#include "OneWireAccess.h"
#include "config.h"
#include "utilities.h"

static OneWire  ds(ONE_WIRE_PIN);

void OneWireAccess::setup()
{
  for (int i = 0; i < ArrayLength(inputBuffer.ow); i++)
    Clear(inputBuffer.ow[i]);
}

void OneWireAccess::Clear(OneWireData & data)
{
  for (int j = 0; j < 8; j++) data.addr[j] = 0xFF;
  data.count = 0;
  data.temp = -1000.0;
}

void OneWireAccess::ReadAddress(OneWireData & data)
{
  if (!ds.search(data.addr))
  {
    Clear(data);
    pos = 0;
    state = 0;
    ds.reset_search();
    return;
  }

  ds.reset();
  ds.select(data.addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
}

void OneWireAccess::ReadData(OneWireData & data)
{
  uint8_t raw[9];

  ds.reset();
  ds.select(data.addr);
  ds.write(0xBE);         // Read Scratchpad

  for (int i = 0; i < 9; i++) {           // we need 9 bytes
    raw[i] = ds.read();
  }

  uint8_t crc = OneWire::crc8(raw, 8);
  if (crc != raw[8]) return;

  Convert( data, raw );
}

void OneWireAccess::Next()
{
  state = 0;

  pos++;
  if (pos > ArrayLength(inputBuffer.ow))
  {
    pos = 0;
    ds.reset_search();
  }
}

void OneWireAccess::Convert( OneWireData & item, const uint8_t * data )
{
  uint8_t type_s;

  switch (item.addr[0]) {
    case 0x10: type_s = 1; break;
    case 0x28: type_s = 0; break;
    case 0x22: type_s = 0; break;
    default:  return;
  }

  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    uint8_t cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }

  item.temp = raw * 0.0625;
  item.count ++;

}

void OneWireAccess::tick()
{
  if (!ticker.NewTick(250, millis())) return;

  OneWireData & data = inputBuffer.ow[pos];

  switch (state++)
  {
    case 0:
      ReadAddress(data);
      break;

    case 1:
    case 2:
    case 3:
    case 4:
      // just wait
      break;

    case 5:
      ReadData(data);
      Next();
      break;
  }
}
