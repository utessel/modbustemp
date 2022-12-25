#include <Adafruit_BME280.h>
#include <U8x8lib.h>
#include "modbus.h"
#include "OneWireAccess.h"
#include "config.h"

// -------------------------------------------------------------------
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

class Display
{
  public:
    void setup();
    void tick();
  private:
    int line;
    Ticker ticker;
};

void Display::setup()
{
  u8x8.begin();
  u8x8.setPowerSave(0);

  u8x8.setCursor(0, 0);
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f); //u8x8_font_8x13B_1x2_f);

  u8x8.print("Unikat!");
}

static const char * AnimText[4] = { "-", "/", "|", "\\" };

void Display::tick()
{
  unsigned long now = millis();
  if (!ticker.NewTick(100, now)) return;
  
  u8x8.setCursor(0, 0);
  u8x8.print( AnimText[inputBuffer.displayTick & 0x3] );
  u8x8.setCursor(15, 0);
  u8x8.print( AnimText[inputBuffer.modbusTick & 0x3] );
  u8x8.setCursor(0, line);

  switch (line)
  {
    case 0:
      u8x8.setCursor(1, 0);
      u8x8.print("   ");
      u8x8.print(inputBuffer.pressure, 1); u8x8.print("hPa ");
      break;
    case 1:
      u8x8.print(inputBuffer.temp, 2); u8x8.print("\260C ");
      u8x8.print(inputBuffer.hum, 1); u8x8.println("%");
      break;

    case 2:
      break;

    case 3:
    case 4:
    case 5:
    case 6:
      for (int i = 0; i < 2; i++)
      {
        OneWireData & data = inputBuffer.ow[i + (line-3)*2];
        //u8x8.setCursor((i % 2) * 8, (i / 2) + 3);
        if (data.temp <= -1000) u8x8.print("--.--   ");
        else
        {
          //u8x8.print(data.addr[7],HEX);
          //u8x8.print(":");
          u8x8.print(data.temp, 2);
          u8x8.print("\260C ");
        }
      }
      break;
    case 7:
  for (int i = 0; i < 8; i++)
  {
    OneWireData & data = inputBuffer.ow[i];
    if (data.temp <= -1000) u8x8.print("--");
    else
    {
      if (data.addr[7] < 16) u8x8.print("0");
      u8x8.print(data.addr[7], HEX);
    }
  }

  default:
  case 8:
      line = 0;
      inputBuffer.displayTick++;
      return;
   }
  
  //  u8x8.println(inputBuffer.displayTick, HEX);
  //  u8x8.print(inputBuffer.temp, 2); u8x8.println("\260C");
  //  u8x8.print(inputBuffer.hum, 1); u8x8.println("%");
  //  u8x8.print(inputBuffer.pressure, 1); u8x8.println("hPa");

  line++;

}
// -------------------------------------------------------------------

class BME280
{
  public:
    void setup();
    void tick();
  private:
    Ticker ticker;
    Adafruit_BME280 bme;
};

void BME280::setup()
{
  if (bme.begin(0x76)) {
    bme.setSampling(Adafruit_BME280::MODE_FORCED);
  }
  inputBuffer.temp = -1000;
  inputBuffer.pressure = -1000;
  inputBuffer.hum = -1000;
}

void BME280::tick()
{
  if (!ticker.NewTick(5000, millis())) return;

  bme.takeForcedMeasurement();
  inputBuffer.temp = bme.readTemperature();
  inputBuffer.pressure = bme.readPressure() / 100.0;
  inputBuffer.hum = bme.readHumidity();
}

// -------------------------------------------------------------------

DataBuffer dataBuffer;
InputBuffer inputBuffer;

Modbus modbus { 0x01 };
OneWireAccess onewire;
Display disp;
BME280 bme;

void setup() {
  modbus.setup();
  onewire.setup();
  disp.setup();
  bme.setup();
}

void loop() {
  modbus.tick();
  onewire.tick();
  disp.tick();
  bme.tick();
}
// -------------------------------------------------------------------
