/**
 * Wifi_Switch.ino
 *  by René Vollmer, inspired by [this issue](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues/3)
 *  and adapted from [gickowtf](https://github.com/gickowtf)'s solution.
 * 
 *  Example code for switching WIFI interfaces on and off.
 * 
 * Please adjust your sensitive data in the file/tab `arduino_secrets.h`
 *  and the settings below.
 *  
 * Created on: 10.06.2020
 *  Latest update: 11.01.2023
 */
#include "arduino_secrets.h"

#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WiFiMulti.h>
  #include <ESP8266HTTPClient.h>
  ESP8266WiFiMulti WiFiMulti;
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WiFiMulti.h>
  #include <HTTPClient.h>
  WiFiMulti WiFiMulti;
#endif

#include <tr064.h>


//-------------------------------------------------------------------------------------
// Hardware settings
//-------------------------------------------------------------------------------------
// Pin for the push button, e.g. D3
//  (depends on the microcontroller and circuit you use)
#define PUSH_BUTTON 0

// Pin for the indicator LED, e.g. D5
//  (depends on the microcontroller and circuit you use)
#define LED 1

//-------------------------------------------------------------------------------------
// Initializations. No need to change these.
//-------------------------------------------------------------------------------------
bool flag, switch_state, state;

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

// -------------------------------------------------------------------------------------


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
  //  DEBUG_NONE   ///< Print no debug messages whatsoever (production)
  //  DEBUG_ERROR  ///< Only print error messages
  //  DEBUG_WARNING  ///< Only print error and warning messages
  //  DEBUG_INFO   ///< Print error, warning and info messages
  //  DEBUG_VERBOSE  ///< Print all messages
  connection.debug_level = connection.DEBUG_WARNING;
  if(Serial) Serial.setDebugOutput(true);
  
  // Define button input and LED output
  pinMode(PUSH_BUTTON, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  
  // The following line retrieves a list of all available services on the router.
  // It is not required for operation, so it can be safely commented and save
  //   ressources on the microcontroller. However, it can be helpful for debugging
  //   and development to keep it activated.
  if(Serial) Serial.printf("Initialize TR-064 connection\n\n");
  connection.init();
}

void loop() {
  // Detect push button press
  
  // Read push button state
  switch_state = !digitalRead(PUSH_BUTTON);
  
  
  if (switch_state && !flag) {
    // Low-high transition detected
  
    // Debounce: make sure this is not detected again,
    // during the same button press.
    flag = true;
  
    // We detected a button press, toggle the state
    state = !state;
    
    // Set WIFI state accordingly
    switchGuestWifi(state);
  
    // debounce
    delay(50); 
  } else if (!switch_state) {
    // reset debounce flag...
    flag = false;
    // ...now we can detect a new button press.
    
    // debounce
    delay(50);
  }
  
  // Use LED to indicate WIFI status
  digitalWrite(LED, state);
}



void switchGuestWifi(bool status) {
  String params[][2] = {{"NewEnable", "0"}};
  if (status) {
    params[0][1] = "1";
    Serial.println ("Set status: on");
  } else {
    Serial.println ("Set status: off");
  }
  String req[][2] = {{}};
  // TODO: allow for other WIFI interfaces (such as "urn:dslforum-org:service:WLANConfiguration:2")
  connection.action("urn:dslforum-org:service:WLANConfiguration:3", "SetEnable", params, 1, req, 2);
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
