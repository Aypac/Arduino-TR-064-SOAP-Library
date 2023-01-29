/**
 * Smart_Plug.ino
 *  by René Vollmer, inspired by the Laundry Notifier by Thorsten Godau
 *
 * Tested with FRITZ!DECT210.
 *
 * Example to aquire the power information from two home automation smartplugs
 * TODO: Add code to turn them on and off.
 *
 * Please adjust your sensitive data in the file/tab `arduino_secrets.h`
 *  and the settings below.
 *
 *  created on: 11.01.2023
 */
#include "arduino_secrets.h"

#if defined(ESP8266)
  //Imports for ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266WiFiMulti.h>
  #include <ESP8266HTTPClient.h>
  ESP8266WiFiMulti WiFiMulti;
#elif defined(ESP32)
  //Imports for ESP32
  #include <WiFi.h>
  #include <WiFiMulti.h>
  #include <HTTPClient.h>
  WiFiMulti WiFiMulti;
#endif

#include <tr064.h>

//-------------------------------------------------------------------------------------
// Telephone and power meter settings
//-------------------------------------------------------------------------------------

const char *FbApiAIN01  = "11657 1234567";    // AIN of the first smart plug

//-------------------------------------------------------------------------------------
// Initializations. No need to change these.
//-------------------------------------------------------------------------------------
// Array to contain the four power values for averaging
float   afPwrAIN01[4] = { 0.0, 0.0, 0.0, 0.0 };
uint8_t u8IdxPwrAIN01 = 0;

// Power state of the smart plug
//  0: no power drawn, 1: power drawn
uint8_t u8StateAIN01 = 0;

// Threshold for the power state
#define u8StateAIN_TRESHOLD 5.0

// Read every 5s
#define u32Interval 5000

// Last time we read time time
uint32_t u32MillisTmp;

// TR-064 connection
#if TR_PROTOCOL == 0
	TR064 connection(TR_PORT, TR_IP, TR_USER, TR_PASS);
#else
	#if TRANSPORT_PROTOCOL == 1
		Protocol protocol = Protocol::useHttpsInsec;
	#else
		Protocol protocol = Protocol::useHttps;
	#endif
	X509Certificate myX509Certificate = TR_ROOT_CERT;
	TR064 connection(TR_PORT, TR_IP, TR_USER, TR_PASS, protocol, myX509Certificate);
#endif

//------------------------------------------------

//###########################################################################################
//############################ OKAY, LET'S DO THIS! #########################################
//###########################################################################################

void setup() {
  // See more https://techtutorialsx.com/2017/01/21/esp8266-watchdog-functions/
  // Start the serial connection
  // Not required for production, but helpful for development.
  // You might also want to change the baud-rate.
  Serial.begin(115200);

  // Clear some space in the serial monitor.
  if(Serial) {
    Serial.println();
    Serial.println();
    Serial.println();
  }
  
  // Wait a few secs for warm-up (dunno why, was in the default code for http connections).
  delay(5000);

  // Connect to wifi
  ensureWIFIConnection();
  
  // Set debug level. Available levels are:
  //  DEBUG_NONE   ///< Print no debug messages whatsoever (production)
  //  DEBUG_ERROR  ///< Only print error messages
  //  DEBUG_WARNING  ///< Only print error and warning messages
  //  DEBUG_INFO   ///< Print error, warning and info messages
  //  DEBUG_VERBOSE  ///< Print all messages
  connection.debug_level = connection.DEBUG_WARNING;
  if(Serial) Serial.setDebugOutput(true);
  
  // The following line retrieves a list of all available services on the router.
  // It is not required for operation, so it can be safely commented and save
  //   ressources on the microcontroller. However, it can be helpful for debugging
  //   and development to keep it activated.
  if(Serial) Serial.printf("Initialize TR-064 connection\n\n");
  connection.init();

  u32MillisTmp = millis();
}


void loop(void) {
  float fAvgPwrAIN01;

  uint8_t u8count;

  // Smartplug processing
  if (( millis() - u32MillisTmp ) > u32Interval) {

    afPwrAIN01[u8IdxPwrAIN01] = getPwrAIN(FbApiAIN01);

    if ( u8StateAIN01 == 0 && afPwrAIN01[u8IdxPwrAIN01] > u8StateAIN_TRESHOLD ) {
      u8StateAIN01 = 1;  // No power -> Power drawn
    }

    if ( ++u8IdxPwrAIN01 > 3 ) u8IdxPwrAIN01 = 0;

    delay(100);

    // Accumulate arrays and average
    fAvgPwrAIN01 = 0;
    for ( u8count = 0; u8count < 4; u8count++ ) {
      fAvgPwrAIN01 += afPwrAIN01[u8count];
    }

    fAvgPwrAIN01 = fAvgPwrAIN01 / 4.0;

    Serial.printf("AIN01: %.2f W (avg.), State; %d\r\n", fAvgPwrAIN01, u8StateAIN01);

    if ( u8StateAIN01 == 1 && fAvgPwrAIN01 <= u8StateAIN_TRESHOLD ) {
      u8StateAIN01 = 0;  // Power -> No power drawn
    }

    u32MillisTmp = millis();
    delay(100);
  }
}



float getPwrAIN(const char *FbApiAIN) {
  String params[][2] = {{"NewAIN", FbApiAIN}};
  String req[][2] = {{"NewMultimeterPower", ""}};

  connection.action("X_AVM-DE_Homeauto:1", "GetSpecificDeviceInfos", params, 1, req, 1);

  float power = (float)((req[0][1]).toInt()) / 100.0;

  return power;
}

/**
 * Makes sure there is a WIFI connection and waits until it is (re-)established.
 */
void ensureWIFIConnection() {
  if ((WiFiMulti.run() != WL_CONNECTED)) {
    WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
    //WiFiMulti.run();
    while ((WiFiMulti.run() != WL_CONNECTED)) {
      delay(100);
    }
  }
}
