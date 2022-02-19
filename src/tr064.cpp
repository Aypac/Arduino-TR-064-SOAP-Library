/*!
 * @file tr064.cpp
 *
 * @mainpage Library for communicating via TR-064 protocol (e.g. Fritz!Box)
 *
 * @section intro_sec Introduction
 * 
 * This library allows for easy communication of TR-064 (and possibly TR-069) enabled devices,
 * such as Routers, smartplugs, DECT telephones etc.
 * Details, examples and the latest version of this library can be found on <a href="https://github.com/Aypac/Arduino-TR-064-SOAP-Library">the Github page</a>.
 * A descriptor of the protocol can be found <a href="https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AVM_TR-064_first_steps.pdf" target="_blank">here</a>.
 * Modifications to work with https by RoSchmi Feb 2022
 * 
 * Initial version: November 2016<br />
 * Last updated: Feb 2022
 *
 * @section dependencies Dependencies
 *
 * This library depends on:<ul>
 * <li>MD5Builder (versions for <a href="https://github.com/esp8266/Arduino/blob/master/cores/esp8266/MD5Builder.cpp" target="_blank">ESP8266</a>, <a href="https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/MD5Builder.h" target="_blank">ESP32</a></li>)
 * <li><a href="https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPClient" target="_blank">ESP8266HTTPClient</a> (ESP8266) or <a href="https://github.com/espressif/arduino-esp32/tree/master/libraries/HTTPClient" target="_blank">HTTPClient</a> (ESP32)</li></ul>
 *
 * @section author Author
 *
 * Written by René Vollmer (aka <a href="https://github.com/Aypac" target="_blank">Aypac</a>), with contributions from others. See <a href="https://github.com/Aypac/Arduino-TR-064-SOAP-Library">Github for details</a>.
 *
 * @section license License
 *
 * <a href="https://github.com/Aypac/Arduino-TR-064-SOAP-Library/blob/master/LICENSE">MIT License</a>
 *
 */


#include "tr064.h"


/**************************************************************************/
/*! 
    @brief  Library/Class to easily make TR-064 calls. Do not construct this
            unless you have a working connection to the device!
    @param    port
                Port number to be used to establish the TR-064 connection.
    @param    ip
                IP address to be used to establish the TR-064 connection.
    @param    user
                User name to be used to establish the TR-064 connection.
    @param    pass
                Password to be used to establish the TR-064 connection.
    @param    protocol
                Transmission protocol to be used (http/https).
    @param    certificate
                X509Certificate of Fritzbox to be used for https transmission.
*/
/**************************************************************************/
TR064::TR064(uint16_t port, const String& ip, const String& user, const String& pass, Protocol protocol, X509Certificate certificate) { 
    _port = port;
    _ip = ip;
    _user = user;
    _pass = pass;
    debug_level = DEBUG_NONE;
    this->_state = TR064_NO_SERVICES;
    _certificate = certificate;
    _protocol = protocol;  
    if (protocol == Protocol::useHttps)
    {
        tr064SslClient.setCACert(certificate);
        tr064ClientPtr = &tr064SslClient;       
    }
    else
    {
        tr064ClientPtr = &tr064SimpleClient;
    }  
}

/**************************************************************************/

/*! 
    @brief  Library/Class to easily make TR-064 calls. Do not construct this
            unless you have a working connection to the device!   
*/
/**************************************************************************/
TR064::TR064() {
   debug_level = DEBUG_NONE;
   this->_state = TR064_NO_SERVICES;
}


/**************************************************************************/
/*!
    @brief  Initializes the library. Needs to be explicitly called.
            There should already be a working connection to the device.
*/
/**************************************************************************/
void TR064::init() {
    delay(100); // TODO: REMOVE (after testing, that it still works!)
    // Get a list of all services and the associated urls
    initServiceURLs();  
}


