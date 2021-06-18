/**
 * LaundryNotifier.ino
 *
 * ====================================================================
 *  Copyright (c) 2019 Thorsten Godau (dl9sec). All rights reserved.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *  3. Neither the name of the author(s) nor the names of any contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *  ====================================================================
 * 	
 *  Example code for placing internal DECT phone calls based on the power
 *   consumption of a device (using a smart plug, such as DECT210).
 *  Can be used to notify a user when a washingmachine/dryer is done.
 *
 *  created on: 11.06.2019
 *  latest update: 11.06.2019
 *	 (Adapted from commit bd0cb80 of Thorsten Godau's repository)
 */

#include <ArduinoSIP.h>

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
const char *wifi_ssid = "WLANSID"; 

// Wifi network password
const char *wifi_password = "XXXXXXXXXXXXXXXXXXXXX";

// The username if you created an account, "admin" otherwise
const char* FbApiUSER = "homechecker";

// The password for the aforementioned account.
const char* FbApiPW = "this_shouldBEaDecentPassword!";

// IP address of your router. This should be "192.168.179.1" for most FRITZ!Boxes
const char* FbApiIP = "192.168.179.1";

// Port of the API of your router. This should be 49000 for all TR-064 devices.
const int FbApiPORT = 49000;


//-------------------------------------------------------------------------------------
// Telephone and power meter settings
//-------------------------------------------------------------------------------------
// Sip parameters
const char *SipIP   = "192.168.2.1";  // IP of the FRITZ!Box
const int   SipPORT   = 5060;         // SIP port of the FRITZ!Box
const char *SipUSER   = "SIP-User";   // SIP-Call username at the FRITZ!Box
const char *SipPW   = "SIP-Password"; // SIP-Call password at the FRITZ!Box

// Dial parameters
const char *SipDIAL   = "**9";        // Dial number
const char *SipTEXT_1   = "Washing machine ready";  // Dial text 1 for washing machine
const char *SipTEXT_2   = "Dryer ready";    // Dial text 2 for dryer

const char *FbApiAIN01  = "11657 1234567";    // AIN of the washing machine
const char *FbApiAIN02  = "11657 7654321";    // AIN of the dryer

//-------------------------------------------------------------------------------------
// Hardware settings
//-------------------------------------------------------------------------------------
#define LED_ESP12E  2
#define LED_NODEMCU   16

//-------------------------------------------------------------------------------------
// Initializations. No need to change these.
//-------------------------------------------------------------------------------------
char  acSipIn[2048];
char  acSipOut[2048];

float   afPwrAIN01[4] = { 0.0, 0.0, 0.0, 0.0 };
uint8_t u8IdxPwrAIN01 = 0;

float   afPwrAIN02[4] = { 0.0, 0.0, 0.0, 0.0 };
uint8_t u8IdxPwrAIN02 = 0;

uint8_t u8StateAIN01 = 0;   // 0: idle, 1: in progress
uint8_t u8StateAIN02 = 0;   // 0: idle, 1: in progress

uint32_t u32Interval = 30000;   // Every 30s

uint32_t u32MillisTmp;


TR064 tr064conn(FbApiPORT, FbApiIP, FbApiUSER, FbApiPW);
Sip   aSip(acSipOut, sizeof(acSipOut));

//------------------------------------------------

//###########################################################################################
//############################ OKAY, LET'S DO THIS! #########################################
//###########################################################################################

void setup() {
  // See more https://techtutorialsx.com/2017/01/21/esp8266-watchdog-functions/
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);

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
  
  // Set the LED output pins
  pinMode(LED_ESP12E, OUTPUT);
  pinMode(LED_NODEMCU, OUTPUT);

  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  
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

  digitalWrite(LED_ESP12E, 1);  // LED off
  digitalWrite(LED_NODEMCU, 1); // LED off


  aSip.Init(SipIP, SipPORT, WiFiIP, SipPORT, SipUSER, SipPW, 38);
  
  // The following line retrieves a list of all available services on the router.
  // It is not required for operation, so it can be safely commented and save
  //   ressources on the microcontroller. However, it can be helpful for debugging
  //   and development to keep it activated.
  if(Serial) Serial.printf("Initialize TR-064 connection\n\n");
  tr064conn.init();

  u32MillisTmp = millis();
}


void loop(void) {
  float fAvgPwrAIN01;
  float fAvgPwrAIN02;

  uint8_t u8count;

  // SIP processing
  aSip.Processing(acSipIn, sizeof(acSipIn));

  // Smartplug processing
  if (( millis() - u32MillisTmp ) > u32Interval) {

    afPwrAIN01[u8IdxPwrAIN01] = getPwrAIN(FbApiAIN01);

    if ( u8StateAIN01 == 0 && afPwrAIN01[u8IdxPwrAIN01] > 5.0 ) {
      u8StateAIN01 = 1;  // Idle -> In Progress
    }

    if ( ++u8IdxPwrAIN01 > 3 ) u8IdxPwrAIN01 = 0;

    delay(100);

    afPwrAIN02[u8IdxPwrAIN02] = getPwrAIN(FbApiAIN02);

    if ( u8StateAIN02 == 0 && afPwrAIN02[u8IdxPwrAIN02] > 5.0 ) {
      u8StateAIN02 = 1;  // Idle -> In Progress
    }

    if ( ++u8IdxPwrAIN02 > 3 ) u8IdxPwrAIN02 = 0;

    // Accumulate arrays and average
    fAvgPwrAIN01 = 0;
    fAvgPwrAIN02 = 0;
    for ( u8count = 0; u8count < 4; u8count++ ) {
      fAvgPwrAIN01 += afPwrAIN01[u8count];
      fAvgPwrAIN02 += afPwrAIN02[u8count];
    }

    fAvgPwrAIN01 = fAvgPwrAIN01 / 4.0;
    fAvgPwrAIN02 = fAvgPwrAIN02 / 4.0;

    Serial.printf("AIN01: %.2f W (avg.), State; %d\r\n", fAvgPwrAIN01, u8StateAIN01);
    Serial.printf("AIN02: %.2f W (avg.), State; %d\r\n", fAvgPwrAIN02, u8StateAIN02);

    if ( u8StateAIN01 == 1 && fAvgPwrAIN01 <= 1.0 ) {
      aSip.Dial(SipDIAL, SipTEXT_1);
      u8StateAIN01 = 0;  // Ready -> Idle
    }

    if ( u8StateAIN02 == 1 && fAvgPwrAIN02 <= 1.0 ) {
      aSip.Dial(SipDIAL, SipTEXT_2);
      u8StateAIN02 = 0;  // Ready -> Idle
    }

    u32MillisTmp = millis();
  }

  ESP.wdtFeed();
}



float getPwrAIN(const char *FbApiAIN) {
  String params[][2] = {{"NewAIN", FbApiAIN}};
  String req[][2] = {{"NewMultimeterPower", ""}};

  tr064conn.action("X_AVM-DE_Homeauto:1", "GetSpecificDeviceInfos", params, 1, req, 1);

  float power = (float)((req[0][1]).toInt()) / 100.0;

  return power;
}

/**
 * Makes sure there is a WIFI connection and waits until it is (re-)established.
 */
void ensureWIFIConnection() {
  if ((WiFiMulti.run() != WL_CONNECTED)) {
    WiFiMulti.addAP(wifi_ssid, wifi_password);
    //WiFiMulti.run();
    while ((WiFiMulti.run() != WL_CONNECTED)) {
      delay(100);
    }
  }
}
