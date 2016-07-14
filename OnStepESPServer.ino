/*
 * Title       OnStepESPServer
 * by          Howard Dutton
 *
 * Copyright (C) 2016 Howard Dutton
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * 
 *
 * Revision History, see GitHub
 *
 *
 * Author: Howard Dutton
 * http://www.stellarjourney.com
 * hjd1964@gmail.com
 *
 * Description
 *
 * ESP8266-01 OnStep control
 *
 */
 
// -------------------------------------------------------------------------------
// Configuration
#define DEBUG_OFF
#define Default_Password "password"
#define SERIAL_BAUD ":SB1#" // 0=115.2K, 1=57.6K, 2=38.4K Baud
// -------------------------------------------------------------------------------

#define Product "On-Esp"
#define Version "0.5"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiAP.h>
#include <EEPROM.h>

char masterPassword[40]=Default_Password;

bool accessPointEnabled=true;
bool stationEnabled=false;
bool stationDhcpEnabled=true;

char wifi_sta_ssid[40]="";
char wifi_sta_pwd[40]="";

IPAddress wifi_sta_ip = IPAddress(192,168,0,1);
IPAddress wifi_sta_gw = IPAddress(192,168,0,1);
IPAddress wifi_sta_sn = IPAddress(255,255,255,0);

char wifi_ap_ssid[40]="ONSTEP";
char wifi_ap_pwd[40]="password";
byte wifi_ap_ch=7;

IPAddress wifi_ap_ip = IPAddress(192,168,0,1);
IPAddress wifi_ap_gw = IPAddress(192,168,0,1);
IPAddress wifi_ap_sn = IPAddress(255,255,255,0);

int WebTimeout=15;
int CmdTimeout=30;

ESP8266WebServer server(80);

WiFiServer cmdSvr(9999);
WiFiClient cmdSvrClient;
unsigned long clientTime = 0;

char writeBuffer[40]="";
int writeBufferPos=0;

enum Errors {ERR_NONE, ERR_MOTOR_FAULT, ERR_ALT, ERR_LIMIT_SENSE, ERR_DEC, ERR_AZM, ERR_UNDER_POLE, ERR_MERIDIAN, ERR_SYNC};
Errors lastError = ERR_NONE;

#define PierSideNone     0
#define PierSideEast     1
#define PierSideWest     2
#define PierSideBest     3
#define PierSideFlipWE1  10
#define PierSideFlipWE2  11
#define PierSideFlipWE3  12
#define PierSideFlipEW1  20
#define PierSideFlipEW2  21
#define PierSideFlipEW3  22
byte pierSide = PierSideNone;

const char* html_head1 = "<!DOCTYPE HTML>\r\n<html>\r\n<head>\r\n";
const char* html_headerPec = "<meta http-equiv=\"refresh\" content=\"5; URL=/pec.htm\">\r\n";
const char* html_headerIdx = "<meta http-equiv=\"refresh\" content=\"5; URL=/index.htm\">\r\n";
const char* html_head2 = "</head>\r\n<body bgcolor=\"#26262A\">\r\n";

const char* html_main_css1 = "<STYLE>\r\n";
const char* html_main_css2 = ".a { background-color: #111111; }\r\n .t { padding: 15px; border: 15px solid #551111;\r\n";
const char* html_main_css3 = " margin: 25px; color: #999999; background-color: #111111; }\r\n input { width:4em; font-weight: bold; background-color: #A01010; margin: 2px; padding: 2px 2px; }\r\n";
const char* html_main_css4 = ".b { padding: 30px; border: 2px solid #551111; margin: 30px; color: #999999; background-color: #111111; }\r\n";
const char* html_main_css5 = "select { width:4em; font-weight: bold; background-color: #A01010; padding: 2px 2px; }\r\n .c { color: #A01010; font-weight: bold; }\r\n";
const char* html_main_css6 = "h1 { text-align: right; }\r\n a:hover, a:active { background-color: red; }\r\n .g { color: #105010; font-weight: bold; }";
const char* html_main_css7 = "a:link, a:visited { background-color: #332222; color: #a07070; border:1px solid red; padding: 5px 10px;";
const char* html_main_css8 = " margin: none; text-align: center; text-decoration: none; display: inline-block; }\r\n";
const char* html_main_css9 = "button { background-color: #A01010; font-weight: bold; border-radius: 5px; font-size: 12px; margin: 2px; padding: 4px 8px; }\r\n</STYLE>\r\n";

