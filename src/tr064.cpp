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
 * 
 * Initial version: November 2016<br />
 * Last updated: Feb 2020
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
*/
/**************************************************************************/
TR064::TR064(int port, String ip, String user, String pass) {
    _port = port;
    _ip = ip;
    _user = user;
    _pass = pass;
    debug_level = DEBUG_ERROR;
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
    @brief  Fetches a list of all services and the associated URLs for internal use.
*/
/**************************************************************************/
void TR064::initServiceURLs() {
    /* TODO: We should give access to this data for users to inspect the
     * possibilities of their device(s) - see #9 on Github.
     */
    String inStr = httpRequest(_detectPage, "", "");
    int CountChar = 7; //length of word "service"
    int i = 0;
    deb_println("Detected Services:", DEBUG_INFO);
    while (inStr.indexOf("<service>") > 0 || inStr.indexOf("</service>") > 0) {
        int indexStart=inStr.indexOf("<service>");
        int indexStop= inStr.indexOf("</service>");
        String serviceXML = inStr.substring(indexStart+CountChar+2, indexStop);
        String servicename = xmlTakeParam(serviceXML, "serviceType");
        String controlurl = xmlTakeParam(serviceXML, "controlURL");
        _services[i][0] = servicename;
        _services[i][1] = controlurl;
        ++i;
        
        deb_print("    " + String(i) + "\t", DEBUG_INFO);
        if (Serial) Serial.flush();
        deb_println(servicename + " @ " + controlurl, DEBUG_INFO);
        if (Serial) Serial.flush();
        inStr = inStr.substring(indexStop+CountChar+3);
    }
}

/**************************************************************************/
/*!
    @brief  <b>Deprecated, not required anymore.</b>
              Only here for backwards-compatibility.
              Fetches the initial nonce and the realm for internal use.    
*/
/**************************************************************************/
void TR064::initNonce() {
    deb_println("<warning> initNonce is deprecated and not required anymore. Might be removed in future versions.", DEBUG_WARNING);
    deb_println("Geting the initial nonce and realm", DEBUG_INFO);
    // TODO: Is this request supported by all devices or should we use a different one here?
    String a[][2] = {{"NewAssociatedDeviceIndex", "1"}};
    String xmlR = action_raw("urn:dslforum-org:service:WLANConfiguration:1", "GetGenericAssociatedDeviceInfo", a, 1);
    takeNonce(xmlR);
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
    @brief  This function will call an action on the service of the device.
            In order to understand how to construct such a call, please
            consult <a href="https://github.com/Aypac/Arduino-TR-064-SOAP-Library/wiki/How-to-create-your-first-own-API-call">the Github page</a>.
    @param    service
                The name of the service you want to adress.
    @param    act
                The action you want to perform on the service.
    @return The response from the device.
*/
/**************************************************************************/
String TR064::action(String service, String act) {
    deb_println("[action] simple", DEBUG_VERBOSE);
    String p[][2] = {{}};
    return action(service, act, p, 0);
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
    @param    req
                A list of pairs of response parameters and values, e.g
              `req[][2] = {{ "resp1", "" }, { "resp2", "" }}`
              will be turned into
              `req[][2] = {{ "resp1", "value1" }, { "resp2", "value2" }}`
    @param    nReq
                The number of response parameters you passed.
    @return The response from the device.
*/
/**************************************************************************/
String TR064::action(String service, String act, String params[][2], int nParam, String (*req)[2], int nReq) {
    deb_println("[action] with extraction", DEBUG_VERBOSE);
    String xmlR = action(service, act, params, nParam);
    String body = xmlTakeParam(xmlR, "s:Body");

    if (nReq > 0) {
        for (int i=0; i<nReq; ++i) {
            if (req[i][0] != "") {
                req[i][1] = xmlTakeParam(body, req[i][0]);
            }
        }
    }
    return xmlR;
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
    @return The response from the device.
*/
/**************************************************************************/
String TR064::action(String service, String act, String params[][2], int nParam) {
    deb_println("[action] with parameters", DEBUG_VERBOSE);
    
    String status = "unauthenticated";
    String xmlR = "";
    int tries = 0; // Keep track on the number of times we tried to request.
    while (status == "unauthenticated" && tries < 3) {
        ++tries;
        while ((_nonce == "" || _realm == "") && tries <= 3) {
            deb_println("[action] no nonce/realm found. requesting...", DEBUG_INFO);
            // TODO: Is this request supported by all devices or should we use a different one here?
            String a[][2] = {{"NewAssociatedDeviceIndex", "1"}};
            xmlR = action_raw("urn:dslforum-org:service:WLANConfiguration:1", "GetGenericAssociatedDeviceInfo", a, 1);
            takeNonce(xmlR);
            if (_nonce == "" || _realm == "") {
                ++tries;
                deb_println("[action]<error> nonce/realm request not successful!", DEBUG_ERROR);
                deb_println("[action]<error> Retrying in 5s", DEBUG_ERROR);
                delay(5000);
            }
        }
        
        xmlR = action_raw(service, act, params, nParam);
        status = xmlTakeParam(xmlR, "Status");
        deb_println("[action] Response status: "+status, DEBUG_INFO);
        status.toLowerCase();
        // If we already have a nonce, but the request comes back unauthenticated. 
        if (status == "unauthenticated" && tries < 3) {
            deb_println("[action]<error> got an unauthenticated error. Using the new nonce and trying again in 3s.", DEBUG_ERROR);
            takeNonce(xmlR);
            delay(3000);
        }
    }
    
    if (tries >= 3) {
        deb_println("[action]<error> Giving up the request ", DEBUG_ERROR);
    } else {    
        deb_println("[action] Done.", DEBUG_INFO);
        takeNonce(xmlR);
    }
    
    return xmlR;
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
    @return The response from the device.
*/
/**************************************************************************/
String TR064::action_raw(String service, String act, String params[][2], int nParam) {
    // Generate the XML-envelop
    String xml = _requestStart + generateAuthXML() + "<s:Body><u:"+act+" xmlns:u='" + service + "'>";
    // Add request-parameters to XML
    if (nParam > 0) {
        for (int i=0; i<nParam; ++i) {
            if (params[i][0] != "") {
                xml += "<"+params[i][0]+">"+params[i][1]+"</"+params[i][0]+">";
            }
        }
    }
    // Close the envelop
    xml += "</u:" + act + "></s:Body></s:Envelope>";
    // The SOAPACTION-header is in the format service#action
    String soapaction = service+"#"+act;
    
    // Send the http-Request
    return httpRequest(findServiceURL(service), xml, soapaction);
}
/**************************************************************************/
/*!
    @brief  This method will extract and remember the nonce of the current
            TR-064 call for the next one.
    @param    xml
                The XML as received from the TR-064 host (e.g. router).
*/
/**************************************************************************/
void TR064::takeNonce(String xml) {
    // Extract the Nonce for the next action/authToken.
    if (xml != "") {
        if (xmlTakeParam(xml, "Nonce") != "") {
            _nonce = xmlTakeParam(xml, "Nonce");
            deb_println("Extracted the nonce '" + _nonce + "' from the last request.", DEBUG_INFO);
        }
        if (_realm == "" && xmlTakeParam(xml, "Realm") != "") {
            _realm = xmlTakeParam(xml, "Realm");
            // Now we have everything to generate our hashed secret.
            String secr = _user + ":" + _realm + ":" + _pass;
            deb_println("Your secret is is '" + secr + "'", DEBUG_INFO);
            _secretH = md5String(secr);
            deb_println("Your hashed secret is '" + _secretH + "'", DEBUG_INFO);
        }
    }
}

// ----------------------------
// ----- Helper-functions -----
// ----------------------------


/**************************************************************************/
/*!
    @brief  Helper function, which returns the (relative) URL for a service.
    @param    service
                The name of the service you want to adress.
    @return String containing the (relative) URL for a service
*/
/**************************************************************************/
String TR064::findServiceURL(String service) {
    for (int i=0;i<arr_len(_services);++i) {
        if (_services[i][0] == service) {
            return _services[i][1];
        }
    }
    //Service not found error!
    // TODO: Proper error-handling? See also #12 on github
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
    @return The response from the device.
*/
/**************************************************************************/
String TR064::httpRequest(String url, String xml, String soapaction) {
    return httpRequest(url, xml, soapaction, true);
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
    @return The response from the device.
*/
/**************************************************************************/
String TR064::httpRequest(String url, String xml, String soapaction, bool retry) {
    deb_println("[HTTP] prepare request to URL: http://" + _ip + ":" + _port + url, DEBUG_INFO);
    
    static WiFiClient wifi;
    HTTPClient http;
    http.begin(wifi, _ip, _port, url);
    if (soapaction != "") {
        http.addHeader("CONTENT-TYPE", "text/xml"); //; charset=\"utf-8\"
        http.addHeader("SOAPACTION", soapaction);
    }
    //http.setAuthorization(fuser.c_str(), fpass.c_str());

    // start connection and send HTTP header
    int httpCode=0;
    if (xml != "") {
        deb_println("[HTTP] Posting XML:", DEBUG_VERBOSE);
        deb_println("---------------------------------", DEBUG_VERBOSE);
        deb_println(xml, DEBUG_VERBOSE);
        deb_println("---------------------------------\n", DEBUG_VERBOSE);
        httpCode = http.POST(xml);
        deb_println("[HTTP] POST... SOAPACTION: '" + soapaction + "'", DEBUG_INFO);
    } else {
        httpCode = http.GET();
        deb_println("[HTTP] GET...", DEBUG_INFO);
    }

    
    String payload = "";
    // httpCode will be negative on error
    if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        deb_println("[HTTP] request code: " + String(httpCode), DEBUG_INFO);

        // file found at server
        if (httpCode == HTTP_CODE_OK) {
            payload = http.getString();
        }
        http.end();
    } else {
        // Error
        // TODO: Proper error-handling? See also #12 on github
        String httperr = http.errorToString(httpCode).c_str();
        deb_println("[HTTP]<Error> Failed, message: '" + httperr + "'", DEBUG_ERROR);
        http.end();
        
        if (retry) {
            _nonce = "";
            deb_println("[HTTP]<Error> Trying again in 1s.", DEBUG_ERROR);
            delay(1000);
            return httpRequest(url, xml, soapaction, false);
        } else {
            deb_println("[HTTP]<Error> Giving up.", DEBUG_ERROR);
            return "";
        }
    }
    
    deb_println("[HTTP] Received back", DEBUG_VERBOSE);
    deb_println("---------------------------------", DEBUG_VERBOSE);
    deb_println(payload, DEBUG_VERBOSE);
    deb_println("---------------------------------\n", DEBUG_VERBOSE);
    return payload;
}


/**************************************************************************/
/*!
    @brief  Translates a string into its MD5 hash.
    @param  text
                Input string of which to calculcate the md5 hash.
    @return The calculated MD5 hash (as `String`).
*/
/**************************************************************************/
String TR064::md5String(String text){
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
    @return The content of the requested XML tag (as `String`).
*/
/**************************************************************************/
String TR064::xmlTakeParam(String inStr, String needParam) {
    String cont = xmlTakeSensitiveParam(inStr, needParam);
    if (cont != "") {
        return cont;
    }
    //As backup
    //TODO: Give warning?
    return xmlTakeInsensitiveParam(inStr, needParam);
}

/**************************************************************************/
/*!
    @brief  Extract the content of an XML element with a certain tag, with
            case-sensitive matching.
    @param    inStr
                The XML from which to extract.
    @param    needParam
                The name of the XML tag, you want the content of.
    @return The content of the requested XML tag (as `String`).
*/
/**************************************************************************/
String TR064::xmlTakeSensitiveParam(String inStr, String needParam) {
    return _xmlTakeParam(inStr, needParam);
}

/**************************************************************************/
/*!
    @brief  Extract the content of an XML element with a certain tag, with
            case-insensitive matching.
            Not recommend to use directly/as default, since XML is
            case-sensitive by definition/specification, this is just made
            to be used as backup, if the case-sensitive method failed.
    @param    inStr
                The XML from which to extract.
    @param    needParam
                The name of the XML tag, you want the content of.
    @return The content of the requested XML tag (as `String`).
*/
/**************************************************************************/
String TR064::xmlTakeInsensitiveParam(String inStr,String needParam) {
    needParam.toLowerCase();
    String instr = inStr;
    instr.toLowerCase();
    return _xmlTakeParam(instr, needParam);
}

/**************************************************************************/
/*!
    @brief  Underlying function to extract the content of an XML element
            with a certain tag.
    @param    inStr
                The XML (as `String`) from which to extract.
    @param    needParam
                The name of the XML tag, you want the content of.
    @return The content of the requested XML tag (as `String`).
*/
/**************************************************************************/
String TR064::_xmlTakeParam(String inStr, String needParam) {
   int indexStart = inStr.indexOf("<"+needParam+">");
   int indexStop = inStr.indexOf("</"+needParam+">");  
   if (indexStart > 0 || indexStop > 0) {
    int CountChar = needParam.length();
    return inStr.substring(indexStart+CountChar+2, indexStop);
   }
    //TODO: Proper error-handling? See also #12 on github
    return "";
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
void TR064::deb_print(String message, int level) {
    if (Serial) {
        if (debug_level >= level) {
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
void TR064::deb_println(String message, int level) {
    if (Serial) {
        if (debug_level >= level) {
            Serial.println(message);
            //Serial.flush();
        }
    }
}


