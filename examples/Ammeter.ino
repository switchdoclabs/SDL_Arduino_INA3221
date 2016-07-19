#include <Arduino.h>
//#include <pins_jared.h>
//#include <I2Cscan.h>
#include <NewLiquidCrystal.h>  // for library resolution
#include <LiquidCrystal_I2C.h>  // for object defination in library
//#include <LiquidCrystal.h>  // for object defination in library
#include <Wire.h>
#include <SDL_Arduino_INA3221.h>

/*-----( Declare Constants )-----*/
static const uint8_t _LED = LED_BUILTIN;  // LED hooked to Pin 13
static const uint8_t _rsPin = 4;
static const uint8_t _rwPin = 5;
static const uint8_t _enPin = 6;
static const uint8_t _contrastPin = 9; // PWM pin controling contrast
static const uint8_t _backlightPin = 10; // PWM pin controling backlight

static const uint8_t _am_addr = 64;  //  hex 40 I2C address of sdl board
static const int32_t _am_shunt_value = 100;  // shunt resistor value on sdl board in milli-ohms

/*-----( Declare objects )-----*/
// initialize the library with the numbers of the interface pins
//     RS, RW, EN, D0-7, backlight, polarity
//LiquidCrystal lcd(_rsPin, _rwPin, _enPin, 
//                  A3, A2, A1, A0, 
//                  _backlightPin,  POSITIVE);

// initialize the library with the numbers of the interface pins
//LiquidCrystal_I2C lcd(0x23);  // address
//  PCF8574 port::LCD2004A pin  
//  P0=Rs/4, P1=RW/5, P2=En/6, P3=nc, P4=D4/11, P5=D5/12, P6=D6/13, P7=D7/14

//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x23, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

SDL_Arduino_INA3221 ampmeter(_am_addr, _am_shunt_value);

/*-----( Declare Variables )-----*/
INA3221_ConfigValues configValues;
int channels;
float time;

/*-----( Declare Helper functions )-----*/
void outLCDformatted(int32_t value, int width, int shift=6, bool trace=false);
void showChannelData(int channel, bool trace=false); 


void setup() 
{
  RXLED0;   TXLED0;  // set the LEDS off

// ------- Fire up peripherials  -------------
  Serial.begin(9600);  //This pipes to the serial monitor
  Wire.begin();        // I am the master
  lcd.begin(20, 4);    // cols, rows
  lcd.setContrast( 76, _contrastPin );  // lower is darker "off" dots
  lcd.clear();
  ampmeter.begin();

  RXLED1;   TXLED1;  
//  delay(5000);              // wait so user can open serial monitor

//  I2Cscan(true, false, 5);  // header, ALL, delay after active address found
  TWBR = (F_CPU/(100L*1000L) - 16)/2;   // I2C bus speed to 100 Mhz
  RXLED0;   TXLED1;  

  // Determine some config info needed for the LCD heading lines.
  configValues = ampmeter.getConfigSettings();
  channels = (configValues.configRegister && INA3221_CONFIG_ENABLE_CHAN1) +
                 (configValues.configRegister && INA3221_CONFIG_ENABLE_CHAN2) +
                 (configValues.configRegister && INA3221_CONFIG_ENABLE_CHAN3);
  time = ((configValues.busCT + configValues.shuntCT) * channels) * 1.10;  // plus 10% fudge
  time = time / 1000;  // micros to milli-seconds
/**/
  // Show hardware settings, sample rate, sample times
  ampmeter.PrintConfigValues(configValues);
  Serial.print(F("\nMinimum cycle time "));  Serial.print(time);  Serial.print(F(" ms total for "));
    Serial.print(channels);  Serial.println(F(" channel(s)"));
/**/    
}