/**************************************************************************/
/*!
    @brief  Set the server parameters, needed if the empty constructor was used.
    @return Reference to this object
    @param    port
                Port number to be used to establish the TR-064 connection.
    @param    ip
                IP address to be used to establish the TR-064 connection.
    @param    user
                User name to be used to establish the TR-064 connection.
    @param    pass
                Password to be used to establish the TR-064 connection.
    @param    protocol
                Transmission protocol to be used (http/https).
    @param    certificate
                X509Certificate of Fritzbox to be used for https transmission.
*/
/**************************************************************************/
TR064& TR064::setServer(uint16_t port, const String& ip, const String& user, const String& pass, Protocol protocol, X509Certificate certificate){
    this->_ip = ip;
    this->_port = port;
    this->_user = user;
    this->_pass = pass; 
    _certificate = certificate;
    _protocol = protocol;  
    if (protocol == Protocol::useHttps)
    {
        tr064SslClient.setCACert(certificate);
        tr064ClientPtr = &tr064SslClient;       
    }
    else
    {
        tr064ClientPtr = &tr064SimpleClient;
    }
    return *this;
}

/**************************************************************************/
/*!
    @brief  Fetches a list of all services and the associated URLs for internal use.
*/
/**************************************************************************/
/**************************************************************************/
void TR064::initServiceURLs() {
    /* TODO: We should give access to this data for users to inspect the
     * possibilities of their device(s) - see #9 on Github.
     */

    this->_state = TR064_NO_SERVICES;
        if(httpRequest(_detectPage, "", "", true, _protocol))
        {
            deb_println("[TR064][initServiceURLs] get the Stream ", DEBUG_INFO);
            int i = 0;
            while(1) {              
                if(!http.connected()) {   
                    deb_println("[TR064][initServiceURLs] xmlTakeParam : http connection lost", DEBUG_INFO);
                    break;                      
                }
                if(xmlTakeParam(_services[i][0], "serviceType")) {
                    deb_print("[TR064][initServiceURLs] "+ String(i) + "\treadServiceName: "+ _services[i][0] , DEBUG_VERBOSE);
                    if(xmlTakeParam(_services[i][1], "controlURL")) {
                        deb_println(" @ readServiceUrl: "+ _services[i][1], DEBUG_VERBOSE);
                        i++;
                    }else{
                        deb_println(" @ readServiceUrl: NOTFOUND", DEBUG_VERBOSE);
                        break;
                    }
                }else{
                    deb_println(" @ serviceType: NOTFOUND", DEBUG_VERBOSE);
                    break;
                }
            }            
            deb_println("[TR064][initServiceURLs] message: reading done", DEBUG_INFO);                    
    } 
    else {  
        deb_println("[TR064][initServiceURLs]<Error> initServiceUrls failed", DEBUG_ERROR);  
        return;      
    }
    _state = TR064_SERVICES_LOADED;   
    http.end();
}

/**************************************************************************/
/*!
    @brief  Generates and returns the XML-header for authentification.
    @return The XML-header (as `String`).
*/
/**************************************************************************/
String TR064::generateAuthXML() {
    String token;
    if (_nonce == "") {
        // If we do not have a nonce yet, we need to use a different header
        token = "<s:Header><h:InitChallenge xmlns:h=\"http://soap-authentication.org/digest/2001/10/\" s:mustUnderstand=\"1\"><UserID>" + _user + "</UserID></h:InitChallenge ></s:Header>";
    } else {
        // Otherwise we produce an authorisation header
        token = generateAuthToken();
        token = "<s:Header><h:ClientAuth xmlns:h=\"http://soap-authentication.org/digest/2001/10/\" s:mustUnderstand=\"1\"><Nonce>" + _nonce + "</Nonce><Auth>" + token + "</Auth><UserID>" + _user + "</UserID><Realm>" + _realm + "</Realm></h:ClientAuth></s:Header>";
    }
    return token;
}

