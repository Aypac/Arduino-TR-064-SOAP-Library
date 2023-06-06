/*!
 * @file tr064.h
 * 
 * Library for communicating via TR-064 protocol (e.g. FRITZ!Box).
 * This allows for easy communication with TR-064 (and possibly TR-069) enabled devices,
 * such as Routers, smartplugs, DECT telephones etc.
 * Details, examples and the latest version of this library can be found <a href='https://github.com/Aypac/Arduino-TR-064-SOAP-Library'>on my Github page</a>.
 * A descriptor of the protocol can be found <a href='https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AVM_TR-064_first_steps.pdf'>here</a>.
 * 
 * This library depends on:
 *   MD5Builder
 *   ESP8266HTTPClient or HTTPClient, depending on the intended platform (ESP8266 or ESP32).
 *
 * Written by René Vollmer "Aypac" in November 2016, with contributions from others. See <a href="https://github.com/Aypac/Arduino-TR-064-SOAP-Library">Github for details</a>..
 *
 * MIT License, all text above must be included in any redistribution.
 *
 */



#ifndef tr064_h
#define tr064_h

#include "Arduino.h"
#include <MD5Builder.h>
#include <map>
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

/// Types of http protocol
typedef enum {
      useHttp,
      useHttpsInsec,
      useHttps
  } Protocol;

/// X509Certificate
typedef const char* X509Certificate;



#define arr_len( x )  ( sizeof( x ) / sizeof( *x ) ) ///< Gives the length of an array


/**************************************************************************/
/*! 
    @brief Class to easily make TR-064 calls. This is the main class
             of this library. An object of this class keeps track of
             
*/
/**************************************************************************/

class TR064 {
    public:
        /*!  Different debug levels
         *   DEBUG_NONE         ///< Print no debug messages whatsoever
         *   DEBUG_ERROR        ///< Only print error messages
         *   DEBUG_WARNING      ///< Only print error and warning messages
         *   DEBUG_INFO         ///< Print error, warning and info messages
         *   DEBUG_VERBOSE      ///< Print all messages
         */
        enum LoggingLevels {
            DEBUG_NONE, ///< Print no debug messages whatsoever
            DEBUG_ERROR, ///< Only print error messages
            DEBUG_WARNING, ///< Only print error and warning messages
            DEBUG_INFO, ///< Print error, warning and info messages
            DEBUG_VERBOSE ///< Print all messages
        };
        
        /// Values of the _state variable
        enum InitializationLevels {
            TR064_STATUS_INIT, ///< The library has been instanciated without settings
            TR064_STATUS_SETTINGS, ///< Settings have been set
            TR064_STATUS_SERVICES ///< Service URLs have been initiated
        };
        
        /// HTTP codes see RFC7231
        enum tr064_codes{
            //TR064_CODE_UNAUTHORIZED = 401, ///< If a user is not authenticated, 401 (“Unauthorized”) will be returned.
            TR064_CODE_UNKNOWNACTION = 401, ///< If an unknown action is used the returned code is 401. This return code is used for obsoleted actions, too. 
            TR064_CODE_FALSEARGUMENTS = 402, ///< If a user is not authenticated, 401 (“Unauthorized”) will be returned.
            TR064_CODE_AUTHFAILED = 503, ///< If a user is not authenticated, 401 (“Unauthorized”) will be returned.
            TR064_CODE_ARGUMENTVALUEINVALIDE = 600, ///< Argument Value Invalid
            TR064_CODE_ACTIONNOTAUTHORIZED = 606, ///< If a user is authenticated but has not the needed rights, 606 (“Action not authorized”) will be returned
            TR064_CODE_ARRAYINDEXINVALID = 713, ///< SpecifiedArrayIndexInvalid
            TR064_CODE_NOSUCHENTRY = 714, ///< No such array entry in array
            TR064_CODE_INTERNALERROR = 820, ///< Internal Error
            TR064_CODE_SECONDFACTORAUTHREQUIRED = 866, ///< If an action needs 2FA, the status code 866 (“second factor authentication required”)
            TR064_CODE_SECONDFACTORAUTHBLOCKED = 867, ///< (“second factor authentication blocked”) or 
            TR064_CODE_SECONDFACTORAUTHBUSY = 868, ///< (“second factorauthentication busy”) will be returned and the 2FA procedure    
        };

        TR064();
        TR064(uint16_t port, const String& ip, const String& user, const String& pass, Protocol protocol = Protocol::useHttp, X509Certificate pCertificate = nullptr);
        ~TR064() {}
        TR064& setServer(uint16_t port, const String& ip, const String& user, const String& pass, Protocol protocol, X509Certificate certificate);
        void init();
        int state();
        
        bool action(const String& service, const String& act, String params[][2] = {}, int nParam = 0,const String& url = "");
        //bool action(const String& service, const String& act, String params[][2], int nParam, const String& url = "");
        bool action(const String& service, const String& act, String params[][2], int nParam, String (*req)[2], int nReq, const String& url = "");

        String md5String(const String& s);
        String byte2hex(byte number);
        int debug_level; ///< Available levels are `DEBUG_NONE`, `DEBUG_ERROR`, `DEBUG_WARNING`, `DEBUG_INFO`, and `DEBUG_VERBOSE`.
        bool useStreamingForXML;
         
    private:
        WiFiClient *wifiClient;
        WiFiClient wifiSimpleClient;
        WiFiClientSecure wifiSSLClient;
        HTTPClient http;
        
        //TODO: More consistent naming
        void deb_print(const String& message, int level);
        void deb_println(const String& message, int level);
        static String errorToString(int error);
		
        void initServiceURLs();
        bool action_raw(const String& service,const String& act, String params[][2], int nParam, const String& url = "");
        bool httpRequest(const String& url, const String& xml, const String& action, bool retry);
		
        String generateAuthToken();
        String generateAuthXML();
        String cleanOldServiceName(const String& service);
		
        bool xmlTakeParamsStream(String (*params)[2], int nParam);
        bool xmlTakeParamStream(String& value, const String& needParam);
		String xmlTakeParamFull(String& xml, String needParam);
        bool xmlTakeParamsFull(String (*params)[2], int nParam);
		void processGeneralXMLParam(const String& htmltag, const String& value);

        int _state; // Initialization status of the library.
        String _ip;
        uint16_t _port;
        String _user;
        String _pass;
        String _realm; // To be requested from the router
        String _secretH; // To be generated
        String _nonce = "";
        String _status; // Request status, as given by the API. Not to be confused with _state!
        X509Certificate _certificate;
        Protocol _protocol;

        const char* const _requestStart = "<?xml version=\"1.0\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">";
        const char* const _detectPage = "/tr64desc.xml";
        const char* const _servicePrefix = "urn:dslforum-org:service:";
        unsigned long lastOutActivity;
        unsigned long lastInActivity;
        std::map<String, String> _services;
};

#endif