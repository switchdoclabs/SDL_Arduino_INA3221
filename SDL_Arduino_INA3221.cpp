//
//   SDL_Arduino_INA3221 Library
//   SDL_Arduino_INA3221.cpp Arduino code - runs in continuous mode
//   Version 1.1
//   SwitchDoc Labs   January 31, 2015
//
/*
    Initial code from INA219 code (Basically just a core structure left)
    @author   K.Townsend (Adafruit Industries)
    @author  Jared Thomas 
	@license  BSD (see BSDlicense.txt)
*/

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>

#include "SDL_Arduino_INA3221.h"

/**************************************************************************/
/*! 
    @brief  Instantiates a new SDL_Arduino_INA3221 class
*/
/**************************************************************************/
SDL_Arduino_INA3221::SDL_Arduino_INA3221(uint8_t addr, int32_t shuntResistor) {
    global.i2cAddr = addr;
    global.shuntResistor = shuntResistor;
    //  all channels start configured with the same shunt resistor value.
    //  Use begin(...) to change values if board has been custom altered
    global.channelShuntResistors[0] = shuntResistor;
    global.channelShuntResistors[1] = shuntResistor;
    global.channelShuntResistors[2] = shuntResistor;
 }

/**************************************************************************/
/*! 
    @brief  Sets up the HW configuration
*/
/**************************************************************************/
void SDL_Arduino_INA3221::begin(uint16_t config, 
                                int32_t sr0, int32_t sr1, int32_t sr2 )
{
  // save configuration
  global.configRegister = config;
  //  If board shunt resistors  have been altered then here's the customized values
  if (sr0 != 0) { global.channelShuntResistors[0] = sr0; }
  if (sr1 != 0) { global.channelShuntResistors[1] = sr1; }
  if (sr2 != 0) { global.channelShuntResistors[2] = sr2; }

  Wire.begin();
  // Set chip to known config values to start
  setConfigSettings(config);  
}

/**************************************************************************/
/*! 
    @brief  Sends a single command byte over I2C
*/
/**************************************************************************/
void SDL_Arduino_INA3221::wireWriteRegister (uint8_t reg, uint16_t value)
{
  Wire.beginTransmission(global.i2cAddr);
  #if ARDUINO >= 100
    Wire.write(reg);                       // Register
    Wire.write((value >> 8) & 0xFF);       // Upper 8-bits
    Wire.write(value & 0xFF);              // Lower 8-bits
  #else
    Wire.send(reg);                        // Register
    Wire.send(value >> 8);                 // Upper 8-bits
    Wire.send(value & 0xFF);               // Lower 8-bits
  #endif
  Wire.endTransmission();
}

/**************************************************************************/
/*! 
    @brief  Reads a 16 bit values over I2C
*/
/**************************************************************************/
void SDL_Arduino_INA3221::wireReadRegister(uint8_t reg, uint16_t *value)
{

  Wire.beginTransmission(global.i2cAddr);
  #if ARDUINO >= 100
    Wire.write(reg);                       // Register
  #else
    Wire.send(reg);                        // Register
  #endif
  Wire.endTransmission();
  
  delay(1); // Max 12-bit conversion time is 586us per sample

  Wire.requestFrom(global.i2cAddr, (uint8_t)2);  
  #if ARDUINO >= 100
    // Shift values to create properly formed integer
    *value = ((Wire.read() << 8) | Wire.read());
  #else
    // Shift values to create properly formed integer
    *value = ((Wire.receive() << 8) | Wire.receive());
  #endif
}

/**************************************************************************/
/*! 
    @brief  Gets the raw bus voltage (16-bit signed integer,  milli-volts, +/- 4 milli-volts)
*/
/**************************************************************************/
int16_t SDL_Arduino_INA3221::getBusVoltage_raw(int channel) {
  union bit16 { int16_t i16; uint16_t u16; } value;
  wireReadRegister(INA3221_REG_BUSVOLTAGE_1+(channel -1) *2, &value.u16);
/*    
  Serial.print("\nBusVoltage_raw="); Serial.print(value);
*/
  return (value.i16);
}