/**************************************************************************/
/*!
    @brief  Returns the authentification token based on the hashed secret and the last nonce.
    @return The authentification token (as `String`).
*/
/**************************************************************************/
String TR064::generateAuthToken() {
    String token = md5String(_secretH + ":" + _nonce);
    deb_println("The auth token is '" + token + "'", DEBUG_INFO);
    return token;
}

/**************************************************************************/
/*!
    @brief  This function will call an action on the service of the device
            with certain parameters and return values.
            It will fill the array `req` with the values of the assiciated
            return variables of the request.
            In order to understand how to construct such a call, please
            consult <a href="https://github.com/Aypac/Arduino-TR-064-SOAP-Library/wiki/How-to-create-your-first-own-API-call">the Github page</a>.
    @param    service
                The name of the service you want to adress.
    @param    act
                The action you want to perform on the service.
    @param    params
                A list of pairs of input parameters and values, e.g
              `params[][2] = {{ "arg1", "value1" }, { "arg2", "value2" }}`.
    @param    nParam
                The number of input parameters you passed.   
    @param    url
                The url you want to call.
    @return success state.
*/
/**************************************************************************/
bool TR064::action(const String& service, const String& act, String params[][2], int nParam,  const String& url) {
    deb_println("[TR064]", DEBUG_VERBOSE);
    deb_println("[TR064][action] with parameters", DEBUG_VERBOSE);
    String req[][2] = {{}};
    if(action(service, act, params, nParam, req, 0, url)){
              
        http.end();
        return true;
    }
      
    http.end();    
    return false;
}


/**************************************************************************/
/*!
    @brief  This function will call an action on the service of the device
            with certain parameters.
            In order to understand how to construct such a call, please
            consult <a href="https://github.com/Aypac/Arduino-TR-064-SOAP-Library/wiki/How-to-create-your-first-own-API-call">the Github page</a>.
    @param    service
                The name of the service you want to adress.
    @param    act
                The action you want to perform on the service.
    @param    params
                A list of pairs of input parameters and values, e.g
              `params[][2] = {{ "arg1", "value1" }, { "arg2", "value2" }}`.
    @param    nParam
                The number of input parameters you passed.
    @param    req
                A list of pairs of response parameters and values, e.g
              `req[][2] = {{ "resp1", "" }, { "resp2", "" }}`
              will be turned into
              `req[][2] = {{ "resp1", "value1" }, { "resp2", "value2" }}`
    @param    nReq
                The number of response parameters you passed.                
    @param    url
                The url you want to call.
    @return success state.
*/
/**************************************************************************/
bool TR064::action(const String& service, const String& act, String params[][2], int nParam, String (*req)[2], int nReq, const String& url) {
    deb_println("[TR064][action] with extraction", DEBUG_VERBOSE);
    
    int tries = 0; // Keep track on the number of times we tried to request.
    if (action_raw(service, act, params, nParam, url)) 
    {
        if(xmlTakeParam(req, nReq))
        {
            deb_println("[TR064][action] extraction complete.", DEBUG_VERBOSE);
        }
        else
        {
            return false;
        }
        deb_println("[TR064][action] Response status: "+ _status +", Tries: "+String(tries), DEBUG_INFO);
        if(_status == "unauthenticated"){
            
            while (_status == "unauthenticated"  && (_nonce == "" || _realm == "") && tries <= 3) {
                ++tries;
                deb_println("[TR064][action] No nonce/realm found. Requesting...", DEBUG_INFO);
                // TODO: Is this request supported by all devices or should we use a different one here?
                String a[][2] = {{"NewAssociatedDeviceIndex", "1"}};
                String wlanService = "WLANConfiguration:1", deviceInfo="GetGenericAssociatedDeviceInfo";
                action_raw(wlanService, deviceInfo, a, 1, "/upnp/control/wlanconfig1");
                             
                if(xmlTakeParam(req, nReq)){
                    deb_println("[TR064][action] extraction complete.", DEBUG_VERBOSE);
                }
                
                if (_nonce == "" || _realm == "") {
                    ++tries;
                    deb_println("[TR064][action]<Error> Nonce/realm request not successful!", DEBUG_ERROR);
                    deb_println("[TR064][action]<Error> Retrying in 5s", DEBUG_ERROR);
                    delay(5000);
                }
               
                http.end();             
            }
            if (tries >= 3) {
                deb_println("[TR064][action]<error> Giving up the request ", DEBUG_ERROR);               
                http.end();
                return false;
            }
            return action(service, act, params, nParam, req, nReq, url);
        }
        deb_println("[TR064][action] Done.", DEBUG_INFO);       
        http.end();
        return true;
        
    }
    deb_println("[TR064][action]<error> Request Failed ", DEBUG_ERROR);
    http.end();
    return false;
}

