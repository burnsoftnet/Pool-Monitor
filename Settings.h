/*
  General Setttings section to toggle on and off options
*/
#define VERSION "2024.03.25"              //Current Version of Sketch, also related to the release date
#define USE_BIG_TEXT true                 //Use bigger text for display, which will get rid of outside temp and just so pool temp PT and Ph
#define DISPLAYCONNECTED true             // Switch to toggle functions for the LCD Screen if attached.
#define BUGGERME false                    // Enabled Debug Messages
#define USE_WIFI true                     // Enabled or disable the wifi functionality and the webpage
#define webRefresh 60                     // Web Refresh interval
#define GET_OUTSIDE_TEMP true             // Report back the Outside Local Temperature
#define GET_POOL_TEMP true                // Report back on the Pool Temperature
#define GET_PH true                       // Report back on the pH; 
#define GET_BATTERY_VOLTAGE true          // Report back on the battery voltage
#define OutsideTemp A0                    // Analog Pin sensor is connected to
#define SLAVE_ADDRESS 0x40                // Address for dallas module for the I2C
#define SCREEN_WIDTH 128                  // OLED display width, in pixels
#define SCREEN_HEIGHT 64                  // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C               ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32