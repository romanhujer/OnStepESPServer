/* GPS module for OnStepESPServer

   by Roman Hujer (roman@hujer.net)

   Copyright (C) 2017  Roman Hujer

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef GPS_ON

bool GPS_LOCK = false;
bool GPS_sync  = false;

bool GPS_data_OK = true;
bool GPS_sats_OK  = true;
bool GPS_wiring_OK = true;

bool GPS_u_offset = false;
bool GPS_u_date = false;
bool GPS_u_time = false;
bool GPS_u_lat = false;
bool GPS_u_lon = false;

byte g_sats = 0;
float g_alt = 0;
int g_year = 0;
byte g_month = 0;
byte g_day = 0;
byte g_hour = 0;
byte g_minute = 0;
byte g_second = 0;
float g_lat = 0;
float g_lon = 0;


//byte  g_wait = 0;


const char* html_gpsGPS1 = "Latest GPS data: <br/>";
const char html_gpsGPS2[] =
  "Longitude = <font class=\"c\">%s&quot;</font>,&nbsp; Latitude = <font class=\"c\">%s&quot;</font>,&nbsp; Altitude [m] = <font class=\"c\">%d</font><br />"
  "Satelite  = <font class=\"c\">%d</font>,&nbsp; Date = <font class=\"c\">%04d/%02d/%02d</font>,&nbsp; Time (UTC) = <font class=\"c\">%02d:%02d:%02d</font>";
const char html_gpsGPS3[] =
  "<form method=\"get\" action=\"/settings.htm\"><button name=\"gp\" value=\"on\" type=\"submit\">GPS Upload</button>&nbsp;"
  "(Upload current GPS Date, Time, Longitude and Latitude)</form><br />\r\n";
// const char* html_gpsGPS4 = "<br /><br />";
const char* html_gpsGPS4 = "<font class=\"c\">No GPS detected: check wiring.</font><br /><br />\r\n";
const char* html_gpsGPS5 = "<font class=\"c\">No GPS satelite sync.</font><br /><br />\r\n";
const char* html_gpsGPS6 = "<font class=\"c\">Wait for multiple satellites.</font><br /><br />\r\n";


void OnStepGPS() {
  if (GPS_LOCK ) return;
  GPS_LOCK =true;
  
 //read GPS data
 // for (unsigned long start = millis(); millis() - start < 1000;) {
    while (gpsSerial.available() > 0)
      if (gps.encode(gpsSerial.read()))  ProcGPSData();

    if ( millis() > 5000 && gps.charsProcessed() < 10) {
      GPS_sync = false;
      if ( GPS_wiring_OK ) {
        GPS_wiring_OK = false;
#ifdef DEBUG_ON
        Serial.println("\r\nNo GPS detected: check wiring.");
#endif
      }
    } else GPS_wiring_OK = true;
//  }
  GPS_LOCK = false;
}


void ProcGPSData () {
  char temp[20];
  if (gps.satellites.isValid() && gps.location.isValid() && gps.date.isValid() && gps.time.isValid() && gps.altitude.isValid() ) {

    GPS_data_OK = true;

    g_sats   = gps.satellites.value();
    g_alt    = gps.altitude.meters();
    g_year   = gps.date.year();
    g_month  = gps.date.month();
    g_day    = gps.date.day();
    g_hour   = gps.time.hour();
    g_minute = gps.time.minute();
    g_second = gps.time.second();
    g_lat    = gps.location.lat();
    g_lon    = gps.location.lng();



#ifdef DEBUG_ON
    byte last_sats;
    if ( last_sats != g_sats) {
      last_sats = g_sats;
      Serial.println(":");
      Serial.print(" SAT: " );
      Serial.print( g_sats  );
/*
      Serial.print(" LAT: " );
      Serial.print( g_lat  );
      Serial.print(" LON: " );
      Serial.print( g_lon );
      Serial.print(" ALT: " );
      Serial.println( g_alt );
      
      sprintf(temp, "%02d/%02d/%02d", g_day, g_month, g_year );
      Serial.print(" Date: " );
      Serial.println( temp );
      sprintf(temp, "%02d:%02d:%02d", g_hour, g_minute, g_second);
      Serial.print(" Time: " );
      Serial.println( temp );
 */
    }
#endif

    if (g_sats > 3 ) {

      GPS_sats_OK = true;

      if ( GPS_upload_request ) {

        // Set UTC offset to 0
        if ( ! GPS_u_offset ) {
          GPS_u_offset = true;
          Serial.print(":SG+00#");
  //        return;
        }
        // Set date:  :SCMM/DD/YY#
        if ( ! GPS_u_date ) {
          GPS_u_date = true;
          sprintf(temp, ":SC%02d/%02d/%02d#", g_month, g_day, g_year - 2000);
          Serial.print(temp );
  //        return;
        }
        // Set time:  :SLHH:MM:SS#
        if ( ! GPS_u_time ) {
          GPS_u_time = true;
          sprintf(temp, ":SL%02d:%02d:%02d#", g_hour, g_minute, g_second);
          Serial.print(temp );
  //        return;
        }
        // Set Latitude (for current site)  :StsDD*MM#
        if ( ! GPS_u_lat ) {
          GPS_u_lat = true;
          if ( g_lat > 0 ) {
            sprintf(temp, ":St+%02d*%02d#", int(g_lat), i_g_min( g_lat ));
          } else {
            sprintf(temp, ":St-%02d*%02d#", int(-g_lat), i_g_min( -g_lat ));
          }
          Serial.print(temp );
 //         return;
        }

        // Set Longitude (for current site) :SgDDD*MM#
 
       if ( ! GPS_u_lon ) {
         
        if ( g_lon > 0 ) {
          sprintf(temp, ":Sg-%03d*%02d#", int(g_lon), i_g_min( g_lon ) );
        } else {
          sprintf(temp, ":Sg+%03d*%02d#", int(-g_lon), i_g_min( -g_lon ));
        }
        Serial.print( temp );
 //         return;
        
       }
        GPS_sync  = true;
        GPS_u_offset = false;
        GPS_u_date = false;
        GPS_u_time = false;
//        GPS_u_lat = false;
//        GPS_u_lot = false;
        GPS_upload_request = false; // it is not necessary to send the coordinates in the next cycle
      }  // endif ( GPS_upload_request )

    } else  {  // elsif (g_sats > 3 )
      GPS_sync  = false;
      if ( GPS_sats_OK ) {
        GPS_sats_OK = false;
#ifdef DEBUG_ON
        Serial.println("A small number of satellites.");
#endif
      }
    } // endif (g_sats > 3 )

  } else  { // elsif (gps.*.isValid() )
    GPS_sync  = false;
    if ( GPS_data_OK ) {
      GPS_data_OK = false;
#ifdef DEBUG_ON
      Serial.println("Invalid GPS Data");
#endif
    }
  } // endif (gps.*.isValid() )

} //  end of void ProcGPSData ()

int i_g_min ( float g_in)
{
  return int((g_in - int(g_in) * 1.) * 60. + 0.5);
}

int i_g_min_s ( float g_in)
{
  return int((g_in - int(g_in) * 1.) * 60. );
}

int i_g_sec_s ( float g_in)
{
  return int(((g_in - int(g_in) * 1.) * 3600. + 0.5 ) - int((g_in - int(g_in) * 1.) * 60.) * 60);
}

#endif
