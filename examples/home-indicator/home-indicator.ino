#include "arduino_secrets.h"
/**
 * home-indicator.ino
 *  by René Vollmer
 *  Example code for the home-indicator-project [ https://www.instructables.com/id/Who-Is-Home-Indicator-aka-Weasley-Clock-Based-on-T ].
 *  
 *  Please adjust your data below.
 *  
 *  Created on: 09.12.2015,
 *  latest update: 24.06.2021
 *
 */

 
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
// Put your router settings here
//-------------------------------------------------------------------------------------

///////please enter your sensitive data in the Secret tab/arduino_secrets.h 

char wifi_ssid[] = SECRET_WIFI_SSID;
char wifi_password[] = SECRET_WIFI_PASSWORD;

char fuser[] = SECRET_FUSER;
char fpass[] = SECRET_FPASS;

char IP[] = SECRET_IP;
int PORT = 49000;

//-------------------------------------------------------------------------------------
// Put the settings for the devices to detect here
//-------------------------------------------------------------------------------------

// The number of different people/user you want to be able to detect
const int numUser = 3;

// The maximum amount of devices per user
const int maxDevices = 3;
/*
 * The array of macs. Structure:
 * = {{"mac1:user1", "mac2:user1", ..., "macX:user1"}, {"mac1:user2", "mac2:user2", ..., "macX:user2"}, ..., {"mac1:userY", "mac2:userY", ..., "macX:userY"}};
 * Doesn't matter if upper or lowercase :)
 */
const char macsPerUser[numUser][maxDevices][18] =
    { {"01:23:45:67:89:AB","12:34:56:78:9A:BC"}, //User1, two devices
      {"23:45:67:89:AB:CD"}, //User2, one device
      {"34:56:78:9A:BC:DE", "45:67:89:AB:CD:EF", "56:78:9A:BC:DE:F0"}}; //User3, three devices

//-------------------------------------------------------------------------------------
// Hardware settings
//-------------------------------------------------------------------------------------
/*
 * The pins for the LED output the users
 * Look these pins up. They might depend on your board.
 * Default is {5, 4, 0, 2}, which are the D1, D2, D3, D4 (in that order) pins of the MCU ESP8266 board.
 * (Adjust this to the amount of users you have :))
 */
int userPins[numUser] = {5, 4, 0}; //Three LED's because there are three users

//-------------------------------------------------------------------------------------
// Initializations. No need to change these.
//-------------------------------------------------------------------------------------

// TR-064 connection
TR064 connection(PORT, IP, fuser, fpass);


// Status array. 
bool onlineUsers[numUser];

// Array-settings.
const String STATUS_MAC = "MAC";
const String STATUS_IP = "IP";
const String STATUS_ACTIVE = "ACTIVE";
const String STATUS_HOSTNAME = "HOSTNAME";
const int STATUS_MAC_INDEX = 0;
const int STATUS_IP_INDEX = 1;
const int STATUS_ACTIVE_INDEX = 3;
const int STATUS_HOSTNAME_INDEX = 2;
//-------------------------------------------------------------------------------------

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

	//Define the pins for the LEDs as outputs.
	for (int i=0;i<numUser;i++) {
		pinMode(userPins[i], OUTPUT);
	}

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

	// Request the number of (connected) Wifi-Devices
	int numDev = getWifiNumber();
	if(Serial) Serial.printf("WIFI has %d (connected) devices.\n", numDev);

	// Check the status of all the devices connected to wifi
	getStatusOfAllWifi(numDev);

	// Get the number of all devices, that are known to this router
	numDev = getDeviceNumber();
	if (Serial) Serial.printf("Router has %d known devices.\n", numDev);
}

