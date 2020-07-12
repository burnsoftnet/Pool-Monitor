#include <SPI.h>
#include <WiFiNINA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "dht.h"
#include "ph_grav.h"  
#include "arduino_secrets.h" 
#define SLAVE_ADDRESS 0x40 

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;                  // your network SSID (name)
char pass[] = SECRET_PASS;                  // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                           // your network key Index number (needed only for WEP)
bool isConnected;                           //marker to toggle the connection for serial diagnostics
#define OutsideTemp A0                         // Analog Pin sensor is connected to
#define PoolTemp A2
Gravity_pH pH = A1;                         //assign analog pin A1 of Arduino to class Gravity_pH. connect output of pH sensor to pin A0
//const int PoolTemp = A2;                    //Assigned Pool monitor A2 to the Water proof temperature sensor

//Define the length of our buffer for the I2C interface
const int I2C_BUFFER_LEN = 24;  //IMPORTANT MAX is 32!!!

//Load OneWire - proprietary dallas semiconductor sensor protocol - no license required
OneWire poolTemp(PoolTemp);

//Load Dallas - proprietary dallas sensor protocol utilizing onewire - no license required
DallasTemperature poolSensor(&poolTemp);

//Define I2C buffer
char data[I2C_BUFFER_LEN];
String temperatureData;

dht DHT;
//String inputstring = "";                    //a string to hold incoming data from the PC
//boolean input_string_complete = false;      //a flag to indicate have we received all the data from the PC
//char inputstring_array[10];                 //a char array needed for string parsing
bool buggerme = true;
WiFiClient client;

int status = WL_IDLE_STATUS;  

WiFiServer server(80);                      //The port to open up the web server on the wifi

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  isConnected = false;

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();
  // you're connected now, so print out the status:
  
  printWifiStatus();

  SetupPH();
  SetupDallasTempMonitor();
}

/*
 * pH MOnitor Setup Function
 */
void SetupPH()
{
  if (pH.begin()) { Serial.println("Loaded EEPROM");} 
  Serial.println("");
  Serial.println("Press 0 to Display Command Menu");
  
  poolSensor.begin();
}

/*
 * Dallas Temperature Setup Function
 */
void SetupDallasTempMonitor()
{
  //Start the I2C interface
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(requestEvent);
}

/*
 * Print debug messages to the serial output
 */
void DebugMessage(String msg)
{
  if (buggerme)
  {
    Serial.println(msg);
  }
}

/*
 * Get the local/outside temperature in Farenheit
 * the DHT Module returns the temp in Celsius.
 */
double GetLocalTemp()
{
  DHT.read11(OutsideTemp);
  return (DHT.temperature * 9/5) + 32;
}

/*
 * Get the temperature in the pool with the water proof sensor
 */
double GetPoolTemp()
{
  poolSensor.requestTemperatures();
  return poolSensor.getTempFByIndex(0);
  //DHT.read11(PoolTemp);
  //return (DHT.temperature * 9/5) + 32;
}

/*
 * Print ouot the webpage, close connection after the 
 * completion of the response and refresh every 5 seconds
 */
void DoWebpage()
{
          
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  
          client.println("Refresh: 5"); 
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          //client.println("");
          client.println("<center>");
          client.println("<b><h1>Pool Temperature</h1></b>");
          client.println("</br>");
          client.println("<table border=1>");
          client.println("<tr>");
          client.print("<td>");
          client.print("Outside Temperature </td>");
          client.print("<td>");
          client.print(GetLocalTemp());
          client.println(" F </td>");
          client.println("</tr>");
          client.println("<tr>");
          client.print("<td>");
          client.print("Outside Humidity </td>");
          client.print("<td>");
          client.print(DHT.humidity);
          client.println(" % </td>");
          client.println("</tr>");
          client.println("<tr>");
          client.print("<td>");
          client.print("Pool Temperature:  </td>");
          client.print(" <td>");
          client.print(GetPoolTemp());
          client.println(" degrees </td>");
          client.println("</tr>");
          client.println("<tr>");
          client.println("<td>");
          client.print("Ph Level:  </td>");
          client.print("<td>");
          client.print(pH.read_ph());
          client.println("</td>");
          client.println("</tr>");
          client.println("</table>");
          client.println("<center>");
          client.println("</html>");
}

/*
 * Listen for incoming clients connecting to the webpage
 */
void doWebPage()
{
  client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          DoWebpage();
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data and the Temperature sensor time to refresh
    delay(5000);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
  if (!isConnected)
  {
    Serial.println("Not Connected");
    delay(200);
  }
}

/*
 * Main Function
 */
void loop() {
  doWebPage();
  if (Serial.available())
  {
    char value = Serial.read();           //read the string until we see a <CR>  
    menuExec(value);
  }
  //Serial.println(pH.read_ph());
  //delay(1000);
}

/*
 * Print the SSID of the network that you are connected to
 * and print out the boards IP Address in the serial output
 */
void printWifiStatus() {
  
  isConnected = true;
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

/*
 * Serial Monitor
 */
void printMenu()
{
  Serial.println("");
  Serial.println("POOL MONITOR");
  Serial.println("---------------------");
  Serial.println("");
  Serial.println("Send the number of the command you want to execute");
  Serial.println("");
  Serial.println("1.) Show Wifi");
  Serial.println("2.) Show Ph");
  Serial.println("3.) pH Cal 4");
  Serial.println("4.) pH Cal 7");
  Serial.println("5.) pH Cal 10");
  Serial.println("6.) pH Cal Clear");
  Serial.println("0.) Show Menu");
}

/*
 * Menu to help calibrate the ph monitor
 */
void menuExec(char value)
{ 
  switch(value)
  {
    case '1':
        Serial.println("");
        printWifiStatus();
        break;
    case '2':
       Serial.println("");
       Serial.println("pH Reading is:");
       Serial.println(pH.read_ph());
       break;
    case '3':
        pH.cal_low();                                   //call function for low point calibration
        Serial.println("");
        Serial.println("LOW CALIBRATED");
        break;
    case '4':
        pH.cal_mid();                                   //call function for midpoint calibration
        Serial.println("");
        Serial.println("MID CALIBRATED");
        break;
    case '5':
        pH.cal_high();                                  //call function for highpoint calibration
        Serial.println("");
        Serial.println("HIGH CALIBRATED");
        break;
    case '6':
        pH.cal_clear();                                 //call function for clearing calibration
        Serial.println("");
        Serial.println("CALIBRATION CLEARED");
        break;
    case '0':
        printMenu();
        break;
  }
}
/*
 * Dallas Temperatore Request Handler
 */
void requestEvent() {
  //sends data over I2C in the format "88.99|78.12|100.00" where "PoolTemp|SolarTemp|OutsideTemp"
  temperatureData.toCharArray(data,I2C_BUFFER_LEN);
  Wire.write(data);
}
