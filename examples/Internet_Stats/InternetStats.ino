/**
 * InternetStats.ino
 *  by René Vollmer
 *  
 * Example code for polling internet stats, like the currently used and available speeds.
 * 
 * Please adjust your sensitive data in the file/tab `arduino_secrets.h`
 *  
 * Created on: 11.01.2023
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
// Initializations. No need to change these.
//-------------------------------------------------------------------------------------

// TR-064 connection
TR064 connection(TR_PORT, TR_IP, TR_USER, TR_PASS);

// -------------------------------------------------------------------------------------

//###########################################################################################
//############################ OKAY, LET'S DO THIS! #########################################
//###########################################################################################

void setup() {
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
  //  DEBUG_NONE         ///< Print no debug messages whatsoever (production)
  //  DEBUG_ERROR        ///< Only print error messages
  //  DEBUG_WARNING      ///< Only print error and warning messages
  //  DEBUG_INFO         ///< Print error, warning and info messages
  //  DEBUG_VERBOSE      ///< Print all messages
    connection.debug_level = connection.DEBUG_WARNING;
  if(Serial) Serial.setDebugOutput(true);
  
  // The following line retrieves a list of all available services on the router.
  // It is not required for operation, so it can be safely commented and save
  //   ressources on the microcontroller. However, it can be helpful for debugging
  //     and development to keep it activated.
  if(Serial) Serial.printf("Initialize TR-064 connection\n\n");
  connection.init();
}

void loop() {
    ensureWIFIConnection();
  
    // Query up- and down-link speed and transfer rate
    // Query external TR_IP address
    // Print results
    if(Serial) Serial.println("-------------------------------------------");

    delay(1000);
	
//###########################################################################################
//############################          TODO        #########################################
//###########################################################################################
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
