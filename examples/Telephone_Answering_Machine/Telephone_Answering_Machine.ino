/**
 * Telephone_Answering_Machine.ino
 * 
 * Simple ON/OFF switch for the Telephone Answering Machine (TAM).
 * Only works on M5StickC.
 *
 * https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AHA-HTTP-Interface.pdf
 * https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/x_tam.pdf
 *
 *  Created on: 11.06.2021
 *    Latest update: 11.01.2023
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
#elif defined(M5Stick_C)
  #include <M5StickC.h>
#endif

#include <tr064.h>

//-------------------------------------------------------------------------------------
// Settings
//-------------------------------------------------------------------------------------
const unsigned long UPDATE_INTERVAL = 10000UL; //Check TAM switch every 10 seconds

//-------------------------------------------------------------------------------------
// Initializations. No need to change these.
//-------------------------------------------------------------------------------------
// Time when the last check was performed
unsigned long lastConnection = 0;

// TR-064 connection
TR064 connection(TR_PORT, TR_IP, TR_USER, TR_PASS);
//-------------------------------------------------------------------------------------

//###########################################################################################
//############################ OKAY, LET'S DO THIS! #########################################
//###########################################################################################

void setup() {
  M5.begin();
  pinMode(M5_BUTTON_HOME, INPUT);

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
  
  if (Serial) Serial.print("ESP Board MAC Address: ");
  if (Serial) Serial.println(WiFi.macAddress());
  M5.Lcd.fillScreen(WHITE);

  M5.Lcd.setRotation(3);
  M5.Lcd.setCursor(2, 10);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.print(WiFi.macAddress());
  M5.Lcd.setTextSize(12);

  // Connect to wifi
  ensureWIFIConnection();
  if (Serial) Serial.println("WiFi connection established");
  M5.Lcd.fillScreen(ORANGE);
  M5.Lcd.setCursor(15, 15);
  M5.Lcd.print("---");
  delay(100);
  
  
  // Set debug level. Available levels are:
  //  DEBUG_NONE   ///< Print no debug messages whatsoever (production)
  //  DEBUG_ERROR  ///< Only print error messages
  //  DEBUG_WARNING  ///< Only print error and warning messages
  //  DEBUG_INFO   ///< Print error, warning and info messages
  //  DEBUG_VERBOSE  ///< Print all messages
  if (Serial) Serial.println("Initialize TR-064 connection");
  connection.debug_level = connection.DEBUG_WARNING;
  if(Serial) Serial.setDebugOutput(true);
}

void loop() {
  M5.update();
  yield();
  ensureWIFIConnection();
  unsigned long int now = millis();
  if ( (now - lastConnection) > UPDATE_INTERVAL ) {
    if (Serial) Serial.print("Routine TAM check: ");
    if (Serial) Serial.println(refreshScreen(getABStatus() == 1) ? "ON" : "OFF");
    lastConnection = millis();
  }
  if (M5.BtnA.wasPressed()) {
    ButtonACallback();
  }
}

int getABStatus() {
  String params[][2] = {"NewIndex", "0"};
  int a = 1;
  String req[][2] = {"NewEnable", "0"};
  int b = 1;
  connection.action("urn:dslforum-org:service:X_AVM-DE_TAM:1", "GetInfo", params, a, req, b);
  return req[0][1].toInt();
}

bool switchABStatus() {
  int currentValue = getABStatus();
  int newValue = (currentValue == 1) ? 0 : 1;
  String params[][2] = {{"NewIndex", "0"}, {"NewEnable", String(newValue)}};
  int a = 2;
  connection.action("urn:dslforum-org:service:X_AVM-DE_TAM:1", "SetEnable", params, a);
  return (getABStatus() == newValue) ? true : false;
}

void ButtonACallback() {
  if (Serial) Serial.print("Switch TAM Status: ");
  if (Serial) Serial.println((switchABStatus()) ? "success" : "fail");
  refreshScreen(getABStatus());
  return;
}

int refreshScreen(int value) {
  if (value == 1) {
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.fillScreen(TFT_DARKGREEN);
    M5.Lcd.setCursor(15, 15);
    M5.Lcd.print("ON");
  }
  else {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.fillScreen(TFT_RED);
    M5.Lcd.setCursor(15, 15);
    M5.Lcd.print("OFF");
  }
  return value;
}


/**
 * Makes sure there is a WIFI connection and waits until it is (re-)established.
 */
void ensureWIFIConnection() {
	if ((WiFiMulti.run() != WL_CONNECTED)) {
		WiFiMulti.setAutoConnect(true);
		WiFiMulti.setAutoReconnect(true);
		WiFiMulti.softAPdisconnect(true);

		WiFiMulti.addAP(wifi_ssid, wifi_password);
		
		WiFiMulti.persistent(true);
		while ((WiFiMulti.run() != WL_CONNECTED)) {
			M5.Lcd.fillScreen(ORANGE);
			M5.Lcd.setTextColor(WHITE);
			M5.Lcd.setCursor(15, 15);
			M5.Lcd.print("ERR");
			delay(100);
		}
	}
}
