/*
 *  FILE:     VoltMeter.h
 *  VERSION:  0.1.00
 *  PURPOSE:  10:1 Voltmeter Reader for arduino
 */

 #ifndef VoltMeter_h
 #define VoltMeter_h
 
 #if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

 #define VOLTMETER_LIB_VERSION "0.1.00"

 class voltmeter
 {
  public:
    float readVoltageIn(int meter,float threshold);
  
  private:
    float valueOut(int val);
    float valueIn(float vout);

 };
#endif
