/*
 *  FILE:     OledDisplay.h
 *  VERSION:  0.1.00
 *  PURPOSE:  Handler for the OLEd function on the arduino
 */

#ifndef OledDisplay_h
#define OledDisplay_h

#include "Settings.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#define OLEDDISPLAY_LIB_VERSION "0.1.00"

class oleddisplay
{
    public:
      void initDisplay(bool enabled, String title1, String title2, String version, bool use_big_text);
      void PrintDisplay(bool enabled, String title, bool use_big_text, double pTemp, float myPh, double oTemp = 0, double oHum = 0);
    private:

};
 #endif