/**************************************************************************/
/*! 
    @brief  Gets the raw shunt voltage (16-bit signed integer,  units of 5 micro-volts)
*/
/**************************************************************************/
int16_t SDL_Arduino_INA3221::getShuntVoltage_raw(int channel) {
  union bit16 { int16_t i16; uint16_t u16; } value;
  wireReadRegister(INA3221_REG_SHUNTVOLTAGE_1+(channel -1) *2, &value.u16);
/*
    Serial.print("\nShuntVoltage_raw=");
    Serial.print(value); Serial.print(" x");  Serial.print(value,HEX);
*/
  return (value.i16);
}

 
/**************************************************************************/
/*! 
    @brief  Gets the Bus voltage in milli-volts
*/
/**************************************************************************/
int32_t SDL_Arduino_INA3221::getBusVoltage_mV(int channel) {
  return getBusVoltage_raw(channel);
}

/**************************************************************************/
/*! 
    @brief  Gets the current value in uA, taking into account the
            config settings and current LSB
*/
/**************************************************************************/
int32_t SDL_Arduino_INA3221::getCurrent_uA(int channel) {
  int32_t value = getShuntVoltage_raw(channel);  // units of 5 uV
/*
  Serial.print("\ngetCurrent_uA value=");
      Serial.print(value); Serial.print(" x");  Serial.print(value,HEX);
  float units = value; 
    Serial.print(" units="); Serial.print(units,12);
  float uV = value*5;
    Serial.print(" micro-volts="); Serial.print(uV,12);
  float mA = (uV / global.channelShuntResistors[channel]);  // micro over milli begets milli
    Serial.print("  milli-amps="); Serial.print(mA,12);
  units = (value*5*1000) / global.channelShuntResistors[channel]; 
    Serial.print(" sr=x"); Serial.print(global.channelShuntResistors[channel],HEX);
    Serial.print(" return="); Serial.print(units,12);
*/ 
  // *5 gives uV, *1000 makes mV
  return ((value*5*1000) / global.channelShuntResistors[channel]);
}

/**************************************************************************/
/*! 
    @brief  Gets the shunt voltage in uV ( +/- 168.3mV)
*/
/**************************************************************************/
int32_t SDL_Arduino_INA3221::getShuntVoltage_uV(int channel) {
  int32_t value = getShuntVoltage_raw(channel);  // units of 5 uV
  return (value * 5);  // 5 uV units to uV
}

/**************************************************************************/
/*! 
    @brief  Gets the Bus voltage in volts
*/
/**************************************************************************/
float SDL_Arduino_INA3221::getBusVoltage_V(int channel) {
  int32_t value = getBusVoltage_raw(channel);   // units of 1mv
  return (float(value)/1000.0);
}
/**************************************************************************/
/*! 
    @brief  Gets the shunt current in milli-Amps
*/
/**************************************************************************/
float SDL_Arduino_INA3221::getCurrent_mA(int channel) {
  int32_t value = getShuntVoltage_raw(channel);  // units of 5 uV
  // stay in int mode & keep precision   //micro over milli begets milli
  value = (value*5)/global.channelShuntResistors[channel]; 
  return ( float(value)); 
}
/**************************************************************************/
/*! 
    @brief  Gets the shunt voltage drop in milli volts
*/
//**************************************************************************/
float SDL_Arduino_INA3221::getShuntVoltage_mV(int channel) {
  int32_t value = getShuntVoltage_raw(channel);  // units of 5 uV
  value = value * 5;  // to uV
  return ( float(value) / 1000.0);  // uV units to mV
}

/**************************************************************************/
/*! 
    @brief  Send the configuration bytes
*/
/**************************************************************************/
void SDL_Arduino_INA3221::setConfigSettings(uint16_t config)
{
  wireWriteRegister(INA3221_REG_CONFIG, INA3221_CONFIG_RESET);  // POR
  wireWriteRegister(INA3221_REG_CONFIG, config);
}

/**************************************************************************/
/*! 
    @brief  Returns hardware configurations values in INA3221_ConfigValues struct
*/
/**************************************************************************/
INA3221_ConfigValues SDL_Arduino_INA3221::getConfigSettings()
{
  uint16_t config;
  wireReadRegister(INA3221_REG_CONFIG, &config);
  return (getConfigSettings(config));
}

