/**
 * caller.ino
 *  Oliver-André Urban
 *   based on
 *    home-indicator.ino
 *      by René Vollmer
 *   improved by
 *    Karsten Sauer (gsg2820)
 * 
 *  Example code for placing internal DECT phone calls.
 * 
 *  Please adjust your data below.
 *  
 *  created on: 07.06.2017
 *  latest update: 18.06.2021
 */

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
// Router settings
//-------------------------------------------------------------------------------------

// Wifi network name (SSID)
const char* wifi_ssid = "WLANSID"; 

// Wifi network password
const char* wifi_password = "XXXXXXXXXXXXXXXXXXXXX";

// The username if you created an account, "admin" otherwise
const char* fuser = "homechecker";

// The password for the aforementioned account.
const char* fpass = "this_shouldBEaDecentPassword!";

// IP address of your router. This should be "192.168.179.1" for most FRITZ!Boxes
const char* IP = "192.168.179.1";

// Port of the API of your router. This should be 49000 for all TR-064 devices.
const int PORT = 49000;

//-------------------------------------------------------------------------------------
// Hardware settings
//-------------------------------------------------------------------------------------
// Flash BUTTON - you can connect a separate button to this pin or an opto-coupler 
// for example: use a resistor and an opto-coupler to connect to a doorbell
#define BUTTON 0

//-------------------------------------------------------------------------------------
// Initializations. No need to change these.
//-------------------------------------------------------------------------------------

// TR-064 connection
TR064 connection(PORT, IP, fuser, fpass);

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
	pinMode(BUTTON, INPUT);
	
	// **************************************************
	// Wait a few secs for warm-up (dunno why, was in the default code for http connections).
	// You might be able to remove this block
	for(uint8_t t = 4; t > 0; t--) {
	if(Serial) Serial.printf("[SETUP] WAIT %d...\n", t);
		for (int i=0;i<numUser;++i) {
			digitalWrite(userPins[i], HIGH);
		}
		delay(300);
		for (int i=0;i<numUser;++i) {
			digitalWrite(userPins[i], LOW);
		}
		delay(700);
		if(Serial) Serial.flush();
	}
	// **************************************************
	

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
	int button_state = digitalRead(BUTTON);
	if (digitalRead(BUTTON) == LOW) {
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
}

void callDect() {
	ensureWIFIConnection();

	String params[][2] = {{"NewAIN", "12345 0123456"}, {"NewSwitchState", "TOGGLE"}};
	connection.action("X_AVM-DE_Homeauto:1", "SetSwitch", params, 2);
	// connection.action("urn:dslforum-org:service:X_AVM-DE_Homeauto:1", "SetSwitch", params, 2);
}

String getStatus() {
	ensureWIFIConnection();
	
	String paramsb[][2] = {{"NewAIN", "12345 0123456"}};
	String reqb[][2] = {{"NewDeviceId", ""}, {"NewSwitchState", ""}};
	connection.action("X_AVM-DE_Homeauto:1", "GetSpecificDeviceInfos", paramsb, 1, reqb, 2);
	//connection.action("urn:dslforum-org:service:X_AVM-DE_Homeauto:1", "GetSpecificDeviceInfos", paramsb, 1, reqb, 2);
	return reqb[1][1];
}


/**
 * Makes sure there is a WIFI connection and waits until it is (re-)established.
 */
void ensureWIFIConnection() {
	if ((WiFiMulti.run() != WL_CONNECTED)) {
		WiFiMulti.addAP(wifi_ssid, wifi_password);
		while ((WiFiMulti.run() != WL_CONNECTED)) {
			delay(100);
		}
	}
}