const char* html_links1 = "<a href=\"/index.htm\">Status</a><a href=\"/control.htm\">Control</a>";
const char* html_links2 = "<a href=\"/guide.htm\">Guide</a><a href=\"/pec.htm\">PEC</a><a href=\"/settings.htm\">Settings</a>";
const char* html_links3 = "<a href=\"/wifi.htm\">ESP8266 Config</a><a href=\"/config.htm\">Config.h</a><br />";

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void){

  EEPROM.begin(1024);

Restart:

  // EEPROM Init
  if ((EEPROM_readInt(0)!=8266) || (EEPROM_readInt(2)!=0)) {
    EEPROM_writeInt(0,8266);
    EEPROM_writeInt(2,0);

    EEPROM_writeInt(4,(int)accessPointEnabled);
    EEPROM_writeInt(6,(int)stationEnabled);
    EEPROM_writeInt(8,(int)stationDhcpEnabled);

    EEPROM_writeInt(10,(int)WebTimeout);
    EEPROM_writeInt(12,(int)CmdTimeout);

    EEPROM_writeString(100,wifi_sta_ssid);
    EEPROM_writeString(150,wifi_sta_pwd);
    EEPROM_writeString(200,masterPassword);
    EEPROM.write(20,wifi_sta_ip[0]); EEPROM.write(21,wifi_sta_ip[1]); EEPROM.write(22,wifi_sta_ip[2]); EEPROM.write(23,wifi_sta_ip[3]);
    EEPROM.write(24,wifi_sta_gw[0]); EEPROM.write(25,wifi_sta_gw[1]); EEPROM.write(26,wifi_sta_gw[2]); EEPROM.write(27,wifi_sta_gw[3]);
    EEPROM.write(28,wifi_sta_sn[0]); EEPROM.write(29,wifi_sta_sn[1]); EEPROM.write(30,wifi_sta_sn[2]); EEPROM.write(31,wifi_sta_sn[3]);

    EEPROM_writeString(500,wifi_ap_ssid);
    EEPROM_writeString(550,wifi_ap_pwd);
    EEPROM_writeInt(50,(int)wifi_ap_ch);
    EEPROM.write(60,wifi_ap_ip[0]); EEPROM.write(61,wifi_ap_ip[1]); EEPROM.write(62,wifi_ap_ip[2]); EEPROM.write(63,wifi_ap_ip[3]);
    EEPROM.write(70,wifi_ap_gw[0]); EEPROM.write(71,wifi_ap_gw[1]); EEPROM.write(72,wifi_ap_gw[2]); EEPROM.write(73,wifi_ap_gw[3]);
    EEPROM.write(80,wifi_ap_sn[0]); EEPROM.write(81,wifi_ap_sn[1]); EEPROM.write(82,wifi_ap_sn[2]); EEPROM.write(83,wifi_ap_sn[3]);
    EEPROM.commit();
  } else {  
    accessPointEnabled=EEPROM_readInt(4);
    stationEnabled=EEPROM_readInt(6);
    stationDhcpEnabled=EEPROM_readInt(8);

    WebTimeout=EEPROM_readInt(10);
    CmdTimeout=EEPROM_readInt(12);

//    EEPROM_readString(100,wifi_sta_ssid);
//    EEPROM_readString(150,wifi_sta_pwd);
    EEPROM_readString(200,masterPassword);
    wifi_sta_ip[0]=EEPROM.read(20); wifi_sta_ip[1]=EEPROM.read(21); wifi_sta_ip[2]=EEPROM.read(22); wifi_sta_ip[3]=EEPROM.read(23);
    wifi_sta_gw[0]=EEPROM.read(24); wifi_sta_gw[1]=EEPROM.read(25); wifi_sta_gw[2]=EEPROM.read(26); wifi_sta_gw[3]=EEPROM.read(27);
    wifi_sta_sn[0]=EEPROM.read(28); wifi_sta_sn[1]=EEPROM.read(29); wifi_sta_sn[2]=EEPROM.read(30); wifi_sta_sn[3]=EEPROM.read(31);

    EEPROM_readString(500,wifi_ap_ssid);
    EEPROM_readString(550,wifi_ap_pwd);
    wifi_ap_ch=EEPROM_readInt(50);
    wifi_ap_ip[0]=EEPROM.read(60); wifi_ap_ip[1]=EEPROM.read(61); wifi_ap_ip[2]=EEPROM.read(62); wifi_ap_ip[3]=EEPROM.read(63);
    wifi_ap_gw[0]=EEPROM.read(70); wifi_ap_gw[1]=EEPROM.read(71); wifi_ap_gw[2]=EEPROM.read(72); wifi_ap_gw[3]=EEPROM.read(73);
    wifi_ap_sn[0]=EEPROM.read(80); wifi_ap_sn[1]=EEPROM.read(81); wifi_ap_sn[2]=EEPROM.read(82); wifi_ap_sn[3]=EEPROM.read(83);    
  }

Again:
  char c=0;

#ifdef DEBUG_OFF
  Serial.begin(9600);

  // clear the buffers and any noise on the serial lines
  for (int i=0; i<3; i++) {
    Serial.print(":#");
    delay(500);
    while (Serial.available()>0) { c=Serial.read(); }
  }

  // safety net
  if ((c=='R') || (!accessPointEnabled && !stationEnabled)) {
    // reset EEPROM values, triggers an init
    EEPROM_writeInt(0,0); EEPROM_writeInt(2,0);
    goto Restart;
  }
 
  // switch OnStep Serial1 up to ? baud
  Serial.print(SERIAL_BAUD);
  delay(50);
  int count=0; c=0;
  while (Serial.available()>0) { count++; c=Serial.read(); }
  if ((c='1') && (count==1)) {
    if (!strcmp(SERIAL_BAUD,":SB0#")) Serial.begin(115200); else
    if (!strcmp(SERIAL_BAUD,":SB1#")) Serial.begin(57600); else
    if (!strcmp(SERIAL_BAUD,":SB2#")) Serial.begin(38400); else Serial.begin(9600);
  } else {
    delay(5000);
    goto Again;
  }
#else
  Serial.begin(115200);
  delay(10000);
  
  Serial.println(accessPointEnabled);
  Serial.println(stationEnabled);
  Serial.println(stationDhcpEnabled);

  Serial.println(WebTimeout);
  Serial.println(CmdTimeout);

  Serial.println(wifi_sta_ssid);
  Serial.println(wifi_sta_pwd);
  Serial.println(wifi_sta_ip.toString());
  Serial.println(wifi_sta_gw.toString());
  Serial.println(wifi_sta_sn.toString());

  Serial.println(wifi_ap_ssid);
  Serial.println(wifi_ap_pwd);
  Serial.println(wifi_ap_ch);
  Serial.println(wifi_ap_ip.toString());
  Serial.println(wifi_ap_gw.toString());
  Serial.println(wifi_ap_sn.toString());

#endif
  
//  stationDhcpEnabled=true;
//  stationEnabled=true;
//  accessPointEnabled=false;

  if (!stationDhcpEnabled) WiFi.config(wifi_sta_ip, wifi_sta_gw, wifi_sta_sn);
  if (accessPointEnabled) WiFi.softAPConfig(wifi_ap_ip, wifi_ap_gw, wifi_ap_sn);
  
  if (accessPointEnabled && !stationEnabled) {
    //WiFi.disconnect();
    WiFi.softAP(wifi_ap_ssid, wifi_ap_pwd, wifi_ap_ch);
    WiFi.mode(WIFI_AP);
  } else
  if (!accessPointEnabled && stationEnabled) {
    WiFi.softAPdisconnect(true);
    WiFi.begin(wifi_sta_ssid, wifi_sta_pwd);
    WiFi.mode(WIFI_STA);
  } else
  if (accessPointEnabled && stationEnabled) {
    WiFi.softAP(wifi_ap_ssid, wifi_ap_pwd, wifi_ap_ch);
    WiFi.begin(wifi_sta_ssid, wifi_sta_pwd);
    WiFi.mode(WIFI_AP_STA);
  }

  // clear the buffers and any noise on the serial lines
  for (int i=0; i<3; i++) {
    Serial.print(":#");
    delay(50);
    while (Serial.available()>0) { c=Serial.read(); }
  }

  // Wait for connection
  if (stationEnabled) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
  }

  server.on("/", handleRoot);
  server.on("/index.htm", handleRoot);
  server.on("/settings.htm", handleSettings);
  server.on("/control.htm", handleControl);
  server.on("/guide.htm", handleGuide);
  server.on("/guide.txt", handleGuideAjax);
  server.on("/pec.htm", handlePec);
  server.on("/wifi.htm", handleWifi);
  server.on("/config.htm", handleConfig);
  
  server.onNotFound(handleNotFound);

  cmdSvr.begin();
  cmdSvr.setNoDelay(true);
  server.begin();
  
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();

  // disconnect client
  if (cmdSvrClient && (!cmdSvrClient.connected())) cmdSvrClient.stop();
  if (cmdSvrClient && ((long)(clientTime-millis())<0)) cmdSvrClient.stop();

  // new client
  if (!cmdSvrClient && (cmdSvr.hasClient())) {
    // find free/disconnected spot
    cmdSvrClient = cmdSvr.available();
    clientTime=millis()+2000UL;
  }

  // check clients for data, if found get the command, send cmd and pickup the response, then return the response
//  if (cmdSvrClient && cmdSvrClient.connected()) 
  {
    while (cmdSvrClient && cmdSvrClient.connected() && (cmdSvrClient.available()>0)) {
      // get the data
      byte b=cmdSvrClient.read();
      writeBuffer[writeBufferPos]=b; writeBufferPos++; if (writeBufferPos>39) writeBufferPos=39; writeBuffer[writeBufferPos]=0;

      // send cmd and pickup the response
      if (b=='#') {
        char readBuffer[40]="";
        readLX200Bytes(writeBuffer,readBuffer,CmdTimeout); writeBuffer[0]=0; writeBufferPos=0;

        // return the response, if we have one
        if (strlen(readBuffer)>0) {
          if (cmdSvrClient && cmdSvrClient.connected()) {
            cmdSvrClient.print(readBuffer);
            delay(2);
          }
        }

       // cmdSvrClient.stop();
      } else server.handleClient();
    }
  }
}
