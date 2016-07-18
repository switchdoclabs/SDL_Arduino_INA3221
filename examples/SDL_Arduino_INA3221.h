//
//   SDL_Arduino_INA3221 Library
//   SDL_Arduino_INA3221.cpp Arduino code - runs in continuous mode
//   Version 1.1
//   SwitchDoc Labs   January 31, 2015
//
//

/**************************************************************************/
/*! 
    Initial code from INA219 code (Basically just a core structure left)
    @author   K. Townsend (Adafruit Industries)
    @author   J. Thomas added ConfigValues structure support.
    @license  BSD (see BSDlicense.txt)
    
*/
/**************************************************************************/

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>

/*=========================================================================
    Shunt Resistor value in milli-ohms
    -----------------------------------------------------------------------*/
#define SHUNT_RESISTOR_VALUE  100   // default shunt resistor value of 0.1 Ohm
/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
#define INA3221_ADDRESS                         (0x40)    // 1000000 (A0+A1=GND)

    #define INA3221_REG_CONFIG                        (0x00)
    #define INA3221_READ                              (0x01)
    #define INA3221_REG_SHUNTVOLTAGE_1                (0x01)
    #define INA3221_REG_BUSVOLTAGE_1                  (0x02)
/*=========================================================================
    CONFIG REGISTER (R/W)
    -----------------------------------------------------------------------*/
    #define INA3221_CONFIG_RESET                    (0x8000)  // Reset Bit

    #define INA3221_CONFIG_ENABLE_CHAN1             (0x4000)  // Enable Channel 1
    #define INA3221_CONFIG_ENABLE_CHAN2             (0x2000)  // Enable Channel 2
    #define INA3221_CONFIG_ENABLE_CHAN3             (0x1000)  // Enable Channel 3

        // {1, 4, 16, 64, 128, 256, 512, 1024}[three setting bits]
    #define INA3221_CONFIG_AVG2                     (0x0800)  // AVG Samples Bit 2 - See table 3 spec  128
    #define INA3221_CONFIG_AVG1                     (0x0400)  // AVG Samples Bit 1 - See table 3 spec   16
    #define INA3221_CONFIG_AVG0                     (0x0200)  // AVG Samples Bit 0 - See table 3 spec    4

        // msec {.140 .204 332 .588 1.1 2.116 4.156 8.244} [3 setting bits]
    #define INA3221_CONFIG_VBUS_CT2                 (0x0100)  // VBUS bit 2 Conversion time - See table 4 spec  1.1
    #define INA3221_CONFIG_VBUS_CT1                 (0x0080)  // VBUS bit 1 Conversion time - See table 4 spec   .332
    #define INA3221_CONFIG_VBUS_CT0                 (0x0040)  // VBUS bit 0 Conversion time - See table 4 spec   .204

        // msec {.140 .204 332 .588 1.1 2.116 4.156 8.244} [3 setting bits]
    #define INA3221_CONFIG_VSH_CT2                  (0x0020)  // Vshunt bit 2 Conversion time - See table 5 spec  1.1
    #define INA3221_CONFIG_VSH_CT1                  (0x0010)  // Vshunt bit 1 Conversion time - See table 5 spec   .332
    #define INA3221_CONFIG_VSH_CT0                  (0x0008)  // Vshunt bit 0 Conversion time - See table 5 spec   .204
    
    #define INA3221_CONFIG_MODE_2                   (0x0004)  // Operating Mode bit 2 - See table 6 spec   continuous
    #define INA3221_CONFIG_MODE_1                   (0x0002)  // Operating Mode bit 1 - See table 6 spec   bus
    #define INA3221_CONFIG_MODE_0                   (0x0001)  // Operating Mode bit 0 - See table 6 spec   shunt
    
    #define INA3221_CONFIG_SETCONFIG    INA3221_CONFIG_ENABLE_CHAN1 | \
                                        INA3221_CONFIG_ENABLE_CHAN2 | \
                                        INA3221_CONFIG_ENABLE_CHAN3 | \
                                        INA3221_CONFIG_AVG1       | \
                                        INA3221_CONFIG_VBUS_CT2 | \
                                        INA3221_CONFIG_VSH_CT2 | \
                                        INA3221_CONFIG_MODE_2 | \
                                        INA3221_CONFIG_MODE_1 | \
                                        INA3221_CONFIG_MODE_0

        // available number of sampleSize  collected and averaged together for measurement 
    #define INA3221_SAMPLE_NUMBERS                   1, 4, 16, 64, 128, 256, 512, 1024
        // available conversion times for shunt and bus voltage measurement
    #define INA3221_CONVERSION_TIMES                 140, 204, 332, 588, 1100, 2116, 4156, 8244
/*=========================================================================*/
typedef struct INA3221_ConfigValues 
{
  uint16_t configRegister;        // hardware bit inside INA3221
  uint16_t sampleSize;            // number of points averaged in a sample
  uint16_t busCT;                 // Conversion Time microseconds
  uint16_t shuntCT;               // Conversion Time microseconds
  uint16_t opMode;                // 0-7
  uint8_t  avail_1;               // padding for boundary alignment
  uint8_t  i2cAddr;               // IIC hardware address byte in use 
  int32_t  shuntResistor;         // milli-ohms
  int32_t  channelShuntResistors[3];  // milli-ohms
} 
INA3221_ConfigValues;

class SDL_Arduino_INA3221{
 public:
  SDL_Arduino_INA3221(uint8_t addr = INA3221_ADDRESS,
                      int32_t shuntResistor = SHUNT_RESISTOR_VALUE);
  void  begin(uint16_t config = INA3221_CONFIG_SETCONFIG,
              int32_t shunt1 = 0,
              int32_t shunt2 = 0,
              int32_t shunt3 = 0);
  float   getBusVoltage_V(int channel);
  float   getShuntVoltage_mV(int channel);  // who cares?
  float   getCurrent_mA(int channel);

  // These functions because integer arithmetic can be faster and more precise
  int32_t getBusVoltage_mV(int channel);
  int32_t getCurrent_uA(int channel);
  int32_t getShuntVoltage_uV(int channel);  // who cares?

  // These functions provided for setting/obtaining/documenting operating parameters 
  void setConfigSettings(uint16_t config);
  INA3221_ConfigValues getConfigSettings();
  INA3221_ConfigValues getConfigSettings(uint16_t config);
  void  PrintConfigValues(INA3221_ConfigValues values);
   
 private:
  INA3221_ConfigValues global;
  
  void wireWriteRegister(uint8_t reg, uint16_t value);
  void wireReadRegister(uint8_t reg, uint16_t *value);
  int16_t getBusVoltage_raw(int channel);
  int16_t getShuntVoltage_raw(int channel);

};
