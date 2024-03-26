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
 */
#include "Settings.h"
#include "WebServer.h"
#include <SPI.h>
#include <WiFiNINA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
/*
 * moved to oleddisplay
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>      
*/
#include "OledDisplay.h"
#include "dht.h"
#include "ph_grav.h"  
#include "arduino_secrets.h" 
#include "VoltMeter.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;                  // your network SSID (name)
char pass[] = SECRET_PASS;                  // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                           // your network key Index number (needed only for WEP)
bool isConnected;                           // marker to toggle the connection for serial diagnostics
Gravity_pH pH = A1;                         // assign analog pin A1 of Arduino to class Gravity_pH. connect output of pH sensor to pin A0
const int PoolTemp = 2;                     // Assigned Pool monitor A2 to the Water proof temperature sensor
const int VoltMeter = 2;                     // Volt meter input

/*
 * moved to oleddisplay
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
*/
//Define the length of our buffer for the I2C interface Fallas Temperature
const int I2C_BUFFER_LEN = 24;  //IMPORTANT MAX is 32!!!

//Load OneWire - proprietary dallas semiconductor sensor protocol - no license required
OneWire poolTemp(PoolTemp);

//Load Dallas - proprietary dallas sensor protocol utilizing onewire - no license required
DallasTemperature poolSensor(&poolTemp);

void(* resetFunc) (void) = 0; //declare reset function @ address 0

//Define I2C buffer
char data[I2C_BUFFER_LEN];
String temperatureData;
dht DHT;
voltmeter voltmeter;
webserver webserver;
oleddisplay oleddisplay;

WiFiClient client;

int status = WL_IDLE_STATUS;  

WiFiServer server(80);                      //The port to open up the web server on the wifi

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  Serial.print("Starting Pool Monitor v");
  Serial.println(VERSION);
  oleddisplay.initDisplay(DISPLAYCONNECTED, "Pool", "Monitor", VERSION, USE_BIG_TEXT);
  /*
 * moved to oleddisplay
  if (DISPLAYCONNECTED)
  {
     if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  } 
  }
  
  if (DISPLAYCONNECTED)
  {
    delay(2000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    if (USE_BIG_TEXT)
    {
        display.setFont(&FreeMono9pt7b);
    }
    display.println("Pool");
    display.println("Monitor");
    display.print("v");
    display.println(VERSION);
    display.display();   
  }
  */
  if (USE_WIFI)
  {
    InitWifi();
    printWifiStatus();  
  }
  
  SetupPH();
  SetupDallasTempMonitor();
  if (GET_BATTERY_VOLTAGE)
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
  if (pH.begin()) { 
    Serial.println("Loaded Atlas pH EEPROM");
    Serial.println("");
  } 
}

/*
 * Dallas Temperature Setup Function
 */
void SetupDallasTempMonitor()
{
  Serial.print("Initializing Temperature Monitor I2C");
  poolSensor.begin();
  Serial.print(".");
  //Start the I2C interface
  Wire.begin(SLAVE_ADDRESS);
  Serial.print(".");
  Wire.onRequest(requestEvent);
  Serial.print(".");
  Serial.println("DONE!");
}

/*
 * Print debug messages to the serial output
 * @param msg = The String message that you want printed in the console.
 */
void DebugMessage(String msg)
{
  if (BUGGERME)
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
 * Print out the webpage, close connection after the 
 * completion of the response and refresh every 5 seconds
 */
void DoWebpageContent()
{
    double oTemp = 0;
    double oHum = 0;
    double pTemp = 0;
    double volt = 0;
    float myPh = 0;
    if (GET_OUTSIDE_TEMP)
    {
      oTemp = GetLocalTemp();
      oHum = DHT.humidity;
    }
    if (GET_POOL_TEMP)
    {
      pTemp = GetPoolTemp();
    }
    if (GET_BATTERY_VOLTAGE)
    {
      volt = voltmeter.readVoltageIn(VoltMeter,0.90) / 11;
    }
    if (GET_PH)
    {
      myPh = pH.read_ph();
    }
    String value = webserver.doWebpageContent(webRefresh, GET_OUTSIDE_TEMP, oTemp, oHum, GET_POOL_TEMP, pTemp, GET_BATTERY_VOLTAGE, volt, GET_PH, myPh);
    DebugMessage(value);
    client.println(value);

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
    // an http request ends with a blank line
    String myRequest = "";
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        myRequest.concat(c);

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
          if (doCalibration)
          {
            client.println(webserver.doPhCalibrationWeb());
          } else if (doCal4)
          {
            pH.cal_low();
            client.println(webserver.doPhCalibrationWeb(1));
          }else if (doCal7)
          {
            pH.cal_mid(); 
            client.println(webserver.doPhCalibrationWeb(2));
          }else if (doCal10)
          {
            pH.cal_high(); 
            client.println(webserver.doPhCalibrationWeb(3));
          }else if (doCalClear)
          {
            pH.cal_clear();
            client.println(webserver.doPhCalibrationWeb(4));
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
  if (USE_WIFI)
  {
      doWebPage(); 
  }
 
  if (Serial.available())
  {
    char value = Serial.read();           //read the string until we see a <CR>  
    menuExec(value);
  }
  if (DISPLAYCONNECTED)
  {
      /*
 * moved to oleddisplay
    PrintDisplay();
    */
    double oTemp = GetLocalTemp();
    double oHum = DHT.humidity;
    double pTemp = GetPoolTemp();
    float myPh = pH.read_ph();
    oleddisplay.PrintDisplay(DISPLAYCONNECTED, "Pool Mon", USE_BIG_TEXT, pTemp, myPh, oTemp, oHum);
    delay(5000); 
  }
}
/*
 * Print information to the OLEd Screen
 */
   /*
 * moved to oleddisplay
void PrintDisplay()
{
    double oTemp = GetLocalTemp();
    double oHum = DHT.humidity;
    double pTemp = GetPoolTemp();
    float myPh = pH.read_ph();
    display.clearDisplay();
    display.setCursor(10, 10);
    display.println("Pool Mon");
    if (USE_BIG_TEXT)
    {
      display.setCursor(0, 40);
      display.print("PT:");
    
      if (pTemp > 0)
      {
        display.println(pTemp);  
      } else {
        display.println("OFFLINE");
      }

      display.print("pH:");
      if (myPh > 0 && myPh < 14)
      {
        display.println(myPh);  
      } else {
        display.println("OFFLINE");
      }
    } else {
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
    }
    
    display.display(); 
}
*/
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
  Serial.print("signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

/*
 * Serial Monitor
 */
void printMenu()
{
  Serial.println("");
  Serial.print("POOL MONITOR v");
  Serial.println(VERSION);
  Serial.println("--------------------------------------------------");
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
  Serial.println("r.) Reboot Arduino");
  Serial.println("");
  Serial.println("--------------------------------------------------");
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
    case 'r':
        Serial.println("Attempting Reset of the Arduino");
        resetFunc();  //call reset
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
