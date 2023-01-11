/**
 * Caller.ino
 *  Example code for placing internal DECT phone calls.
 *
 *  Oliver-André Urban
 *   improved by
 *    Karsten Sauer (saak2820)
 *   updated by
 *      René Vollmer
 * 
 * Please adjust your sensitive data in the file/tab `arduino_secrets.h`
 *  
 * Created on: 07.06.2017
 *  Latest update: 11.01.2023
 */
#include "arduino_secrets.h"

#include <Arduino.h>
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
// Hardware settings
//-------------------------------------------------------------------------------------
// BELL_BUTTON - you can connect a separate button to this pin or an opto-coupler
//  (depends on the microcontroller and circuit you use)
//   for example: use a resistor and an opto-coupler to connect to a doorbell
#define BELL_BUTTON 0

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
  
  // Define button port as input
  pinMode(BELL_BUTTON, INPUT);
  
  // Wait a few secs for warm-up (dunno why, was in the default code for http connections).
  delay(5000);
  
  // Connect to wifi
  ensureWIFIConnection();
  
  // Define button port as input
  pinMode(BELL_BUTTON, INPUT);
  
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
  if (digitalRead(BELL_BUTTON) == LOW) {
    if (Serial) {
      Serial.println();
      Serial.printf("Button pressed");
    }
    callWahlhilfe();
    // callDect();
    // char* status=getStatus();
    delay(20000); // 20s
    if(Serial) Serial.println("-------------------------------------------");
  } else {
    // You can add a debug message here if you want.
  }
}


void callWahlhilfe() {
  ensureWIFIConnection();

  if(connection.state()<0){
    connection.init();
  }
  String params[][2] = {{"NewX_AVM-DE_PhoneNumber", "**799"}};
  String req[][2] = {{}};
  connection.action("X_VoIP:1", "X_AVM-DE_DialNumber", params, 1, req, 0);
  //connection.action("urn:dslforum-org:service:X_VoIP:1", "X_AVM-DE_DialNumber", params, 1, req, 0);
  
  // without loading available services through init() you have to set the url
  //connection.action("X_VoIP:1", "X_AVM-DE_DialNumber", params, 1, req, 0, "/upnp/control/x_voip");
}

void callDect() {
  ensureWIFIConnection();

  String params[][2] = {{"NewAIN", "12345 0123456"}, {"NewSwitchState", "TOGGLE"}};
  connection.action("X_AVM-DE_Homeauto:1", "SetSwitch", params, 2);
  // connection.action("urn:dslforum-org:service:X_AVM-DE_Homeauto:1", "SetSwitch", params, 2);
  
  // without loading available services through init() you have to set the url
  //connection.action("X_AVM-DE_Homeauto:1", "SetSwitch", params, 2, "/upnp/control/x_homeauto");
}

String getStatus() {
  ensureWIFIConnection();
  
  String paramsb[][2] = {{"NewAIN", "12345 0123456"}};
  String reqb[][2] = {{"NewDeviceId", ""}, {"NewSwitchState", ""}};
  connection.action("X_AVM-DE_Homeauto:1", "GetSpecificDeviceInfos", paramsb, 1, reqb, 2);
  //connection.action("urn:dslforum-org:service:X_AVM-DE_Homeauto:1", "GetSpecificDeviceInfos", paramsb, 1, reqb, 2);
 
  // without loading available services through init() you have to set the url
  //connection.action("X_AVM-DE_Homeauto:1", "GetSpecificDeviceInfos", paramsb, 1, reqb, 2, "/upnp/control/x_homeauto");
  return reqb[1][1];
}

/**
 * Makes sure there is a WIFI connection and waits until it is (re-)established.
 */
void ensureWIFIConnection() {
  if ((WiFiMulti.run() != WL_CONNECTED)) {
    WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
    while ((WiFiMulti.run() != WL_CONNECTED)) {
      delay(100);
    }
  }
}
