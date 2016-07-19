SwitchDoc Labs SDL_Arduino_INA3221 Library

SwitchDoc Labs - www.switchdoc.com

Version 2.1 July 17, 2017

The INA3221 is Triple-Channel, High-SideMeasurement, Shunt and Bus Voltage Monitor with I2C Interface

It is used in the SwitchDoc Labs Solar Controller Product, SunAirPlus and in a standalone SwitchDoc Labs INA3221 Breakout Board

http://www.switchdoc.com/sunairplus-solar-power-controllerdata-collector/

This fork enhances V1.1 with improved arithmetic precision and easier configuration, including a pretty print of the current config.

V2.1 has one incompatibility with V1 -- the #define SHUNT_RESISTOR_VALUE units are changed to from ohms to milli-ohms.  The Arduino IDE with default compiler warnings will flag an existing #define SHUNT_RESISTOR_VALUE in your sketch so you can adjust it (or remove it).

These methods are faster and more precise if you avoid floating point numbers:
  // These functions because integer arithmetic can be faster and more precise
  int32_t getBusVoltage_mV(int channel);
  int32_t getCurrent_uA(int channel);
  int32_t getShuntVoltage_uV(int channel);  // who cares? -- get the current directly

You may set some configuration options such as the number of samples in each measurement and the conversion time for each sample.

Increasing the number of samples averaged into a measurement can increase accuracy when values are fluctuating.  Using a longer conversion time increases the ADC accuracy of each sample.  Of course, more samples or longer times slows the rate at which measurements can be obtained.  

You can also speed things up by disabling unused channels, and turning off shunt or bus reporting if you don't need that information. 

The default settings will probably limit you to capture and return all three channels no faster than every 7ms.  Then add the time for your sketch to do its work...

The begin(config) operand is a 16 bit configuration register value.  It defaults to INA3221_CONFIG_SETCONFIG, which is "all channels", "16 samples" per measurement, "1.1 ms/sample" conversion time.  In the header file just before INA3221_CONFIG_SETCONFIG is defined, you will find the various values which can be or'ed together.  Mix and match to create your own.

Also in the header file are two arrays with the values available for sample size and conversion time:
        // available number of sampleSize  collected and averaged together for measurement 
    #define INA3221_SAMPLE_NUMBERS                   1, 4, 16, 64, 128, 256, 512, 1024
        // available conversion times for shunt and bus voltage measurement
    #define INA3221_CONVERSION_TIMES                 140, 204, 332, 588, 1100, 2116, 4156, 8244
Each 3 bit configuration register sub-field is functionally an index 0-7 into one of these arrays.

You can change the operating configuration on the fly using setConfigSettings(config).

If you have Serial port connected up, printConfigValues(config) will pretty print any settings passed to it.  To print the active settings,
call printConfigvalues(getConfigValues()) to read and pass them to the formatter.

Finally, if you get out the soldering iron and hack your board address, instantiating the INA3321 object with your new address is enough.  If you solder in new individual shunt resistors, pass their values (in milliohms) using the begin(config,r1,r2,r3) method operands.  Otherwise all three resistors will be assumed to (all) have the (same) value given the INA3321 instantiation.