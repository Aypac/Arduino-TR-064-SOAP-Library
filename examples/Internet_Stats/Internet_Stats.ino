/**
 * Internet_Stats.ino
 *  by René Vollmer
 *  
 * Example code for polling internet stats, like the currently used and available speeds.
 * 
 * Please adjust your sensitive data in the file/tab `arduino_secrets.h`
 *  
 * Created on: 11.01.2023
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
#endif

#include <tr064.h>

//-------------------------------------------------------------------------------------
// Initializations. No need to change these.
//-------------------------------------------------------------------------------------

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
  
  // Wait a few secs for warm-up (dunno why, was in the default code for http connections).
  delay(3000);

  // Connect to wifi
  ensureWIFIConnection();
  
  // Set debug level. Available levels are:
  //  DEBUG_NONE         ///< Print no debug messages whatsoever (production)
  //  DEBUG_ERROR        ///< Only print error messages
  //  DEBUG_WARNING      ///< Only print error and warning messages
  //  DEBUG_INFO         ///< Print error, warning and info messages
  //  DEBUG_VERBOSE      ///< Print all messages
  connection.debug_level = connection.DEBUG_VERBOSE;
  if(Serial) Serial.setDebugOutput(true);
  
  // The following line retrieves a list of all available services on the router.
  // It is not required for operation, so it can be safely commented and save
  //   ressources on the microcontroller. However, it can be helpful for debugging
  //     and development to keep it activated.
  if(Serial) Serial.printf("Initialize TR-064 connection\n\n");
  connection.init();
}

void loop() {
    ensureWIFIConnection();

    if(Serial) Serial.println("#################################################################################################################");
    int uptime = getUptime();
    delay(100);
    if(Serial) Serial.println("\n\rUptime: " + String(uptime) + "seconds");
    if(Serial) Serial.println("Uptime: " + String(int(uptime/3600))+ ':' + String(int((uptime%3600)/60))+ ':' + String((uptime%60)) + "(H:m:s) \n\r");

    String conStat = getConnectionStatus();
    delay(100);
    if(Serial) Serial.println("\n\rConnection status: " + conStat + "\n\r");
    if (conStat != "Disconnected") {
      String extIP = getExtIP();
      delay(100);
      if(Serial) Serial.println("\n\rExternal IP: " + extIP + "\n\r");
    }
    int nDev = getDeviceNumber();
    delay(100);
    if(Serial) Serial.println("\n\rNumber of connected devices: " + String(nDev) + "\n\r");

    int umr = 0;
    int dmr = 0;
    getMaxRates(umr, dmr);
    delay(100);
    if(Serial) Serial.println("\n\rMaximum up/download speed: " + String(umr) + "/" + String(dmr) + "\n\r");
    
    float ucr = 0;
    float dcr = 0;
    getCurRates(ucr, dcr);
    delay(100);
    if(Serial) Serial.println("\n\rCurrent up/download speed: " + String(ucr) + "/" + String(dcr));
    if(Serial) Serial.println("Load up/down: " + String(100*ucr/umr) + "%/" + String(100*dcr/dmr) + "%\n\r");
    if(Serial) Serial.println("#################################################################################################################");

    delay(10000);
	
//###########################################################################################
//############################          TODO        #########################################
//###########################################################################################
}

String getExtIP() {
  String params[][2] = {{}};
  String req[][2] = {{"NewExternalIPAddress", ""}};
  if (connection.action("urn:dslforum-org:service:WANIPConnection:1", "GetExternalIPAddress", params, 0, req, 1)) {
    return req[0][1];
  }
  return "unknown";
}

int getUptime() {
  String params[][2] = {{}};
  String req[][2] = {{"NewUptime", ""}};
  if (connection.action("urn:dslforum-org:service:WANIPConnection:1", "GetStatusInfo", params, 0, req, 1)) {
    return (req[0][1]).toInt();
  }
  return -1;
}

String getConnectionStatus() {
  String params[][2] = {{}};
  String req[][2] = {{"NewConnectionStatus", ""}};
  if (connection.action("urn:dslforum-org:service:WANIPConnection:1", "GetStatusInfo", params, 0, req, 1)) {
    return req[0][1];
  }
  return "unknown";
}

int getDeviceNumber() {
  String params[][2] = {{}};
  String req[][2] = {{"NewHostNumberOfEntries", ""}};
  if (connection.action("urn:dslforum-org:service:Hosts:1", "GetHostNumberOfEntries", params, 0, req, 1)) {
    return (req[0][1]).toInt();
  }
  return -1;
}

float factor1 = 1024;
float factor2 = 1048576;

void getMaxRates(int& u, int& d) {
  String params[][2] = {{}};
  String req[][2] = {{"NewLayer1UpstreamMaxBitRate", ""}, {"NewLayer1DownstreamMaxBitRate", ""}};
  if (connection.action("urn:dslforum-org:service:WANCommonInterfaceConfig:1", "GetCommonLinkProperties", params, 0, req, 2)) {
    long num = req[1][1].toInt();
    d = int(num*8 / factor1);
    num = req[0][1].toInt();
    u = int(num*8 / factor1);
  }
}

float numlistFormat(String l) {
  int i = 1;
  int n = 0;
  while (l.indexOf(',') > 0) {
    n += (l.substring(0, l.indexOf(','))).toInt();
    l = l.substring(l.indexOf(',')+1);
    ++i;
  }
  n += l.toInt();
  return n/i;
}

void getCurRates(float& u, float& d) {
  String params[][2] = {{"NewSyncGroupIndex", "0"}};
  String req[][2] = {{"Newus_current_bps", ""}, {"Newds_current_bps", ""}};
  if (connection.action("urn:dslforum-org:service:WANCommonInterfaceConfig:1", "X_AVM-DE_GetOnlineMonitor", params, 1, req, 2)) {
    String list = req[0][1];
    Serial.println(list + "=>" + String(numlistFormat(list)));
    u = (numlistFormat(list) / factor1);
    list = req[1][1];
    d = (numlistFormat(list) / factor1);
  }
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
