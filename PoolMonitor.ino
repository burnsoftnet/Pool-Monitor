/*
 * Arduino Project to monitor PH and Temperature as well as outside temperature. 
 * This project is put together from other Arduino Projects as well as some custom 
 * stuff that was lacking in the other projects, or at least didn't function the way 
 * i needed it to
 * 
 * Repo: https://github.com/burnsoftnet/Pool-Monitor
 * 
 * Developer: Joe M.
 * Version 1.0.0.1
 */

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
bool useWifi=true;                         //Enabled or disable the wifi functionality and the webpage
#define OutsideTemp A0                      // Analog Pin sensor is connected to
Gravity_pH pH = A1;                         //assign analog pin A1 of Arduino to class Gravity_pH. connect output of pH sensor to pin A0
const int PoolTemp = 2;                     //Assigned Pool monitor A2 to the Water proof temperature sensor
bool buggerme = true;                       //Toggle Mesages and Serial Prints
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
WiFiClient client;

int status = WL_IDLE_STATUS;  

WiFiServer server(80);                      //The port to open up the web server on the wifi

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  if (useWifi)
  {
    InitWifi();
    printWifiStatus();  
  }
  
  SetupPH();
  SetupDallasTempMonitor();
}

/*
 * Initialize the Wifi connection.
 */
void InitWifi()
{
  isConnected = false;

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    DebugMessage("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    DebugMessage("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    DebugMsg("Attempting to connect to SSID: ");
    DebugMessage(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();
  // you're connected now, so print out the status:
}

/*
 * pH MOnitor Setup Function
 */
void SetupPH()
{
  if (pH.begin()) { DebugMessage("Loaded EEPROM");} 
  DebugMessage("");
  DebugMessage("Press 0 to Display Command Menu");
}

/*
 * Dallas Temperature Setup Function
 */
void SetupDallasTempMonitor()
{
  poolSensor.begin();
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
    if (Serial.available())
    {
      Serial.println(msg); 
    }
  }
}

void DebugMsg(String msg)
{
  if (buggerme)
  {
    if (Serial.available())
    {
      Serial.print(msg);  
    }
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
  double temp = poolSensor.getTempFByIndex(0);
  return temp;
  //DHT.read11(PoolTemp);
  //return (DHT.temperature * 9/5) + 32;
}

/*
 * Print ouot the webpage, close connection after the 
 * completion of the response and refresh every 5 seconds
 */
void DoWebpage()
{
          String request = client.readStringUntil('\r');
          DebugMessage(request);

          
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
          client.println(" F </td>");
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
    DebugMessage("new client");
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
    DebugMessage("client disconnected");
  }
  if (!isConnected)
  {
    DebugMessage("Not Connected");
    delay(200);
  }
}

/*
 * Main Function
 */
void loop() {
  if (useWifi)
  {
      doWebPage(); 
  }
 
  if (Serial.available())
  {
    char value = Serial.read();           //read the string until we see a <CR>  
    menuExec(value);
  }
}

/*
 * Print the SSID of the network that you are connected to
 * and print out the boards IP Address in the serial output
 */
void printWifiStatus() {
  
  isConnected = true;
  DebugMsg("SSID: ");
  DebugMessage(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  DebugMsg("IP Address: ");
  DebugMessage((String)ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  DebugMsg("signal strength (RSSI):");
  DebugMsg((String)rssi);
  DebugMessage(" dBm");
}

/*
 * Serial Monitor
 */
void printMenu()
{
  DebugMessage("");
  DebugMessage("POOL MONITOR");
  DebugMessage("---------------------");
  DebugMessage("");
  DebugMessage("Send the number of the command you want to execute");
  DebugMessage("");
  DebugMessage("1.) Show Wifi");
  DebugMessage("2.) Show Ph");
  DebugMessage("3.) pH Cal 4");
  DebugMessage("4.) pH Cal 7");
  DebugMessage("5.) pH Cal 10");
  DebugMessage("6.) pH Cal Clear");
  DebugMessage("o.) Show Outside Temp");
  DebugMessage("p.) Show Pool Temp");
  DebugMessage("h.) Show Outside Humidity");
  DebugMessage("0.) Show Menu");
}

/*
 * Menu to help calibrate the ph monitor
 */
void menuExec(char value)
{ 
  switch(value)
  {
    case '1':
        DebugMessage("");
        printWifiStatus();
        break;
    case '2':
       DebugMessage("");
       DebugMessage("pH Reading is:");
       DebugMessage((String)pH.read_ph());
       break;
    case '3':
        pH.cal_low();                                   //call function for low point calibration
        DebugMessage("");
        DebugMessage("LOW CALIBRATED");
        break;
    case '4':
        pH.cal_mid();                                   //call function for midpoint calibration
        DebugMessage("");
        DebugMessage("MID CALIBRATED");
        break;
    case '5':
        pH.cal_high();                                  //call function for highpoint calibration
        DebugMessage("");
        DebugMessage("HIGH CALIBRATED");
        break;
    case '6':
        pH.cal_clear();                                 //call function for clearing calibration
        DebugMessage("");
        DebugMessage("CALIBRATION CLEARED");
        break;
    case 'o':
        DebugMessage("");
        DebugMsg("Outside Temp: ");
        DebugMessage((String)GetLocalTemp());
        break;
    case 'p':
        DebugMessage("");
        DebugMsg("Pool Temp: ");
        DebugMessage((String)GetPoolTemp());
        break;
    case 'h':
        DebugMessage("");
        DebugMsg("Outside humidity :");
        DebugMessage((String)DHT.humidity);
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
