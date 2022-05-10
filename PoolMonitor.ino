/*
 * Arduino Project to monitor PH and Temperature as well as outside temperature. 
 * This project is put together from other Arduino Projects as well as some custom 
 * stuff that was lacking in the other projects, or at least didn't function the way 
 * i needed it to
 * 
 * Registers Emulator: ATMEGA328
 * Board: Arduino Uno Wifi Rev2
 * 
 * Repo: https://github.com/burnsoftnet/Pool-Monitor
 * 
 * Developer: Joe M.
 * Version 3.0.0.5
 * Last Rev. Date: May 9th 2022
 */

#include <SPI.h>
#include <WiFiNINA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "dht.h"
#include "ph_grav.h"  
#include "arduino_secrets.h" 
#include "VoltMeter.h"

#define SLAVE_ADDRESS 0x40 

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;                  // your network SSID (name)
char pass[] = SECRET_PASS;                  // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                           // your network key Index number (needed only for WEP)
int webRefresh = 60;                        // Web Refresh interval
bool isConnected;                           // marker to toggle the connection for serial diagnostics
bool useWifi=true;                          // Enabled or disable the wifi functionality and the webpage
bool _getOutsideTemp=true;                     // Report back the Outside Local Temperature
bool _getPoolTemp=true;                      // Report back on the Pool Temperature
bool GetpH=true;                            // Report back on the pH; 
bool GetVm=true;                            // Report back on the battery voltage
bool buggerme = false;                       // Enabled Debug Messages
bool DisplayConnected = true;  // Switch to toggle functions for the LCD Screen if attached.
#define OutsideTemp A0                      // Analog Pin sensor is connected to
Gravity_pH pH = A1;                         // assign analog pin A1 of Arduino to class Gravity_pH. connect output of pH sensor to pin A0
const int PoolTemp = 2;                     // Assigned Pool monitor A2 to the Water proof temperature sensor
const int VoltMeter = 2;                     // Volt meter input

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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
voltmeter voltmeter;

WiFiClient client;

int status = WL_IDLE_STATUS;  

WiFiServer server(80);                      //The port to open up the web server on the wifi

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  if (DisplayConnected)
  {
     if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  } 
  }
  
  if (DisplayConnected)
  {
    delay(2000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("Pool Monitor");
    display.display();   
  }
  
  if (useWifi)
  {
    InitWifi();
    printWifiStatus();  
  }
  
  SetupPH();
  SetupDallasTempMonitor();
  if (GetVm)
  {
    pinMode(VoltMeter, INPUT);
  }
 
  
  Serial.println("Press ? to Display Command Menu");
}

/*
 * Initialize the Wifi connection.
 */
void InitWifi()
{
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
}

/*
 * pH MOnitor Setup Function
 */
void SetupPH()
{
  if (pH.begin()) { Serial.println("Loaded EEPROM");} 
  Serial.println("");
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
  double temp = poolSensor.getTempFByIndex(0);
  return temp;
}
/*
 * Print out the local/outside temperature and humity in the webpage
 */
void WebPage_LocalTemp()
{
  if (_getOutsideTemp)
  {
    double oTemp = GetLocalTemp();
    double oHum = DHT.humidity;
  
    client.println("<tr>");
    client.print("<td>");
    client.print("Outside Temperature </td>");
    client.print("<td>");
    if (oTemp > 32)
    {
      client.print(oTemp);
      client.println(" F </td>"); 
    } else {
      client.print("<font color=red>OFFLINE</font>");
      client.println("</td>");
    }
    client.println("</tr>");
    client.println("<tr>");
    client.print("<td>");
    client.print("Outside Humidity </td>");
    client.print("<td>");
    if (oHum > 0) 
    {
      client.print(oHum);
      client.println(" % </td>");  
    } else {
      client.print("<font color=red>OFFLINE</font>");
      client.println("</td>");
    }
    client.println("</tr>");
  }
}
/*
 * Print out the Pool Temperature for the Web Page
 */
