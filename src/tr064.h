/*!
 * @file tr064.h
 * 
 * Library for communicating via TR-064 protocol (e.g. Fritz!Box)
 * This library allows for easy communication of TR-064 (and possibly TR-069) enabled devices,
 * such as Routers, smartplugs, DECT telephones etc.
 * Details, examples and the latest Version of this library can be found <a href='https://github.com/Aypac/Arduino-TR-064-SOAP-Library'>on my Github page</a>.
 * A descriptor of the protocol can be found <a href='https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AVM_TR-064_first_steps.pdf'>here</a>.
 * 
 * This library depends on:
 *    MD5Builder
 *  ESP8266HTTPClient or HTTPClient, depending on the intended platform (ESP8266 or ESP32).
 *
 * Written by René Vollmer "Aypac" in November 2016.
 *
 * MIT License, all text here must be included in any redistribution.
 *
 */



#ifndef tr064_h
#define tr064_h

#include "Arduino.h"
#include <MD5Builder.h>
#if defined(ESP8266)
    //if(Serial) Serial.println(F("Version compiled for ESP8266."));
    #include <ESP8266WiFi.h>
    #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
    //if(Serial) Serial.println(F("Version compiled for ESP32."));
    #include <WiFi.h>
    #include <HTTPClient.h>

#else
    //INCOMPATIBLE!
#endif

#define arr_len( x )  ( sizeof( x ) / sizeof( *x ) ) ///< Gives the length of an array

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds
#ifndef TR064_SOCKET_TIMEOUT
#define TR064_SOCKET_TIMEOUT 1500
#endif

// Different debug level
// #define DEBUG_NONE 0        ///< Print no debug messages whatsoever
// #define DEBUG_ERROR 1        ///< Only print error messages
// #define DEBUG_WARNING 2        ///< Only print error and warning messages
// #define DEBUG_INFO 3        ///< Print error, warning and info messages
// #define DEBUG_VERBOSE 4        ///< Print all messages

// Possible values for client.state()
#define TR064_NO_SERVICES           -1
#define TR064_SERVICES_LOADED       0


/**************************************************************************/
/*! 
    @brief Class to easily make TR-064 calls. This is the main class
             of this library. An object of this class keeps track of
             
*/
/**************************************************************************/

class TR064 {
    public:
        enum LoggingLevels {DEBUG_NONE, DEBUG_ERROR, DEBUG_WARNING, DEBUG_INFO, DEBUG_VERBOSE};
        
        TR064();
        TR064(uint16_t port, const String& ip, const String& user, const String& pass);
        ~TR064() {}
        TR064& setServer(uint16_t port, const String& ip, const String& user, const String& pass);
        void init();
        void initNonce();

        bool action(const String& service, const String& act);
        bool action(const String& service, const String& act, String params[][2], int nParam);
        bool action(const String& service, const String& act, String params[][2], int nParam, String (*req)[2], int nReq);
        
        String md5String(const String& s);
        String byte2hex(byte number);
        int debug_level; ///< Available levels are `DEBUG_NONE`, `DEBUG_ERROR`, `DEBUG_WARNING`, `DEBUG_INFO`, and `DEBUG_VERBOSE`.
        boolean connected();
        void disconnect(bool disconnect_package = false);
        int state();        
    private:
        WiFiClient tr064client;
        HTTPClient http;
        
        //TODO: More consistent naming.
        void initServiceURLs();
        void deb_print(const String& message, int level);
        void deb_println(const String& message, int level);
        bool action_raw(const String& service,const String& act, String params[][2], int nParam);
        void takeNonce();
        bool httpRequest(const String& url, const  String& xml, const  String& action);
        bool httpRequest(const String& url,  const String& xml, const  String& action, bool retry);
        String generateAuthToken();
        String generateAuthXML();
        String findServiceURL(const String& service);
        String clearOldServiceName(const String& service);
        bool xmlTakeParam(String& value, const String& needParam);
        void clear();
        int _state;
        String _ip;
        uint16_t _port;
        String _user;
        String _pass;
        String _realm; //To be requested from the router
        String _secretH; //to be generated
        String _nonce = "";

        const char* const _requestStart = "<?xml version=\"1.0\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">";
        const char* const _detectPage = "/tr64desc.xml";
        const char* const _servicePrefix = "urn:dslforum-org:service:";
        unsigned long lastOutActivity;
        unsigned long lastInActivity;
        /* TODO: We should give access to this data for users to inspect the
        * possibilities of their device(s) - see #9 on Github.
        TODO: Remove 100 services limits here
        */
        String _services[100][2];
};

#endif
