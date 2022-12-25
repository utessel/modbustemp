#include "modbus.h"
#include "Arduino.h"
#include "config.h"
// -------------------------------------------------------------------

void Modbus::setup()
{
  modbusSerial.begin(19200);
  modbusSerial.setTimeout(0);

#ifdef MODBUS_RE_PIN
  pinMode( MODBUS_RE_PIN, OUTPUT ); digitalWrite(MODBUS_RE_PIN, LOW );  // RE on
#endif

#ifdef MODBUS_DE_PIN
  pinMode( MODBUS_DE_PIN, OUTPUT ); digitalWrite(MODBUS_DE_PIN, LOW );  // DE off
#endif
}
// -------------------------------------------------------------------

void Modbus::tick()
{
  unsigned long current = micros();
  if (modbusSerial.available()) {
    Data();
    ticker.last = current;
  } else {
    if (ticker.NewTick(2000, current)) // at 19200, about 2ms
      Timeout();
  }
}
// -------------------------------------------------------------------

void Modbus::Timeout()
{
  if (len > 3) CheckMessage();
  len = 0;
}
// -------------------------------------------------------------------

void Modbus::Data()
{
  if (len >= sizeof(rxBuf))
  {
    // overrun
    len = 0;
  }
  int bytes = modbusSerial.readBytes(&rxBuf[len], sizeof(rxBuf) - len);
  len += bytes;
}
// -------------------------------------------------------------------

static uint16_t CRC16(const void * raw, unsigned int len, uint16_t pre = 0xFFFF)
{
  uint16_t result = pre;
  const uint8_t * buf = static_cast<const uint8_t*>(raw);

  for (; len > 0; len--, buf++)
  {
    result ^= *buf;
    for (char i = 8; i > 0; i--)
    {
      int bit = (result & 0x0001);
      result >>= 1;
      if (bit) result ^= 0xA001;
    }
  }
  return result;
}
// -------------------------------------------------------------------

void Modbus::SendRaw(const void * raw, uint8_t count)
{
  modbusSerial.write(static_cast<const uint8_t*>(raw), count);
}
// -------------------------------------------------------------------

void Modbus::Send(const void * header, uint8_t headerSize, const void * data, uint8_t dataSize)
{
  uint16_t crc = CRC16(header, headerSize);
  if (dataSize) crc = CRC16(data, dataSize, crc);

  // enable transmitter, disable receiver
#ifdef MODBUS_RE_PIN
  digitalWrite( MODBUS_RE_PIN, HIGH );  // RE off
#endif

#ifdef MODBUS_DE_PIN
  digitalWrite( MODBUS_DE_PIN, HIGH );  // DE on
#endif

  SendRaw(header, headerSize);
  if (dataSize) SendRaw( data, dataSize );
  SendRaw( &crc, 2 );

  modbusSerial.flush();

#ifdef MODBUS_RE_PIN
  digitalWrite( MODBUS_RE_PIN, LOW );  // RE on
#endif

#ifdef MODBUS_DE_PIN
  digitalWrite( MODBUS_DE_PIN, LOW );  // DE off
#endif
}
// -------------------------------------------------------------------

void Modbus::CheckMessage()
{
  // todo: what about broadcast?
  if (rxBuf[0] != OwnId) return;

  uint16_t rxedCRC = CRC16(&rxBuf[0], len - 2);
  if (rxedCRC != *reinterpret_cast<uint16_t*>(&rxBuf[len - 2]))
  {
    // invalid crc: ignore
    return;
  }

  inputBuffer.modbusTick++;

  switch (rxBuf[1])
  {
    case 0x3: ReadHoldingRegisters( BEValue(2), BEValue(4) ); break;
    case 0x4: ReadInputRegisters( BEValue(2), BEValue(4) ); break;
    case 0x6: WriteSingleRegister( BEValue(2), LEValue(4) ); break;
    case 0x11: Diagnostics(); break;
    default: Exception(ILLEGAL_FUNCTION); break;
  }
}
// -------------------------------------------------------------------

void Modbus::ReadHoldingRegisters( uint16_t offset, uint8_t count)
{
  const uint16_t * data = reinterpret_cast<const uint16_t*>(&dataBuffer);
  if (offset + 2 * count > sizeof(dataBuffer))
  {
    Exception( ILLEGAL_DATA_ADDRESS );
    return;
  }

  uint8_t header[3];

  header[0] = OwnId;
  header[1] = 0x3;
  header[2] = count * 2;

  Send( &header[0], sizeof(header), &data[offset], count * 2 );
}
// -------------------------------------------------------------------

void Modbus::ReadInputRegisters( uint16_t offset, uint8_t count)
{
  const uint16_t * data = reinterpret_cast<const uint16_t*>(&inputBuffer);
  if (offset + 2 * count > sizeof(inputBuffer))
  {
    Exception( ILLEGAL_DATA_ADDRESS );
    return;
  }
  uint8_t header[3];

  header[0] = OwnId;
  header[1] = 0x4;
  header[2] = count * 2;

  Send( &header, sizeof(header), &data[offset], count * 2 );
}
// -------------------------------------------------------------------

void Modbus::Diagnostics()
{
  uint8_t msg[] = {
    OwnId,
    0x11,
    0x08,

    0xAB,
    0xFF,
    'U', 'N', 'I', 'K', 'A', 'T',
  };

  Send( msg );
}
// -------------------------------------------------------------------

void Modbus::WriteSingleRegister( uint16_t offset, uint16_t value)
{
  uint16_t * data = reinterpret_cast<uint16_t*>(&dataBuffer);

  if (offset + 2 > sizeof(dataBuffer))
  {
    Exception( ILLEGAL_DATA_ADDRESS );
    return;
  }

  data[offset] = value;

  uint8_t msg[] = {
    OwnId,
    0x06,

    static_cast<uint8_t>(offset & 0xFF), 
    static_cast<uint8_t>(offset >> 8),
    static_cast<uint8_t>(value >> 8), 
    static_cast<uint8_t>(value & 0xFF)
  };

  Send( msg );
}
// -------------------------------------------------------------------

void Modbus::Exception( uint8_t code )
{
  uint8_t msg[] = {
    OwnId,
    static_cast<uint8_t>(rxBuf[1] | 0x80),
    code
  };

  Send( msg );
}
// -------------------------------------------------------------------
