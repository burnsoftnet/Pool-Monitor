
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "orp_grav.h"
#include "EEPROM.h"


Gravity_ORP::Gravity_ORP(uint8_t pin){
	this->pin = pin;
}

bool Gravity_ORP::begin(){
	if(EEPROM.read(0) == magic_char){
		EEPROM.get(0,Orp);
		return true;
    }
	return false;
}

float Gravity_ORP::read_voltage() {
  float voltage_mV = 0;
    for (int i = 0; i < volt_avg_len; ++i) {
      voltage_mV += analogRead(this->pin) / 1024.0 * 5000.0;
    }
    voltage_mV /= volt_avg_len;
  return voltage_mV;
}

float Gravity_ORP::read_orp(float voltage_mV) {
    return voltage_mV - 1500.0 - this->Orp.cal_offset; //midpoint
}

float Gravity_ORP::cal(float value) {
    this->Orp.cal_offset = read_voltage() - (value + 1500);
    EEPROM.put(0,Orp);
}

float Gravity_ORP::cal_clear() {
    this->Orp.cal_offset = 0;
    EEPROM.put(0,Orp);
}

float Gravity_ORP::read_orp() {
  return(read_orp(read_voltage()));
}