void loop() 
{
// heading/static text
  lcd.setCursor(0, 0);    // column, line (zero origin)
  lcd.print(F("mA+V v1 sampleN="));  lcd.print(configValues.sampleSize);
  
  lcd.setCursor(0, 1);    // column, line (zero origin)
  lcd.print(F("CTa "));  lcd.print(configValues.shuntCT);
  lcd.print(F(" CTv "));  lcd.print(configValues.busCT);
  lcd.print(" ms");
/**/    
  showChannelData(1, true);
  showChannelData(2, true);
  showChannelData(3, true);
/**/  
  for (int i=0; i<49; i++) 
  {
    showChannelData(1);
    showChannelData(2);
    showChannelData(3);
    delay(500);              // wait for a half second
  }
}

void showChannelData(int channel, bool trace) 
{
  int32_t CuA = ampmeter.getCurrent_uA(channel);
  int32_t BmV = ampmeter.getBusVoltage_mV(channel);
  int32_t SuV = ampmeter.getShuntVoltage_uV(channel);
/*  
  if (trace) {
    float bV = ampmeter.getBusVoltage_V(channel);
    float cmA = ampmeter.getCurrent_mA(channel);
    float smV = ampmeter.getShuntVoltage_mV(channel);

    Serial.print(F("\nC"));
    Serial.print(channel); Serial.print(' ');
    Serial.print(cmA,9); Serial.print(F(" milli-amps ")); 
    Serial.print(bV,9); Serial.print(F(" Volts "));
    Serial.print(smV,9); Serial.print(F(" Shunt mV "));
//  The Arduino library can't recognize 32 bit values for proper printing.
//  A circumvention is sprintf adding a 'l' (ell) to the pattern.
    Serial.print(CuA,6); Serial.print(F(" \xb5\x41 ")); // micro symbol, cap A
    Serial.print(BmV,6); Serial.print(F(" bmV    "));
    Serial.print(SuV,6); Serial.print(F(" \xb5V    ")); 
    }
/**/    
  //  setCursor (column, line) // (zero origin)
  //  channels wired upside down inside box
  lcd.setCursor((channel-1)*7, 2);  outLCDformatted(CuA/10,6,2, trace);
  lcd.setCursor((channel-1)*7, 3);  outLCDformatted(BmV,6,3, trace);
}

void outLCDformatted(int32_t value, int width, int shift, bool trace) 
{
  char buffer[16] = "\0\0";
  int dp=12;  // location of decimal point in pattern buffer
  int first, last, ptr;
  
  int size = snprintf (buffer, sizeof(buffer), "%12li.", value);
  // buffer has 14 chars, buffer[dp=12] is a decimal point,
  // leading zeros are suppressed
  
  // shift the decimal point left requested amount
  for (int i = 1; i<=shift, dp>=1; i++) 
  { 
    // float any minus sign
    if (buffer[dp-1] == '-') {buffer[max(dp-2,0)] = '-';  buffer[max(dp-1,0)] = '0';}
    // undo suppression after (new) decimal point
    if (buffer[dp-1] == ' ') {buffer[dp-1] = '0';}
    buffer[dp] = buffer[dp-1];
    buffer[dp-1] = '.';
    dp = dp - 1;
    }
  // undo suppression before (new) decimal point
  if (buffer[dp-1] == ' ') {buffer[dp-1] = '0';}

  //  locate first significant digit
  for (ptr=0; (buffer[ptr] <= '0') && (ptr<(dp-1));  ptr++) {} 
  
  // locate (width) characters, avoiding a trailing decimal point
  first = ptr;  last = first + (width-1);

  // Back up 1 when last selection character is the decimal
  if (last == dp)  {  first = first - 1;  last = last - 1;  }

  // If selection runs off buffer (wide with, small shift), 
  //  move selection left back inside buffer
  while (last>= size) { first = first-1; last = last-1; }

  // Overflow when selection doesn't include the units digit 
  if (last < (dp-1)) {  buffer[last] = '*';  }

// Finally -- the goal of our quest!
  for (int i=first; i<=last; i++) {lcd.write(buffer[i]); }    
}

// #include <I2Cscan.c>
