/**
 * home-indicator.ino
 *  René Vollmer
 *  Example code for the home-indicator-project [ https://www.instructables.com/id/Who-Is-Home-Indicator-aka-Weasley-Clock-Based-on-T ].
 *  
 * Please adjust your data below.
 *  
 * Created on: 09.12.2015,
 * Latest update: 11.06.2019
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

// Wifi network name (SSID)
const char *wifi_ssid = "WLANSID"; 

// Wifi network password
const char *wifi_password = "XXXXXXXXXXXXXXXXXXXXX";

// The username if you created an account, "admin" otherwise
const char *fuser = "homechecker";

// The password for the aforementioned account.
const char *fpass = "this_shouldBEaDecentPassword!";

// IP address of your router. This should be "192.168.179.1" for most FRITZ!Boxes
const char *IP = "192.168.179.1";

// Port of the API of your router. This should be 49000 for all TR-064 devices.
const int PORT = 49000;

// Put the settings for the devices to detect here
//   The number of different people/user you want to be able to detect
const int numUser = 3;

//   The maximum amount of devices per user
const int maxDevices = 3;

//-------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------

// TR-064 connection
TR064 connection(PORT, IP, fuser, fpass);


// Status array. No need to change this!
bool onlineUsers[numUser];

// Array-settings. No need to change these!



//###########################################################################################
//############################ OKAY, LET'S DO THIS! #########################################
//###########################################################################################

void setup() {
    // You might want to change the baud-rate
    Serial.begin(115200);
    if(Serial) Serial.setDebugOutput(true);

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

    // Initialize the TR-064 library
    // (IMPORTANT!)
    if(Serial) Serial.printf("Initialize TR-064 connection\n\n");
    connection.debug_level = DEBUG_VERBOSE; //0: None, 1: Errors, 2: Warning, 3: Info, 4: Verbose
    connection.init();

    //
}

void loop() {
    ensureWIFIConnection();
  
    // Query up- and down-link speed and transfer rate
    // Query external IP address
    // Print results
    if(Serial) Serial.println("-------------------------------------------");

    delay(1000);
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