/**************************************************************************/
/*!
    @brief  This function will call an action on the service of the device
            with certain parameters, without error catching etc. Not
            recommended for direct use.
    @param    service
                The name of the service you want to adress.
    @param    act
                The action you want to perform on the service.
    @param    params
                A list of pairs of input parameters and values, e.g
              `params[][2] = {{ "arg1", "value1" }, { "arg2", "value2" }}`.
    @param    nParam
                The number of input parameters you passed (in `params`).
    @param    url
                The url you want to call.
    @return success state.
*/
/**************************************************************************/

bool TR064::action_raw(const String& service, const String& act, String params[][2], int nParam, const String& url) {
    // Generate the XML-envelop
    String serviceName = cleanOldServiceName(service);
    String xml = _requestStart + generateAuthXML() + "<s:Body><u:"+act+" xmlns:u=\"" + _servicePrefix + serviceName + "\">";
    // Add request-parameters to XML
    if (nParam > 0) {
        for (uint16_t i=0; i<nParam; ++i) {
            if (params[i][0] != "") {
                deb_println("[TR064][action_raw] with parameter, "+params[i][0], DEBUG_VERBOSE);
                deb_println("[TR064][action_raw] with parametervalue, "+params[i][1], DEBUG_VERBOSE);
                xml += "<"+params[i][0]+">"+params[i][1]+"</"+params[i][0]+">";
            }
        }
    }
    // Close the envelop
    xml += "</u:" + act + "></s:Body></s:Envelope>";
    
    // The SOAPACTION-header is in the format service#action
    String soapaction = _servicePrefix + serviceName+"#"+act;
    
    // Send the http-Request
    if(url !=""){
        return httpRequest(url, xml, soapaction, true, _protocol);
    }else{
        return httpRequest(findServiceURL(_servicePrefix + serviceName), xml, soapaction, true, _protocol);
    }
}

/**************************************************************************/
/*!
    @brief  Returns the State of Service Load
    @return The State. TR064_NO_SERVICES / TR064_SERVICES_LOADED
*/
/**************************************************************************/
int TR064::state() {    
    return this->_state;
}

// ----------------------------
// ----- Helper-functions -----
// ----------------------------

/**************************************************************************/
/*!
 * @brief converts error code to String
 * @param error int
 * @return String
 */
/**************************************************************************/
String TR064::errorToString(int error)
{
    switch(error) {
    case TR064_CODE_AUTHFAILED:
        return F("Authentication failed. No Secret in Header?");    
    case TR064_CODE_UNKNOWNACTION:
        return F("Unknown or obsolete action.");
    case TR064_CODE_FALSEARGUMENTS:
        return F("The number of arguments for an action is not as expected or an unexpected argument is used");
    case TR064_CODE_ARGUMENTVALUEINVALIDE:
        return F("Parameter Value in action not valid.");
    case TR064_CODE_ACTIONNOTAUTHORIZED:
        return F("User is authenticated but has not the needed rights, 606 (Action not authorized)");
    case TR064_CODE_ARRAYINDEXINVALID:
        return F("Parameter Value in action not found.");
    case TR064_CODE_NOSUCHENTRY:
        return F("No Entry Found");
    case TR064_CODE_INTERNALERROR:
        return F("Internal Error, please read Fritz Error Log");
    case TR064_CODE_SECONDFACTORAUTHREQUIRED:
        return F("Action needs 2FA, the status code 866 (second factor authentication required)");
    case TR064_CODE_SECONDFACTORAUTHBLOCKED:
        return F("Second factor authentication blocked");
    case TR064_CODE_SECONDFACTORAUTHBUSY:
        return F("Second factorauthentication busy");    
    default:
        return String();
    }
}

