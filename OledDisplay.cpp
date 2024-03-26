#include "OledDisplay.h"

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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

void oleddisplay::PrintDisplay(bool enabled, String title, bool use_big_text, double pTemp, float myPh, double oTemp = 0, double oHum = 0)
{
    if (enabled)
    {
      display.clearDisplay();
      display.setCursor(10, 10);
      display.println(title);
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