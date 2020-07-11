/*
Once uploaded, open the serial monitor, set the baud rate to 9600 and append "Carriage return"
The code allows the user to observe real time pH readings as well as calibrate the sensor.
One, two or three-point calibration can be done.

Calibration commands:
 low-point: "cal,4"
 mid-point: "cal,7"
 high-point: "cal,10"
 clear calibration: "cal,clear"
*/

#include "ph_grav.h"                                  //header file for Atlas Scientific gravity pH sensor
#include "LiquidCrystal.h"                            //header file for liquid crystal display (lcd)

String inputstring = "";                              //a string to hold incoming data from the PC
boolean input_string_complete = false;                //a flag to indicate have we received all the data from the PC
char inputstring_array[10];                           //a char array needed for string parsing
Gravity_pH pH = A0;                                   //assign analog pin A0 of Arduino to class Gravity_pH. connect output of pH sensor to pin A0
LiquidCrystal pH_lcd(2, 3, 4, 5, 6, 7);               //make a variable pH_lcd and assign arduino digital pins to lcd pins (2 -> RS, 3 -> E, 4 to 7 -> D4 to D7)


void setup() {
  Serial.begin(9600);                                 //enable serial port
  pH_lcd.begin(20, 4);                                //start lcd interface and define lcd size (20 columns and 4 rows)
  pH_lcd.setCursor(0,0);                              //place cursor on screen at column 1, row 1
  pH_lcd.print("--------------------");               //display characters
  pH_lcd.setCursor(0,3);                              //place cursor on screen at column 1, row 4
  pH_lcd.print("--------------------");               //display characters
  pH_lcd.setCursor(5, 1);                             //place cursor on screen at column 6, row 2
  pH_lcd.print("pH Reading");                         //display "pH Reading" 
  if (pH.begin()) { Serial.println("Loaded EEPROM");} 
  Serial.println(F("Use commands \"CAL,4\", \"CAL,7\", and \"CAL,10\" to calibrate the circuit to those respective values"));
  Serial.println(F("Use command \"CAL,CLEAR\" to clear the calibration"));
 }


void serialEvent() {                                  //if the hardware serial port_0 receives a char
  inputstring = Serial.readStringUntil(13);           //read the string until we see a <CR>
  input_string_complete = true;                       //set the flag used to tell if we have received a completed string from the PC
}


void loop() {

  if (input_string_complete == true) {                //check if data received
    inputstring.toCharArray(inputstring_array, 30);   //convert the string to a char array
    parse_cmd(inputstring_array);                     //send data to pars_cmd function
    input_string_complete = false;                    //reset the flag used to tell if we have received a completed string from the PC
    inputstring = "";                                 //clear the string
  }
  Serial.println(pH.read_ph());                       //output pH reading to serial monitor
  pH_lcd.setCursor(8, 2);                             //place cursor on screen at column 9, row 3
  pH_lcd.print(pH.read_ph());                         //output pH to lcd
  delay(1000);
}


void parse_cmd(char* string) {                      //For calling calibration functions
  strupr(string);                                   //convert input string to uppercase

  if (strcmp(string, "CAL,4") == 0) {               //compare user input string with CAL,4 and if they match, proceed
    pH.cal_low();                                   //call function for low point calibration
    Serial.println("LOW CALIBRATED");
  }
  else if (strcmp(string, "CAL,7") == 0) {          //compare user input string with CAL,7 and if they match, proceed
    pH.cal_mid();                                   //call function for midpoint calibration
    Serial.println("MID CALIBRATED");
  }
  else if (strcmp(string, "CAL,10") == 0) {         //compare user input string with CAL,10 and if they match, proceed
    pH.cal_high();                                  //call function for highpoint calibration
    Serial.println("HIGH CALIBRATED");
  }
  else if (strcmp(string, "CAL,CLEAR") == 0) {      //compare user input string with CAL,CLEAR and if they match, proceed
    pH.cal_clear();                                 //call function for clearing calibration
    Serial.println("CALIBRATION CLEARED");
  }
}