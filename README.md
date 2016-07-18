SwitchDoc Labs SDL_Arduino_INA3221 Library

SwitchDoc Labs - www.switchdoc.com

Version 2.1 July 17, 2017

The INA3221 is Triple-Channel, High-SideMeasurement, Shunt and Bus Voltage Monitor with I2C Interface

It is used in the SwitchDoc Labs Solar Controller Product, SunAirPlus and in a standalone SwitchDoc Labs INA3221 Breakout Board

http://www.switchdoc.com/sunairplus-solar-power-controllerdata-collector/

This fork enhances V1.1 with improved arithmetic precision and easier configuration, including a pretty print of the current config.

V2.1 has one incompatibility with V1 -- the #define SHUNT_RESISTOR_VALUE units are changed to from ohms to milli-ohms.  The Arduino IDE with default compiler warnings will flag an existing #define in your sketch so you can adjust it (or remove it).


