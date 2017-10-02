// -------------------------------------------------------------------------------
// Configuration

#define HUJER_NET_ON  // Hujer.net  Default OFF
 
#define DEBUG_OFF                 // Turn ON to allow WiFi startup without OnStep attached 
#define DEBUG_SERIAL_BAUD 74880   // Serial port for debug at 74880 baud is Default for WeMos D1 boot
 
#define GPS_ON   // GPS module Default=OFF

#ifdef GPS_ON
#define AUTO_GPS_ON  // Automatic upload GPS sync data after startup  Default=ON
#define gpsRXpin 13  // GPS RXpin Default GIPO 13 (D7) for WeMos D1 or 4 (D2) or 2 for ESP-01 (connect to TXpin on NEO-6 GPS module)
#define gpsTXpin 12  // GPS TXpin Default GIPO 12 (D6) for WeMos D1 or 5 (D1) or 0 for ESP-01 (connect to RXpin on NEO-6 GPS module)
#define GPSBaud 9600
#endif 



#define LED_PIN_OFF               // Default=OFF, Flashes LED when trying to connect to OnStep, use LED_PIN D4 for WeMos D1 Mini

#define SERIAL_SWAP_OFF           // Default=OFF, ON to swap ESP8266 Serial default location GPIO 1(TX)/3(RX) to 15(TX)/13(RX) useful for WeMos D1 Mini, for example
#define SERIAL_BAUD_DEFAULT 9600  // Default=9600, OnStep must be attached and running it's Serial1 interface this baud rate

// At startup this firmware will attempt to switch to the baud rate below and AFTER success, start WiFi, etc.
//#define SERIAL_BAUD ":SB1#"       // 0=115.2K, 1=57.6K, 2=38.4K Baud, 3=28.8K, 4=19.2K Baud ... Default ":SB1#", if talking to a Mega2560 ":SB4#"
#define SERIAL_BAUD ":SB4#"       // 0=115.2K, 1=57.6K, 2=38.4K Baud, 3=28.8K, 4=19.2K Baud ... Default ":SB1#", if talking to a Mega2560 ":SB4#"

// The settings below are for initialization only, afterward they are stored and recalled from EEPROM and must
// be changed in the web interface OR with a reset (for initialization again) as described in the comments below
//#define TIMEOUT_WEB 15            // Default=15, if talking to a Mega2560 use 60
//#define TIMEOUT_CMD 30            // Default=30, if talking to a Mega2560 use 60

#define TIMEOUT_WEB 60            // Default=15, if talking to a Mega2560 use 60
#define TIMEOUT_CMD 60            // Default=30, if talking to a Mega2560 use 60

// On first startup an AP will appear with an SSID of "ONSTEP", after connecting:
// The web-site is at "192.168.0.1" and the cmd channel is at "192.168.0.1:9999".
//
// If locked out of the ESP8266, a Sketch (like MultiSerial) uploaded to the OnStep MCU (Teensy3.2, Launchpad, etc.) can recover access.
// Simply sending an 'R' in reply to any '#' received will cause a reset to AP only enabled and the default SSID/Password.  Be sure to
// upload OnStep again if necessary and then restart both devices to resume operation.

// -------------------------------------------------------------------------------

