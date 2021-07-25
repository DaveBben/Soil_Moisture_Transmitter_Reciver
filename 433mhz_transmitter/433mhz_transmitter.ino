
#include <RH_ASK.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <EEPROMex.h>

#define DEBUG 0 // Switch debug output on and off by 1 or 0


#if DEBUG
#define PRINTS(s)   { Serial.print(F(s)); }
#define PRINT(s,v)  { Serial.print(F(s)); Serial.print(v); }
#else
#define PRINTS(s)
#define PRINT(s,v)
#endif

//Data structure that we are sending
//We can only send 1 byte at a time, so 16 bit data needs to be split into high and low values
//That data will be joined back on the other side
///////////////////////////////////////////////////
 typedef struct
 {
     uint8_t   idLow;
     uint8_t   idHigh;
     uint8_t   mLow;
     uint8_t   mHigh;
     uint8_t   vLow;
     uint8_t   vHigh;
     uint8_t   firmware;
 } DataModel;
////////////////////////////
 
RH_ASK driver(2000, 11, 8, 10, false);  //(uint16_t speed=2000, uint8_t rxPin=11, uint8_t txPin=21, uint8_t pttPin=10, 

const byte LED_PIN = 2;
const byte FIRMWARE_VERSION = 2;
const byte VOLTAGE_DIVIDER_PIN = 7;
const byte SENSOR_ACTIVATE_PIN = 6; //Turns on NPN transistor and Capactive Soil Sensor
const byte SOIL_MOISTURE_PIN = 0;
const byte VCC_VOLTAGE_PIN = 2;

const uint16_t SLEEP_COUNTER_MAX = 1800; // 4 hours == 14,400 seconds / 8 seconds per sleep = 1,800
const float AREF_VOLTAGE = 3300.0; //milivtols - Regulator is connected to Aref


uint16_t ID;
uint16_t sleepCounts = SLEEP_COUNTER_MAX; //starting it at max so we send on startup

 
void setup()
{
    resetWatchdog ();

    #if DEBUG
      Serial.begin(9600);
    #endif
    
    pinMode(LED_PIN, OUTPUT);
    pinMode(SENSOR_ACTIVATE_PIN, OUTPUT);
    pinMode(VOLTAGE_DIVIDER_PIN, OUTPUT);
    analogReference(EXTERNAL); //Connecting a 3.3v regulator to Aref
    randomSeed(analogRead(SOIL_MOISTURE_PIN));
    driver.init();

    //Retrieve ID information
    ////////////////////////////
    ID = (uint16_t) EEPROM.readInt(0);
    PRINT("\nRetrieved Chip ID: ",ID); 
    if(ID == 65535){ //EEPROM address has not been written to most likely. Would use 255 if reading byte instead of int
      uint16_t newID = generateRandomID();
      EEPROM.writeInt(0, newID);
    }
 

   //Indicate that things are working
   ///////////////////////////////
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);


}


uint16_t generateRandomID(){
  uint16_t id = (uint16_t) random(1000, 20000);
  PRINT("\nNew Generated is: ",id); 
  return id;
}


//Samples ADC readings from a specific pin and returns the averag
//////////////////////////////////////////////
float sampleReadings(byte pin, byte samplesize){
   int totalReadings = 0;
   for(byte i =0; i< samplesize; i++){
    totalReadings = totalReadings + analogRead(pin);
    delay(10); //just pause a bit between readings;
  }
   float counts = totalReadings/(float)samplesize;
   return counts;
}


//Returns the  output Voltage of the capacitive soil sensor
/////////////////////////////////////
uint16_t getSoilReading(){
  float counts = sampleReadings(SOIL_MOISTURE_PIN, 25);
  PRINT("\nSoil Sensor Counts is: ",counts); 
  uint16_t voltage = ((counts/1023.0)* AREF_VOLTAGE);
  PRINT("\nSoil Sensor voltage is: ",voltage); 
  return voltage;
}


//Nick Gammon
//https://www.gammon.com.au/forum/?id=11497
///////////////////////////////////////////
void resetWatchdog (){
  // clear various "reset" flags
  MCUSR = 0;     
  WDTCSR = bit (WDCE) | bit (WDE);
  // set interrupt mode and an interval 
  WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
  wdt_reset();  // pat the dog
}  // end of resetWatchdog
  

//Nick Gammon
//https://www.gammon.com.au/forum/?id=11497
///////////////////////////////////////////
void enterSleep(){
  byte old_ADCSRA = ADCSRA;
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  ADCSRA = 0;            // turn off ADC
  power_all_disable ();  // power off ADC, Timer 0 and 1, serial interface
  noInterrupts ();       // timed sequence coming up
  resetWatchdog ();      // get watchdog ready
  sleep_enable ();       // ready to sleep
  interrupts ();         // interrupts are required now
  sleep_cpu ();          // sleep                
  sleep_disable ();      // precaution
  power_all_enable ();   // power everything back on
  ADCSRA = old_ADCSRA;   // re-enable ADC conversion
  
}

// watchdog interrupt
///////////////////////
ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
} 


//Returns the  output Voltage of the battery
/////////////////////////////////////
uint16_t getBatteryVoltage(){
    float counts = sampleReadings(VCC_VOLTAGE_PIN, 25);
    PRINT("\nBattery Counts is: ",counts); 
    uint16_t voltage = ((counts/1023.0)* AREF_VOLTAGE) * 2.0;
    PRINT("\nBattery Voltage is: ",voltage); 
    return voltage;
}

 

void transmit(){
    DataModel data;
    uint16_t moisture = getSoilReading();
    uint16_t voltage = getBatteryVoltage();
    uint8_t idLow = ID & 0xff;
    uint8_t idHigh = (ID >> 8);
    uint8_t xlow = moisture & 0xff;
    uint8_t xhigh = (moisture >> 8);
    uint8_t vlow = voltage & 0xff;
    uint8_t vhigh = (voltage >> 8);
    
    data.idLow = idLow;
    data.idHigh = idHigh;
    data.mLow = xlow;
    data.mHigh = xhigh;
    data.vLow = vlow;
    data.vHigh = vhigh;
    data.firmware = FIRMWARE_VERSION;

    driver.send((uint8_t *)&data, sizeof(data));
    driver.waitPacketSent();
   
}



 
void loop(){
    if(sleepCounts >= SLEEP_COUNTER_MAX ){
      digitalWrite(SENSOR_ACTIVATE_PIN, HIGH);
      digitalWrite(VOLTAGE_DIVIDER_PIN, HIGH);
      delay(500); //this delay is necesary or else adc will be off
      transmit();
      digitalWrite(VOLTAGE_DIVIDER_PIN, LOW);
      digitalWrite(SENSOR_ACTIVATE_PIN, LOW);
      sleepCounts = 0;
    }
    enterSleep();
    sleepCounts++;

}