/**************************************************************************/
/*!
    @brief  Helper function, which deletes the prefix before the servicename.
    @param    service
                The name of the service you want to adress.
    @return String servicename without prefix
*/
/**************************************************************************/
String TR064::cleanOldServiceName(const String& service) {
    deb_println("[TR064][cleanOldServiceName] searching for prefix in servicename: "+service, DEBUG_VERBOSE);
    if(service.startsWith(_servicePrefix)){
        return service.substring(strlen(_servicePrefix));        
    }
    return service;
}

/**************************************************************************/
/*!
    @brief  Helper function, which returns the (relative) URL for a service.
    @param    service
                The name of the service you want to adress.
    @return String containing the (relative) URL for a service
*/
/**************************************************************************/
String TR064::findServiceURL(const String& service) {
    if(state()<TR064_SERVICES_LOADED){
        deb_println("[TR064][findServiceURL]<error> Services NOT Loaded. ", DEBUG_ERROR);
        return "";
    }else{
    
        deb_println("[TR064][findServiceURL] searching for service: "+service, DEBUG_VERBOSE);

        for (uint16_t i=0;i<arr_len(_services);++i) {            
            if (service.equalsIgnoreCase(_services[i][0])) {
                deb_println("[TR064][findServiceURL] found services: "+service+" = "+ _services[i][0]+" , "+ _services[i][1], DEBUG_VERBOSE);
                return _services[i][1];
            }
        }
    }
    return "";
}


