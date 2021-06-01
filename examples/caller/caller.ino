/**
 * caller.ino
 *  Oliver-André Urban
 *   based on
 *    home-indicator.ino
 *      by René Vollmer
 * 
 *  Example code for placing internal DECT phone calls
 * 
 *  Please adjust your data below.
 *  
 *  created on: 07.06.2017
 *  latest update: 11.06.2019
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


// Flash BUTTON - you can connect a separate button to this pin or an opto-coupler 
// for example: use a resistor and an opto-coupler to connect to a doorbell
#define BUTTON 0


//-------------------------------------------------------------------------------------
// Put your router settings here
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

// -------------------------------------------------------------------------------------

// TR-064 connection
TR064 connection(PORT, IP, fuser, fpass);


void setup() {
	Serial.begin(115200);
	// Wait a few secs for warm-up (dunno why, was in the default code for http connections).
	delay(5000);

	// Connect to wifi
	ensureWIFIConnection();
	
	// Port as input
	pinMode(BUTTON, INPUT);
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
		delay(20000);
	} else {
		if (Serial) {
			Serial.println();
			Serial.printf("Button not pressed");
		}
		delay(50);
	}
}


void callWahlhilfe() {
	ensureWIFIConnection();

	String params[][2] = {{"NewX_AVM-DE_PhoneNumber", "**799"}};
	String req[][2] = {{}};

	connection.action("urn:dslforum-org:service:X_VoIP:1", "X_AVM-DE_DialNumber", params, 1, req, 0);
}

void callDect() {
	ensureWIFIConnection();

	String params[][2] = {{"NewAIN", "12345 0123456"}, {"NewSwitchState", "TOGGLE"}};
	connection.action("urn:dslforum-org:service:X_AVM-DE_Homeauto:1", "SetSwitch", params, 2);
}

String getStatus() {
	String paramsb[][2] = {{"NewAIN", "12345 0123456"}};
	String reqb[][2] = {{"NewDeviceId", ""}, {"NewSwitchState", ""}};
	connection.action("urn:dslforum-org:service:X_AVM-DE_Homeauto:1", "GetSpecificDeviceInfos", paramsb, 1, reqb, 2);
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