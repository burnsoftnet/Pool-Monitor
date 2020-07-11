
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "do_grav.h"
#include "EEPROM.h"


Gravity_DO::Gravity_DO(uint8_t pin){
	this->pin = pin;
}

bool Gravity_DO::begin(){
	if(EEPROM.read(0) == magic_char){
		EEPROM.get(0,Do);
		return true;
    }
	return false;
}

float Gravity_DO::read_voltage() {
  float voltage_mV = 0;
    for (int i = 0; i < volt_avg_len; ++i) {
      voltage_mV += analogRead(this->pin) / 1024.0 * 5000.0;
    }
    voltage_mV /= volt_avg_len;
  return voltage_mV;
}

float Gravity_DO::read_do_percentage(float voltage_mV) {
    return voltage_mV * 100.0 / this->Do.full_sat_voltage;
}

float Gravity_DO::cal() {
    this->Do.full_sat_voltage = read_voltage();
    EEPROM.put(0,Do);
}

float Gravity_DO::cal_clear() {
    this->Do.full_sat_voltage = DEFAULT_SAT_VOLTAGE;
    EEPROM.put(0,Do);
}

float Gravity_DO::read_do_percentage() {
  return(read_do_percentage(read_voltage()));
}