/**************************************************************************/
/*!
    @brief  Transmits a http-Request to the given url (relative to _ip on _port)
            - if specified POSTs xml and adds soapaction as header field.
            - otherwise just GETs the url
    @param    url
                The service URL
    @param    soapaction
                The requested action
    @param    xml
                The request XML
    @param    retry
                Should the request be repeated with a new nonce, if it fails?
    @param    protocol
                Transmission protocol to be used (http/https).
                

    @return The response from the device.
*/
/**************************************************************************/
bool TR064::httpRequest(const String& url, const String& xml, const String& soapaction, bool retry, Protocol protocol) {
    if (url=="") {
        deb_println("[TR064][httpRequest] URL is empty, abort http request.", DEBUG_INFO);
        return false;
    }
    
    String protocolPrefix = protocol == Protocol::useHttps ? "https://" : "http://";
    bool useTls = protocol == Protocol::useHttps ? true : false;

    deb_println("[HTTP] prepare request to URL: " + protocolPrefix + _ip + ":" + _port + url, DEBUG_INFO);
    http.setReuse(true);

    if (protocol == Protocol::useHttps)
    {   
        http.begin(tr064SslClient, _ip.c_str(), _port, url.c_str(), useTls);
        http.setConnectTimeout(2000);        
    }
    else
    {
        http.begin(tr064SimpleClient, _ip.c_str(), _port, url.c_str(), useTls);        
    }
    
    if (soapaction != "") {  
        http.addHeader("CONTENT-TYPE", "text/xml"); //; charset=\"utf-8\"
        http.addHeader("SOAPACTION", soapaction);
    }
    
    http.addHeader("ACCEPT-ENCODING", "chunked");
   
    int httpCode=0;
    if (xml != "") {
        deb_println("[HTTP] Posting XML:", DEBUG_VERBOSE);
        deb_println("---------------------------------", DEBUG_VERBOSE);
        deb_println(xml, DEBUG_VERBOSE);
        deb_println("---------------------------------\n", DEBUG_VERBOSE);        
        httpCode = http.POST(xml);
        deb_println("[HTTP] POST... SOAPACTION: '" + soapaction + "'", DEBUG_VERBOSE);
    } else {         
        httpCode = http.GET();
        deb_println("[HTTP] GET...", DEBUG_VERBOSE);
    }

    // httpCode will be negative on error  
    if (httpCode > 0) {
        // HTTP header has been sent and Server response header has been handled
          
        if (httpCode == HTTP_CODE_OK) {             
            return true;
        }else{
            if (httpCode == HTTP_CODE_INTERNAL_SERVER_ERROR) { 
                String req[][2] = {{"errorCode",""},{"errorDescription",""}};
                if (xmlTakeParam(req, 2)) {                                
                    if(req[0][1]!=""){
                        deb_println("[TR064][httpRequest] <TR064> Failed, errorCode: '" + req[0][1]  + "'", DEBUG_VERBOSE);                    
                        deb_println("[TR064][httpRequest] <TR064> Failed, message: '" + errorToString(req[0][1].toInt())  + "'", DEBUG_ERROR);
                        deb_println("[TR064][httpRequest] <Error> Failed, description: '" + req[1][1] + "'", DEBUG_VERBOSE);
                    }
                }
                    
            }
            return false;
        }

    } else {
        // Error
        // TODO: Proper error-handling? See also #12 on github
        
        String httperr = http.errorToString(httpCode).c_str();

        deb_println("[TR064][httpRequest]<Error> Failed, message: '" + httperr + "'", DEBUG_ERROR);

        if (retry) {
            _nonce = "";
            deb_println("[TR064][httpRequest] <Error> Trying again in 1s.", DEBUG_ERROR);
            delay(1000);
            return httpRequest(url, xml, soapaction, false);
        } else {
            deb_println("[TR064][httpRequest] <Error> Giving up.", DEBUG_ERROR);
            return false;
        }
    }  
    return true;
}
            

/**************************************************************************/
/*!
    @brief  Translates a string into its MD5 hash.
    @param  text
                Input string of which to calculcate the md5 hash.
    @return The calculated MD5 hash (as `String`).
*/
/**************************************************************************/
String TR064::md5String(const String& text){
    byte bbuff[16];
    String hash = "";
    MD5Builder nonce_md5; 
    nonce_md5.begin();
    nonce_md5.add(text); 
    nonce_md5.calculate(); 
    nonce_md5.getBytes(bbuff);
    for (byte i = 0; i < 16; i++) hash += byte2hex(bbuff[i]);
    return hash;   
}

/**************************************************************************/
/*!
    @brief  Translates a byte number into a hex number.
    @param    number
                `byte` number to be translated to hex.
    @return Translated hex number (as `String`).
*/
/**************************************************************************/
String TR064::byte2hex(byte number){
    String Hstring = String(number, HEX);
    if (number < 16) {Hstring = "0" + Hstring;}
    return Hstring;
}


