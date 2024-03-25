/*
 *  FILE:     WebServer.h
 *  VERSION:  0.1.00
 *  PURPOSE:  Handler for the Web Server function on the network connected arduino
 */

#ifndef WebServer_h
#define WebServer_h
 
#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#define WEBSERVER_LIB_VERSION "0.1.00"

class webserver
{
    public:
        String localTemp(double temp, double humidity);
        String poolTemp(double temp);
        String measureVoltage(double volt);
        String displayPH(float ph);
        String topOfPage(int web_Refresh, String subTitle = "");
        String doWebpageContent(int web_Refresh, bool dolocal, double local_temp, double humidity, bool doPool, double pool_temp, bool doVoltage, double volt, bool doPh, float ph);
        String endOfPageSub();
        String endOfPage();
        String endOfPageCalBack();
        String doPhCalibrationWeb();
        String doPhCalibrationWeb(int level);
    private:
        String newRow(String description, double value, double greaterThan, String symbol_to_use);
};
 #endif