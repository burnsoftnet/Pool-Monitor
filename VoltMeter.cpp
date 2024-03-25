/*
 *  FILE:     VoltMeter.cpp
 *  VERSION:  0.1.01
 *  PURPOSE:  10:1 Voltmeter Reader for arduino
*/
#include "VoltMeter.h"

float R1 = 100000.00;  //Resistor 1 on the postive side.
float R2 = 10000.00;   //Resistor 2 on the negatvie side.

/*
 * Formula for calculating voltage out.
 * @param val - The reading value from the voltmeter circuit
 * @return - End Calculation from the meter reading for the voltage out
 */
 
float voltmeter::valueOut(int val)
{
  return (val * 5.00)/1024.00;
}

/*
 * Formula for calculating coltage in.
 * @param val - The reading value from the voltmeter circuit
 * @return - End Calculation from the meter reading for the voltage in
 */
float voltmeter::valueIn(float vout)
{
  return vout/(R2/(R1+R2));
}

/*
 * Read the information in the meter and return the 
 * value of the voltage returned
 * @param meter - The reading value from the voltmeter circuit
 * @param threshold - the threshold will handle any bleeding, so if your bleeding is 0.05 thne you can set a threshold of 1 for anything that is true voltage
 * @return - End Calculation from the meter reading for the voltage out
 */
float voltmeter::readVoltageIn(int meter,float threshold)
{
  int val = analogRead(meter);
  float Vout = valueOut(val);
  float Vin = valueIn(Vout);
  if (Vin<threshold)
  {
    Vin=0.00;
  }
  return Vin;
}
