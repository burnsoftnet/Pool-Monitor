#include "OledDisplay.h"

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/*
 *  Initialize the the OLED Display for the startup function, which when the arduino is first starting up it
 *  display the Title in two lines, and the version of the firmware if the display is hooked up and enabled 
 *  @param enabled - If set to false it will not perform the initialization of the display
 *  @param title1 - the First word of the title of the firmware
 *  @param title1 - the Sconed word of the title of the firmware 
 *  @param version - the version of the firmware to be displayed on startup
 *  @param use_big_text - switch to the 9pt font, if set to false it will display the smaller text on the display
 *  @return - Nothing
 */
void oleddisplay::initDisplay(bool enabled, String title1, String title2, String version, bool use_big_text)
{
  if (enabled)
  {
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // Address 0x3C for 128x64
      Serial.println(F("SSD1306 allocation failed!"));
      for(;;);
    } else{
      Serial.println(F("SSD1306 allocation successful!"));
    }
    delay(2000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    if (use_big_text)
    {
        display.setFont(&FreeMono9pt7b);
    }
    display.println(title1);
    display.println(title2);
    display.print("v");
    display.println(version);
    display.display();    
  }
}

/*
 *  Print the data to the display to update the Pool Temp, Ph and Outside Temp and Humidity, 
 *  If the use_big_text is enabled, it will just display the title, pool temp and ph level.
 *  @param enabled - If set to false it will not perform the initialization of the display
 *  @param title - The title of the firmware to be displayed at the top
 *  @param use_big_text - switch to the 9pt font, if set to false it will display the smaller text on the display
 *  @param pTemp - The Pool Temperature to be displayed
 *  @param myPh - The pH Level to be displayed
 *  @param oTemp - The Outside Temperature to be displayed
 *  @param oHum - The Outside Humidity to be displayed
 *  @return - Nothing
 */
void oleddisplay::PrintDisplay(bool enabled, String title, bool use_big_text, double pTemp, float myPh, double oTemp = 0, double oHum = 0)
{
    if (enabled)
    {
      display.clearDisplay();
      display.setCursor(10, 10);
      display.print(title);
      display.println();
      if (use_big_text)
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
}