void WebPage_PoolTemp()
{
  if (_getPoolTemp)
  {
    double pTemp = GetPoolTemp();
    
    client.println("<tr>");
    client.print("<td>");
    client.print("Pool Temperature:  </td>");
    client.print(" <td>");
    if (pTemp > 0)
    {
      client.print(pTemp);
      client.println(" F </td>");  
    } else {
      client.print("<font color=red>OFFLINE</font>");
      client.println("</td>");
    }
    client.println("</tr>");
  }
}
/*
 * Print out the Ph Table Row
 */
void WebPage_pH()
{
  if (GetpH)
  {
    float myPh = pH.read_ph();
    
    client.println("<tr>");
    client.println("<td>");
    client.print("Ph Level:  </td>");
    client.print("<td>");
    if (myPh > 0 && myPh < 6)
    {
      client.print("<font color=darkyellow>");
      client.print(myPh);
      client.print("</font>");  
    } else if (myPh > 6 && myPh < 8)
    {
      client.print("<font color=green>");
      client.print(myPh);
      client.print("</font>");
    } else if (myPh > 8 && myPh < 10)
    {
      client.print("<font color=orange>");
      client.print(myPh);
      client.print("</font>");
    } else if (myPh > 12 && myPh < 14 )
    {
      client.print("<font color=red>");
      client.print(myPh);
      client.print("</font>");
    } else
    {
      client.print("<font color=red>");
      client.print("OFFLINE");
      client.print("</font>");
    }
    
    client.println("</td>");
    client.println("</tr>");
  }
}
/*
 * Display voltage of battery in webpage
 */
void WebPage_Voltage()
{
  if (GetVm)
  {
    double volt = voltmeter.readVoltageIn(VoltMeter,0.90) / 11;
    client.println("<tr>");
    client.println("<td>");
    client.print("Battery Voltage:  </td>");
    client.print("<td>");
    if (volt > 0)
    {
      client.print(volt);
      client.println(" vdc</td>"); 
    } else {
      client.print("<font color=red>OFFLINE</font>");
      client.println("</td>");
    }
    client.println("</tr>");
  }
}

/*
 * Print out the webpage, close connection after the 
 * completion of the response and refresh every 5 seconds
 */
void DoWebpageContent()
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");  
    client.println("Refresh: " + webRefresh); 
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<center>");
    client.println("<b><h1>Pool Monitor</h1></b>");
    client.println("</br>");
    client.println("<table border=1>");
    WebPage_LocalTemp();
    WebPage_PoolTemp();
    WebPage_pH();
    WebPage_Voltage();
    client.println("</table>");
    client.println("</br>");
    client.println("<a href=\"/cal\">Click here to calibrate pH</a>");
    client.println("<center>");
    client.println("</html>");
}

/*
 * Print out the Web Page menu for the pH Calibration Section
 */
void DoPhCalibrationWeb()
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");  
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<center>");
    client.println("<b><h1>Pool Monitor</h1></b>");
    client.println("<b><h1>ph Calibration</h1></b>");
    client.println("</br>");
    client.println("<table border=1>");
    client.println("<tr>");
    client.println("<td>");
    client.print("<a href=\"/phCal4\">ph Cal 4</a></td>");
    client.println("</tr>");
    client.println("<tr>");
    client.println("<td>");
    client.print("<a href=\"/phCal7\">ph Cal 7</a></td>");
    client.println("</tr>");
    client.println("<tr>");
    client.println("<td>");
    client.print("<a href=\"/phCal10\">ph Cal 10</a></td>");
    client.println("</tr>");
    client.println("<tr>");
    client.println("<td>");
    client.print("<a href=\"/phCalClear\">ph Cal Clear</a></td>");
    client.println("</tr>");
    client.println("</table>");
    client.println("</br>");
    client.println("</br>");
    client.print("<a href=\"/\">Back To Main Menu</a>");
    client.println("<center>");
    client.println("</html>");
}

/*
 * Print out the Webpage that will display that the low, mid or high clibration
 * was performed with the option to head back to the calibration menu.
 */
