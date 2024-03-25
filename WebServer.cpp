#include "WebServer.h"

const String table_row_start = "<tr>";
const String table_row_end = "</tr>";
const String table_column_start = "<td>";
const String table_column_end = "</td>";
const String NL = "\n";
const String OFFLINE = "<font color=red>OFFLINE</font>";

String webserver::newRow(String description, double value, double greaterThan, String symbol_to_use)
{
    String s_ans = table_row_start + table_column_start;
    s_ans += description + " " + table_column_end + NL;
    if (value > greaterThan)
        s_ans += table_column_start + value + " " + symbol_to_use + table_column_end + NL;
    else {
        s_ans += table_column_start + OFFLINE + table_column_end + NL;
    }
    s_ans += table_row_end;
}
/*
 * Print out the local/outside temperature and humity in the webpage
 */
String webserver::localTemp(double temp, double humidity)
{
    String s_ans = table_row_start + table_column_start + NL;
    s_ans += "Outside Temperature " + table_column_end + NL;
    if (temp > 32)
        s_ans += table_column_start + temp + " F" + table_column_end + NL;
    else {
        s_ans += table_column_start + OFFLINE + table_column_end + NL;
    }
    s_ans += table_row_end;

    s_ans += table_row_start + table_column_start + NL;
    s_ans += "Outside Humidity " + table_column_end + NL;
    if (humidity > 0)
        s_ans += table_column_start + temp + " %" + table_column_end + NL;
    else {
        s_ans += table_column_start + OFFLINE + table_column_end + NL;
    }
    s_ans += table_row_end + NL;
    return s_ans;
}
/*
 * Print out the Pool Temperature for the Web Page
 */
String webserver::poolTemp(double temp)
{
    String s_ans = table_row_start + table_column_start + NL;
    s_ans += "Pool Temperature " + table_column_end + NL;
    if (temp > 0)
        s_ans += table_column_start + temp + " F" + table_column_end + NL;
    else {
        s_ans += table_column_start + OFFLINE + table_column_end + NL;
    }
    s_ans += table_row_end + NL;

    return s_ans;
}

/*
 * Display voltage of battery in webpage
 */
String webserver::measureVoltage(double volt)
{
    String s_ans = table_row_start + table_column_start + NL;
    s_ans += "Battery Voltage " + table_column_end + NL;
    if (volt > 0)
        s_ans += table_column_start + volt + " vdc" + table_column_end + NL;
    else {
        s_ans += table_column_start + OFFLINE + table_column_end + NL;
    }
    s_ans += table_row_end + NL;

    return s_ans;
}
/*
 * Print out the Ph Table Row
 */
String webserver::displayPH(float ph)
{
    String s_ans = table_row_start + table_column_start + NL;
    s_ans += "Ph Level " + table_column_end + NL;
    if (ph > 0 && ph < 6)
    {
      s_ans += table_column_start +"<font color=darkyellow>" + ph + "</font>" + table_column_end + NL;
    } else if (ph > 6 && ph < 8)
    {
      s_ans += table_column_start +"<font color=green>" + ph + "</font>" + table_column_end + NL;
    } else if (ph > 8 && ph < 10)
    {
      s_ans += table_column_start +"<font color=orange>" + ph + "</font>" + table_column_end + NL;
    } else if (ph > 12 && ph < 14 )
    {
      s_ans += table_column_start +"<font color=red>" + ph + "</font>" + table_column_end + NL;
    } else
    {
      s_ans += table_column_start + "<font color=red>" + OFFLINE + "</font>" + table_column_end + NL;
    }
    s_ans += table_row_end + NL;
    return s_ans;
}


String webserver::topOfPage(int web_Refresh, String subTitle = "")
{
    String s_ans = "HTTP/1.1 200 OK" + NL;
    s_ans += "Content-Type: text/html" + NL;
    s_ans += "Connection: close" + NL;  
    if (web_Refresh > 0)
    {
      s_ans += "Refresh: " + web_Refresh + NL; 
    }
    s_ans += NL;
    s_ans += "<!DOCTYPE HTML>" + NL;
    s_ans += "<html>" + NL;
    s_ans += "<center>" + NL;
    if (subTitle.length() > 0)
    {
      s_ans += "<b><h1>Pool Monitor" + subTitle + "</h1></b>" + NL;
    } else{
      s_ans += "<b><h1>Pool Monitor</h1></b>" + NL;
    }
    s_ans += "</br>" + NL;
    s_ans += "<table border=1>" + NL;
    return s_ans;
}

String webserver::endOfPage()
{
    String s_ans = "</table>" + NL;
    s_ans += "</br>" + NL;
    s_ans += "<a href=\"/cal\">Click here to calibrate pH</a>" + NL;
    s_ans += "<center>" + NL;
    s_ans += "</html>" + NL;
    return s_ans;
}
/*
 * Print out the webpage, close connection after the completion of the response and refresh every 5 seconds
 */
String webserver::doWebpageContent(int web_Refresh, bool dolocal, double local_temp, double humidity, bool doPool, double pool_temp, bool doVoltage, double volt, bool doPh, float ph)
{
  String s_ans = topOfPage(web_Refresh);
  if (dolocal)
  {
    s_ans += localTemp(local_temp, humidity);
  }
  if (doPool)
  {
    s_ans += poolTemp(pool_temp);
  }
  if (doVoltage)
  {
    s_ans += measureVoltage(volt);
  }

  if (doPh)
  {
    s_ans += displayPH(ph);
  }

  s_ans += endOfPage();
  return s_ans;
}