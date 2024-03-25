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
        /*
         * Print out the local/outside temperature and humity in the webpage
         */
        String localTemp(double temp, double humidity);
        /*
         * Print out the Pool Temperature for the Web Page
         */
        String poolTemp(double temp);
        /*
         * Display voltage of battery in webpage
         */
        String measureVoltage(double volt);
        /*
         * Print out the Ph Table Row
         */
        String displayPH(float ph);
        String topOfPage(int web_Refresh, String subTitle = "");
        String endOfPage();
        /*
         * Print out the webpage, close connection after the completion of the response and refresh every 5 seconds
         */
        String doWebpageContent(int web_Refresh, bool dolocal, double local_temp, double humidity, bool doPool, double pool_temp, bool doVoltage, double volt, bool doPh, float ph);
    private:
        String webserver::newRow(String description, double value, double greaterThan, String symbol_to_use);
};
 #endif