/**************************************************************************/
/*!
    @brief  Extract the content of an XML element with a certain tag. It
            tries with case-sensitive
            matching first, but resorts to case-insensitive matching, when
            failing.
    @param    inStr
                The XML from which to extract.
    @param    needParam
                The name of the XML tag, you want the content of.
    @return success state.
*/
/**************************************************************************/
bool TR064::xmlTakeParam(String (*params)[2], int nParam) {
    WiFiClient * stream = tr064ClientPtr;
     
    while(stream->connected()) {  
        if(!http.connected()) {
            deb_println("[TR064][xmlTakeParam] http connection lost", DEBUG_INFO);
            return false;                      
        }        
        if(stream->find("<")){
            const String htmltag = stream->readStringUntil('>');
            const String value = stream->readStringUntil('<');
            deb_println("[TR064][xmlTakeParam] htmltag: "+htmltag, DEBUG_VERBOSE);
            
            if (nParam > 0) {
                for (uint16_t i=0; i<nParam; ++i) {
                    if(htmltag.equalsIgnoreCase(params[i][0])){
                        params[i][1] = value; 
                        deb_println("[TR064][action] found requestparameter: "+params[i][0]+" = "+params[i][1], DEBUG_VERBOSE);
                    }   
                }
            }            
            if(htmltag.equalsIgnoreCase("Nonce")){
                _nonce = value;
                deb_println("[TR064][xmlTakeParam] Extracted the nonce '" + _nonce + "' from the last respuest.", DEBUG_INFO);
            }  
            if (_realm == "" && htmltag.equalsIgnoreCase("Realm")) {
                _realm = value;
                // Now we have everything to generate our hashed secret.
                String secr = _user + ":" + _realm + ":" + _pass;
                deb_println("[TR064][xmlTakeParam] Your secret is is '" + secr + "'", DEBUG_INFO);
                _secretH = md5String(secr);
                deb_println("[TR064][xmlTakeParam] Your hashed secret is '" + _secretH + "'", DEBUG_INFO);
            }
            if(htmltag.equalsIgnoreCase("Status")){             
                _status = value;
                _status.toLowerCase();
                deb_println("[TR064][xmlTakeParam] Response status: "+ _status , DEBUG_INFO);
            }
            if(htmltag.equalsIgnoreCase("errorCode")){
                deb_println("[TR064][xmlTakeParam] <TR064> Failed, errorCode: '" + value  + "'", DEBUG_VERBOSE);
                deb_println("[TR064][xmlTakeParam] <TR064> Failed, message: '" + errorToString(value.toInt())  + "'", DEBUG_VERBOSE);
            }
            if(htmltag.equalsIgnoreCase("errorDescription")){
                deb_println("[TR064][xmlTakeParam] <TR064> Failed, errorDescription: " + value, DEBUG_VERBOSE);
            }        
        }else{
            break;    
        }
        
    }
    return true;
}

/**************************************************************************/
/*!
    @brief  Extract the content of an XML element with a certain tag. It
            tries with case-sensitive
            matching first, but resorts to case-insensitive matching, when
            failing.
    @param    inStr
                The XML from which to extract.
    @param    needParam
                The name of the XML tag, you want the content of.
    @return success state.
*/
/**************************************************************************/
bool TR064::xmlTakeParam(String& value, const String& needParam) {   
    WiFiClient * stream = tr064ClientPtr;
    
    while(stream->connected()) {       
        if(!http.connected()) {  
            deb_println("[TR064][xmlTakeParam] http connection lost", DEBUG_INFO);
            return false;                      
        }
       
        if(stream->find("<")){
            const String htmltag = stream->readStringUntil('>');
            //deb_println("[TR064][xmlTakeParam] htmltag: "+htmltag, DEBUG_VERBOSE);
            if(htmltag.equalsIgnoreCase(needParam)){
                value = stream->readStringUntil('<');                
                break;
            }       
        }else{
            return false;    
        }
    }
    return true;
}

/**************************************************************************/
/*!
    @brief  Debug-print. Only prints the message if the debug level is high enough.
    @param    message
                The message to be conditionally printed.
    @param    level
                The minimally required debug level.
*/
/**************************************************************************/
void TR064::deb_print(const String& message, int level) {
    if (Serial) {
        if (debug_level >= level) {
            // where is location?
            Serial.print(message);
            //Serial.flush();
        }
    }
}

/**************************************************************************/
/*!
    @brief  Same as deb_print, but with a new line at the end.
            Only prints the message if the debug level is high enough.
    @param    message
                The message to be conditionally printed.
    @param    level
                The minimally required debug level.
*/
/**************************************************************************/
void TR064::deb_println(const String& message, int level) {
    if (Serial) {
        if (debug_level >= level) {
            Serial.println(message);
            //Serial.flush();
        }
    }
}