void DoPhCalibrationWeb(int level)
{
    String LevelMarker = "";    
    switch (level)
    {
      case 1:
        pH.cal_low();
        LevelMarker="ph Low Calibrated";
        break;
      case 2:
        pH.cal_mid(); 
        LevelMarker="ph Mid Calibrated";
        break;
      case 3:
        pH.cal_high(); 
        LevelMarker="ph High Calibrated";
        break;
      case 4:
        pH.cal_clear();
        LevelMarker="ph Calibration Cleared";
        break;
    }
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");  
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<center>");
    client.println("<b><h1>" + LevelMarker + "</h1></b>");
    client.println("<b>Completed!</b>");
    client.println("</br>");
    client.print("<a href=\"/cal\">Back</a>");
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
    boolean doCalibration = false;
    bool doCal4 = false;
    bool doCal7 = false;
    bool doCal10 = false;
    bool doCalClear = false;
    //Serial.println("new client");
    // an http request ends with a blank line
    String myRequest = "";
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        myRequest.concat(c);
        //char *name = strtok(NULL,"=");
        //Serial.write(name);

        if (myRequest == "GET /phCal4")
        {
          doCal4 = true;
        }

        if (myRequest == "GET /phCal7")
        {
          doCal7 = true;
        }

        if (myRequest == "GET /phCal10")
        {
          doCal10 = true;
        }

        if (myRequest == "GET /phCalClear")
        {
          doCalClear = true;
        }

        if (myRequest == "GET /cal")
        {
          doCalibration = true;
        }
        
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          //Serial.println(myRequest);
          if (doCalibration)
          {
            DoPhCalibrationWeb();
          } else if (doCal4)
          {
            DoPhCalibrationWeb(1);
          }else if (doCal7)
          {
            DoPhCalibrationWeb(2);
          }else if (doCal10)
          {
            DoPhCalibrationWeb(3);
          }else if (doCalClear)
          {
            DoPhCalibrationWeb(4);
          }
          else 
          {
            DoWebpageContent();  
          }
          
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
  if (useWifi)
  {
      doWebPage(); 
  }
 
  if (Serial.available())
  {
    char value = Serial.read();           //read the string until we see a <CR>  
    menuExec(value);
  }
  if (DisplayConnected)
  {
    PrintDisplay();
    delay(5000); 
  }
}
/*
 * Print information to the OLEd Screen
 */
void PrintDisplay()
{
    double oTemp = GetLocalTemp();
    double oHum = DHT.humidity;
    double pTemp = GetPoolTemp();
    float myPh = pH.read_ph();
    display.clearDisplay();
    
    display.setCursor(10, 0);
    display.print("Pool Monitor");
    
    display.setCursor(0, 20);
    display.print("Outside Temp:");
    if (oTemp > 32)
    {
      display.println(oTemp);  
    } else {
      //display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.println("OFFLINE");
      //display.setTextColor(SSD1306_WHITE);
    }
    
    //display.display(); 

    display.setCursor(0, 30);
    display.print("Outside hum.:");
    if (oHum > 0)
    {
      display.println(oHum);
    } else {
      //display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.println("OFFLINE");
      //display.setTextColor(SSD1306_WHITE); 
    }
     

    display.setCursor(0, 40);
    display.print("Pool Temp:");
    display.println(pTemp);
    
    if (pTemp > 0)
    {
      //display.setTextColor(SSD1306_BLACK);
      display.println(pTemp);  
     // display.setTextColor(SSD1306_WHITE);
    } else {
      //display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.println("OFFLINE");
      //display.setTextColor(SSD1306_WHITE);
    } 

    display.setCursor(0, 50);
    //display.setTextColor(BLUE);
    display.print("pH Level:");
    //display.setTextColor(YELLOW);
        if (myPh > 0 && myPh < 14)
    {
      //display.setTextColor(SSD1306_BLACK);
      display.println(myPh);  
    } else {
      //display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.println("OFFLINE");
      //display.setTextColor(SSD1306_WHITE);
    }
    
    display.display(); 
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
  Serial.println("o.) Show Outside Temp");
  Serial.println("p.) Show Pool Temp");
  Serial.println("h.) Show Outside Humidity");
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
    case 'o':
        Serial.println("");
        Serial.print("Outside Temp: ");
        Serial.println(GetLocalTemp());
        break;
    case 'p':
        Serial.println("");
        Serial.print("Pool Temp: ");
        Serial.println(GetPoolTemp());
        break;
    case 'h':
        Serial.println("");
        Serial.print("Outside humidity :");
        Serial.println(DHT.humidity);
        break;
    case '?':
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
