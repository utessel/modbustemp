Arduino-Nano based, Modbus-using One-Wire temperature sensor
---

- Arduino Nano based: just use the Arduino-IDE to compile
- remote access via Mod-Bus with simple RS485 adapter
- cheap local OLED display support (I2C, install U8g2)
- BME280 (I2C, install Adafruit BME280)
- DS1820 One Wire Sensors (install OneWire)

Just connect one or more "One Wire" Sensors (up to 8): 
The firmware will find them and will read the temperature 
values and shows them on the display.

It then allows to read the values via modbus.

More sensors could be added. Or less.

for Modbus, the device simply allows to read the memory:
Defined in "InputBuffer", see config.h

Hardware:
---
BME280 and the Display are connected via I2C.

The One-Wire Devices are connected to Pin 2.

For real RS485-Modbus (=with rs485 adapter) activate the 
RE/DE pins in config.h

