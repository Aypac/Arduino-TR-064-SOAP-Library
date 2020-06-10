/**
 * caller.ino
 *  René Vollmer, inspired by [this issue](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues/3)
 *  and adapted from [gickowtf](https://github.com/gickowtf)'s solution.
 * 
 *  Example code for switching WIFI interfaces on and off.
 * 
 *  Please adjust your data below.
 *  
 *  created on: 10.06.2020
 */

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

// Pin for the push button
const byte push_button = D3;

// Pin for the indicator LED
const byte led = D5;

// -------------------------------------------------------------------------------------

// Do not mess with these :)
// TR-064 connection
TR064 connection(PORT, IP, fuser, fpass);
bool flag, switch_state, state;


void setup() {
  Serial.begin(115200);
  // Wait a few secs for warm-up (dunno why, was in the default code for http connections).
  delay(5000);
 
  // Connect to wifi
  ensureWIFIConnection();
  
  pinMode(push_button, INPUT_PULLUP);
  pinMode(led, OUTPUT);
}

void loop() {
  // Detect push button press
  
  // Read push button state
  switch_state = !digitalRead(push_button);
  
  
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
  digitalWrite(led, state);
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
		WiFiMulti.addAP(wifi_ssid, wifi_password);
		while ((WiFiMulti.run() != WL_CONNECTED)) {
			delay(100);
		}
	}
}