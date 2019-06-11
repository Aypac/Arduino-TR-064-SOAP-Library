/**
 * LaundryNotifier.ino
 *
 * ====================================================================
 *  Copyright (c) 2019 Thorsten Godau (dl9sec). All rights reserved.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *  3. Neither the name of the author(s) nor the names of any contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
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

//#define DEBUGLOG

#define LED_ESP12E    2
#define LED_NODEMCU   16

//------------------------------------------------
// Configuration with static IP
//------------------------------------------------

// WiFi parameters
const char* WiFiSSID    = "myWiFiSSID";         // WiFi SSID
const char* WiFiPSK     = "myWiFiPreSharedKey"; // WiFi WPA2 preshared key

const char *WiFiIP      = "192.168.2.69";     // WiFi IP of the ESP
const char *WiFiGW      = "192.168.2.1";      // WiFi GW
const char *WiFiNM      = "255.255.255.0";    // WiFi NM
const char *WiFiDNS     = "192.168.2.1";      // WiFi DNS

// Sip parameters
const char *SipIP       = "192.168.2.1";    // IP of the FRITZ!Box
const int   SipPORT     = 5060;             // SIP port of the FRITZ!Box
const char *SipUSER     = "SIP-User";       // SIP-Call username at the FRITZ!Box
const char *SipPW       = "SIP-Password";   // SIP-Call password at the FRITZ!Box

// Dial parameters
const char *SipDIAL     = "**9";                    // Dial number
const char *SipTEXT_1   = "Washing machine ready";  // Dial text 1 for washing machine
const char *SipTEXT_2   = "Dryer ready";            // Dial text 2 for dryer

// FRITZ!Box parameters
const char *FbApiUSER   = "laundry";                // FRITZ!Box API user
const char *FbApiPW     = "mypsaaword";             // FRITZ!Box API password
const char *FbApiIP     = SipIP;                    // FRITZ!Box IP
const int   FbApiPORT   = 49000;                    // Fritz"Box API port

const char *FbApiAIN01  = "11657 1234567";          // AIN of the washing machine
const char *FbApiAIN02  = "11657 7654321";          // AIN of the dryer

//------------------------------------------------

char    acSipIn[2048];
char    acSipOut[2048];

float   afPwrAIN01[4] = { 0.0, 0.0, 0.0, 0.0 };
uint8_t u8IdxPwrAIN01 = 0;

float   afPwrAIN02[4] = { 0.0, 0.0, 0.0, 0.0 };
uint8_t u8IdxPwrAIN02 = 0;

uint8_t u8StateAIN01 = 0;       // 0: idle, 1: in progress
uint8_t u8StateAIN02 = 0;       // 0: idle, 1: in progress

uint32_t u32Interval = 30000;   // Every 30s

uint32_t u32MillisTmp;

TR064 tr064conn(FbApiPORT, FbApiIP, FbApiUSER, FbApiPW);
Sip   aSip(acSipOut, sizeof(acSipOut));


void setup()
{
  int i = 0;
  
  IPAddress myIP;
  IPAddress myGW;
  IPAddress myNM;
  IPAddress myDNS;
  
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);

  Serial.begin(115200);
  Serial.setDebugOutput(false);
  delay(10);

  pinMode(LED_ESP12E, OUTPUT);
  pinMode(LED_NODEMCU, OUTPUT);
  
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);

  digitalWrite(LED_ESP12E, 1);  // LED off
  digitalWrite(LED_NODEMCU, 1); // LED off

  Serial.printf("\r\n\r\nConnecting to %s\r\n", WiFiSSID);

  WiFi.setAutoConnect (true);
  WiFi.setAutoReconnect (true);
  WiFi.softAPdisconnect (true);

  myIP.fromString(WiFiIP);
  myGW.fromString(WiFiGW);
  myNM.fromString(WiFiNM);
  myDNS.fromString(WiFiDNS);

  WiFi.config(myIP, myGW, myNM, myDNS);

  if ( String(WiFiSSID) != WiFi.SSID() )
  {
    Serial.print("Wifi initializing...\r\n");
    WiFi.begin(WiFiSSID, WiFiPSK);
  }

  while ( WiFi.status() != WL_CONNECTED )
  {
    delay(500);
    Serial.print(".");
  }
  
  WiFi.persistent(true);
  Serial.printf("\r\nWiFi connected to: %s\r\n", WiFi.localIP().toString().c_str());
  digitalWrite(LED_ESP12E, 0);

  aSip.Init(SipIP, SipPORT, WiFiIP, SipPORT, SipUSER, SipPW, 38);

  // Initialize the TR-064 library
  Serial.printf("Initialize TR-064 connection...\r\n");
  tr064conn.init();

  u32MillisTmp = millis();
  
}


float getPwrAIN(const char *FbApiAIN)
{
  String params[][2] = {{"NewAIN", FbApiAIN}};
  String req[][2] = {{"NewMultimeterPower", ""}};

  tr064conn.action("urn:dslforum-org:service:X_AVM-DE_Homeauto:1", "GetSpecificDeviceInfos", params, 1, req, 1);

  float power = (float)((req[0][1]).toInt()) / 100.0;
  
  return power;
}


void loop(void)
{
  float fAvgPwrAIN01;
  float fAvgPwrAIN02;

  uint8_t u8count;

  // SIP processing
  aSip.Processing(acSipIn, sizeof(acSipIn));

  // Smartplug processing
  if ( ( millis() - u32MillisTmp ) > u32Interval  )
  {
    
    afPwrAIN01[u8IdxPwrAIN01] = getPwrAIN(FbApiAIN01);
    
    if ( u8StateAIN01 == 0 && afPwrAIN01[u8IdxPwrAIN01] > 5.0 )
    {
      u8StateAIN01 = 1;  // Idle -> In Progress
    }
    
    if ( ++u8IdxPwrAIN01 > 3 ) u8IdxPwrAIN01 = 0;

    delay(100);
  
    afPwrAIN02[u8IdxPwrAIN02] = getPwrAIN(FbApiAIN02);
    
    if ( u8StateAIN02 == 0 && afPwrAIN02[u8IdxPwrAIN02] > 5.0 )
    {
      u8StateAIN02 = 1;  // Idle -> In Progress
    }
    
    if ( ++u8IdxPwrAIN02 > 3 ) u8IdxPwrAIN02 = 0;
  
    // Accumulate arrays and average
    fAvgPwrAIN01 = 0;
    fAvgPwrAIN02 = 0;
    for ( u8count = 0; u8count < 4; u8count++ )
    {
      fAvgPwrAIN01 += afPwrAIN01[u8count];
      fAvgPwrAIN02 += afPwrAIN02[u8count];
    }
    
    fAvgPwrAIN01 = fAvgPwrAIN01 / 4.0;
    fAvgPwrAIN02 = fAvgPwrAIN02 / 4.0;

    Serial.printf("AIN01: %.2f W (avg.), State; %d\r\n", fAvgPwrAIN01, u8StateAIN01);
    Serial.printf("AIN02: %.2f W (avg.), State; %d\r\n", fAvgPwrAIN02, u8StateAIN02);
    
    if ( u8StateAIN01 == 1 && fAvgPwrAIN01 <= 1.0 )
    {
      aSip.Dial(SipDIAL, SipTEXT_1);
      u8StateAIN01 = 0;  // Ready -> Idle
    }

    if ( u8StateAIN02 == 1 && fAvgPwrAIN02 <= 1.0 )
    {
      aSip.Dial(SipDIAL, SipTEXT_2);
      u8StateAIN02 = 0;  // Ready -> Idle
    }

    u32MillisTmp = millis();
  }

  ESP.wdtFeed();
}