/**************************************************************************/
/*! 
    @brief  Returns hardware configurations values in INA3221_ConfigValues struct
*/
/**************************************************************************/
INA3221_ConfigValues SDL_Arduino_INA3221::getConfigSettings(uint16_t config)
{
  INA3221_ConfigValues values;
  const uint16_t numAvgsBits = INA3221_CONFIG_AVG2 + INA3221_CONFIG_AVG1 + INA3221_CONFIG_AVG0;
  const uint16_t sampleSize[8] = {INA3221_SAMPLE_NUMBERS};
  const uint16_t busCTbits = INA3221_CONFIG_VBUS_CT2 + INA3221_CONFIG_VBUS_CT1 + INA3221_CONFIG_VBUS_CT0;
  const uint16_t busCT[8] = {INA3221_CONVERSION_TIMES};
  const uint16_t shuntCTbits = INA3221_CONFIG_VSH_CT2 + INA3221_CONFIG_VSH_CT1 + INA3221_CONFIG_VSH_CT0;
  const uint16_t shuntCT[8] = {INA3221_CONVERSION_TIMES};
  const uint16_t modeBits = INA3221_CONFIG_MODE_2 + INA3221_CONFIG_MODE_1 + INA3221_CONFIG_MODE_0;

  values.configRegister = config;
  values.sampleSize = sampleSize[(config & numAvgsBits) >> 9];
  values.busCT = busCT[(config & busCTbits) >> 6];
  values.shuntCT = shuntCT[(config & shuntCTbits) >> 3];
  values.opMode = (config & modeBits);

  // and while we're here, copy the saved global configuration
  values.i2cAddr =                  global.i2cAddr; 
  values.shuntResistor =            global.shuntResistor;
  values.channelShuntResistors[0] = global.channelShuntResistors[0];
  values.channelShuntResistors[1] = global.channelShuntResistors[1];
  values.channelShuntResistors[2] = global.channelShuntResistors[2];

  return (values);
  }

/**************************************************************************/
/*! 
    @brief  Prints hardware configurations values in INA3221_ConfigValues struct
*/
/**************************************************************************/
void SDL_Arduino_INA3221::printConfigValues(INA3221_ConfigValues values)
{
  float ohms;
  Serial.print(F("\nINA3221 at I2C address x"));  Serial.print(values.i2cAddr,HEX); 
  Serial.print(F("  default shunt value ")); Serial.print(((float)values.shuntResistor) / 1000.0);
      Serial.print(F(" ohm"));
       
  Serial.print(F("\nConfigRegister=")); Serial.print(values.configRegister,HEX);
       
  if (values.configRegister & INA3221_CONFIG_RESET)
    { Serial.print(F("\n Power-On Reset Request")); }
  if (values.configRegister & INA3221_CONFIG_ENABLE_CHAN1)
    { Serial.print(F("\n Channel 1 enabled ")); 
      ohms = ((float)values.channelShuntResistors[0]) / 1000.0; 
      Serial.print(ohms); Serial.print(F(" ohm shunt"));}
  if (values.configRegister & INA3221_CONFIG_ENABLE_CHAN2)
    { Serial.print(F("\n Channel 2 enabled "));       
      ohms = ((float)values.channelShuntResistors[1]) / 1000.0; 
      Serial.print(ohms); Serial.print(F(" ohm shunt"));}
  if (values.configRegister & INA3221_CONFIG_ENABLE_CHAN3)
    { Serial.print(F("\n Channel 3 enabled ")); 
      ohms = ((float)values.channelShuntResistors[2]) / 1000.0; 
      Serial.print(ohms); Serial.print(F(" ohm shunt"));}
       
   Serial.print(F("\n ")); Serial.print(values.sampleSize); Serial.print(F(" points per sample "));
   Serial.print(F("\n Amperage sample time ")); Serial.print(values.shuntCT);  
       Serial.print(F(" micro-seconds"));
   Serial.print(F("\n Voltage sample time ")); Serial.print(values.busCT);  
       Serial.print(F(" micro-seconds"));
   Serial.print(F("\n Operating Mode ")); Serial.print(values.opMode);
       
   if (values.configRegister & INA3221_CONFIG_MODE_2) 
     {Serial.print(F("  Continuous")); }
   else Serial.print(F("  Triggered"));
   if (!(values.configRegister & (INA3221_CONFIG_MODE_1+INA3221_CONFIG_MODE_0)))
     { Serial.print(F(" -- Power Down mode")); }
   else 
   {
     if (values.configRegister & INA3221_CONFIG_MODE_1) 
       { Serial.print(F("  Voltage")); }
     if (values.configRegister & INA3221_CONFIG_MODE_0)
       { Serial.print(F("  Amperage")); }
   } 
} 