void loop() {
	ensureWIFIConnection();
  
	// For the next round, assume all users are offline
	for (int i=0;i<numUser;++i) {
		onlineUsers[i] = false;
	}

	// Check for all users if at least one of the macs is online
	for (int i=0;i<numUser;++i) {
		if (Serial) Serial.printf("> USER %d -------------------------------\n",i);
		boolean b = true; //No online device found yet
		// Check all devices
		for (int j=0;j<maxDevices && b;++j) {
		// Get the mac of the device to check
		String curMac = macsPerUser[i][j];
		b = (curMac!=""); //If it is empty, we don't need to check it (or the next one)
		if (b) {
			// okay, ask the router for the status of this MAC
			String stat2[4][2];
			getStatusOfMAC(curMac, stat2);

			// aaaaaaaaaaaannd??? Is it online?
			if (stat2[STATUS_ACTIVE_INDEX][1] != "" && stat2[STATUS_ACTIVE_INDEX][1] != "0") {
			onlineUsers[i] = true;
			b=true;
			}
			// Okay, print the status to the console!
			verboseStatus(stat2);
		}
		}
	}
	if(Serial) Serial.println("-------------------------------------------");

	// Flash all LEDs and then set them to the status we just found
	for (int i=0;i<numUser;++i) {
		digitalWrite(userPins[i], HIGH);
		delay(7);
		digitalWrite(userPins[i], LOW);
		delay(7);
		if (onlineUsers[i]) {
		digitalWrite(userPins[i], HIGH);
		} else {
		digitalWrite(userPins[i], LOW);
		}
	}
	delay(1000);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// helper methods//////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

/** 
 *  Get the number of devices that were connected to the WIFI lastly
 *  (some of them might not be online anymore, you need to check them individually!)
 *  return (int)
 */
int getWifiNumber() {
	String params[][2] = {{}};
	String req[][2] = {{"NewTotalAssociations", ""}};
	connection.action("WLANConfiguration:1", "GetTotalAssociations", params, 0, req, 1);
	int numDev = (req[0][1]).toInt();
	return numDev;
}

/** Print the status of all devices that were connected to the WIFI lastly
*  (some of them might not be online anymore, also gets you the hostnames and macs)
*  return nothing as of yet
 */
void getStatusOfAllWifi() {
  	getStatusOfAllWifi(getWifiNumber());
}


/** 
 *  Print the status of all devices that were connected to the WIFI lastly
 * (some of them might not be online anymore, also gets you the hostnames and macs)
 * return nothing as of yet
 */
void getStatusOfAllWifi(int numDev) {
	//Query the mac and status of each device
	for (int i=0;i<numDev;++i) {
		String params[][2] = {{"NewAssociatedDeviceIndex", String(i)}};
		String req[][2] = {{"NewAssociatedDeviceAuthState", ""}, {"NewAssociatedDeviceMACAddress", ""}, {"NewAssociatedDeviceIPAddress", ""}};
		if(connection.action("WLANConfiguration:1", "GetGenericAssociatedDeviceInfo", params, 1, req, 2)){
		if(Serial) {
			Serial.printf("%d:\t", i);
			Serial.println((req[1][1])+" is online "+(req[0][1]));
			Serial.flush();
		}
		}else{
			if(Serial) {
        Serial.printf("Fehler");        
				Serial.flush();
			}
		}
	}
}

/** 
 *  Get the status of one very specific device. Only works if it is a WIFI device!
 * return nothing, but fills the array r
 */
void getStatusOfMACwifi(String mac, String (&r)[4][2]) {
	// Ask for one specific device
	mac.toUpperCase();
	String params[][2] = {{"NewAssociatedDeviceMACAddress", mac}};
	String req[][2] = {{"NewAssociatedDeviceIPAddress", ""}, {"NewAssociatedDeviceAuthState", ""}};
	if(connection.action("WLANConfiguration:1", "GetSpecificAssociatedDeviceInfo", params, 1, req, 2)){
		if(Serial) {
			Serial.println(mac + " is online " + (req[2][1]));
			Serial.flush();
		}
		r[STATUS_MAC_INDEX][0] = STATUS_MAC;
		r[STATUS_MAC_INDEX][1] = mac;
		r[STATUS_IP_INDEX][0] = STATUS_IP;
		r[STATUS_IP_INDEX][1] = req[0][1];
		r[STATUS_HOSTNAME_INDEX][0] = STATUS_HOSTNAME;
		r[STATUS_HOSTNAME_INDEX][1] = "";
		r[STATUS_ACTIVE_INDEX][0] = STATUS_ACTIVE;
		r[STATUS_ACTIVE_INDEX][1] = req[1][1];
	}else{
		if(Serial) {
		Serial.println(mac + " Fehler");
		Serial.flush();
		}
	}
}

/** 
 *  Get the number of devices that were connected to the router (ever)
 *  (some of them might not be online, you need to check them individually!)
 *  return (int)
 */
int getDeviceNumber() {
	String params[][2] = {{}};
	String req[][2] = {{"NewHostNumberOfEntries", ""}};
	connection.action("Hosts:1", "GetHostNumberOfEntries", params, 0, req, 1);
	int numDev = (req[0][1]).toInt();
	return numDev;
}

/** 
 *  Get the status of one very specific device. May contain less information as the same option for WIFI.
 * return nothing, but fills the array r
 */
void getStatusOfMAC(String mac, String (&r)[4][2]) {
	//Ask for one specific device
	String params[][2] = {{"NewMACAddress", mac}};
	String req[][2] = {{"NewIPAddress", ""}, {"NewActive", ""}, {"NewHostName", ""}};
	if(connection.action("Hosts:1", "GetSpecificHostEntry", params, 1, req, 2)){
		if(Serial) {
			Serial.println(mac + " is online " + (req[1][1]));
			Serial.flush();
		}
		r[STATUS_MAC_INDEX][0] = STATUS_MAC;
		r[STATUS_MAC_INDEX][1] = mac;
		r[STATUS_IP_INDEX][0] = STATUS_IP;
		r[STATUS_IP_INDEX][1] = req[0][1];
		r[STATUS_HOSTNAME_INDEX][0] = STATUS_HOSTNAME;
		r[STATUS_HOSTNAME_INDEX][1] = req[2][1];
		r[STATUS_ACTIVE_INDEX][0] = STATUS_ACTIVE;
		r[STATUS_ACTIVE_INDEX][1] = req[1][1];
	}else{
		if(Serial) {
		Serial.println(mac + " Fehler");
		Serial.flush();
		}
	}
}


/**
 * Prints the status of the device on the screen (arrays r of the getStatusOfMAC methods).
 * return nothing
 */
void verboseStatus(String r[4][2]) {
	for (int i=0;i<4;++i) {
		if(Serial) Serial.print(r[i][0]+"="+r[i][1]+", ");
	}
	if(Serial) Serial.print("\n");
}

/**
 * Makes sure there is a WIFI connection and waits until it is (re-)established.
 */
void ensureWIFIConnection() {
	if ((WiFiMulti.run() != WL_CONNECTED)) {
		WiFiMulti.addAP(wifi_ssid, wifi_password);
		WiFiMulti.run();
		while ((WiFiMulti.run() != WL_CONNECTED)) {
		//Flash all LED's to indicate, that the connection was lost.
		for (int i = 0; i < numUser; ++i) {
			digitalWrite(userPins[i], HIGH);
			delay(7);
			digitalWrite(userPins[i], LOW);
			delay(7);
		}
			delay(500);
		}
	}
}
