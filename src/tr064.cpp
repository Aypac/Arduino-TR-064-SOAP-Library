/*!
 * @file tr064.cpp
 *
 * @mainpage Arduino library for communicating via TR-064 protocol (e.g. FRITZ!Box)
 *
 * @section intro_sec Introduction
 * 
 * This library allows for easy communication with TR-064 (and possibly TR-069) enabled devices,
 * such as Routers, smartplugs, DECT telephones etc.
 * Details, examples and the latest version of this library can be found on <a href="https://github.com/Aypac/Arduino-TR-064-SOAP-Library">the Github page</a>.
 * A descriptor of the protocol can be found <a href="https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AVM_TR-064_first_steps.pdf" target="_blank">here</a>.
 * 
 * Initial version: November 2016<br />
 * Modifications to work with https by RoSchmi<br />
 * Last updated: Jan 2023<br />
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
                X509Certificate of TR-064 host to be used for https transmission.
*/
/**************************************************************************/
TR064::TR064(uint16_t port, const String& ip, const String& user, const String& pass,
             Protocol protocol, X509Certificate certificate) {
    this->setServer(port, ip, user, pass, protocol, certificate);
	
    debug_level = DEBUG_NONE;
    this->_state = TR064_NO_SERVICES;
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
                X509Certificate of TR-064 host to be used for https transmission.
*/
/**************************************************************************/
TR064& TR064::setServer(uint16_t port, const String& ip, const String& user,
                        const String& pass, Protocol protocol, X509Certificate certificate) {
    this->_ip = ip;
    this->_port = port;
    this->_user = user;
    this->_pass = pass;
    _certificate = certificate;
    _protocol = protocol;
    if (protocol == Protocol::useHttp) {
        tr064Client = &tr064SimpleClient;
    } else {
        if (protocol == Protocol::useHttpsInsec) {
            tr064SslClient.setInsecure();
        } else {
            #if defined(ESP32)
				tr064SslClient.setCACert(certificate);
            #endif
        }
        tr064Client = &tr064SslClient;
    }
    return *this;
}


/**************************************************************************/
/*!
    @brief  Fetches a list of all services and the associated URLs for internal use.
*/
/**************************************************************************/
void TR064::initServiceURLs() {
    /* TODO: We should give access to this data for users to inspect the
     * possibilities of their device(s) - see #9 on Github.
		  for (auto const& service : _services) {
			Serial.println(service.first + ": " + service.second);
		  }
     */

    _state = TR064_NO_SERVICES;
	deb_println("[TR064][initServiceURLs] getting the service detect page", DEBUG_INFO);
    if (httpRequest(_detectPage, "", "", true)) {
		
		int i = 0;
		while (!tr064Client->connected()) {
			delay(10);
			++i;
			if (i > 100) {
				deb_println("[TR064][initServiceURLs]<Error> initServiceUrls failed: could not connect", DEBUG_ERROR);
				return;
			}
		}
		
		// Scan the XML stream for <serviceType> followed by <controlURL> XML tags.
		// Fill their content into the _service Map (dict).
		// Continue until you can't find any new <serviceType> XML tags.
		i = 0;
		deb_println("[TR064][initServiceURLs] Searching the XML for serviceType", DEBUG_VERBOSE);
		while (true) {
			String key;
			if (xmlTakeParam(key, "serviceType")) {
				if (key != "") {
					String value;
					if (xmlTakeParam(value, "controlURL") && value != "") {
						deb_println("[TR064][initServiceURLs] "+ String(i) + "\t" + key + " => " + value, DEBUG_INFO);
						_services[key] = value;
						++i;
					} else {
						deb_println("[TR064][initServiceURLs] "+ String(i) + "\t" + key + " => No control URL found", DEBUG_WARNING);
					}
				} else {
					deb_println("[TR064][initServiceURLs] "+ String(i) + "\t" + key + " => Empty key", DEBUG_WARNING);
				}
			} else {
				// If xmlTakeParam returns false, we reached the end of the XML
				break;
			}
			// This should not trigger. But we have seen problems with this before, so it is better to have an emergency exit.
			if (i > 200) {
				deb_println("[TR064][initServiceURLs] More than 200 services found. Something probably went wrong trying to parse the services - so I'll stop here to prevent the MCU from crashing.", DEBUG_WARNING);
				break;
			}
		}
		deb_println("[TR064][initServiceURLs] Found "+ String(i) + " services in total.", DEBUG_INFO);
		
		_state = TR064_SERVICES_LOADED;
    } else {
        deb_println("[TR064][initServiceURLs]<Error> initServiceUrls failed", DEBUG_ERROR);
        return;
    }
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
	// Now we have everything to generate our hashed secret.
	String secr = _user + ":" + _realm + ":" + _pass;
	deb_println("[TR064][generateAuthToken] Your secret is is '" + secr + "'", DEBUG_INFO);
	_secretH = md5String(secr);
	deb_println("[TR064][generateAuthToken] Your hashed secret is '" + _secretH + "'", DEBUG_INFO);
	
    String token = md5String(_secretH + ":" + _nonce);
    deb_println("[TR064][generateAuthToken] The auth token is '" + token + "'", DEBUG_INFO);
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
			  Optional.
    @param    nParam
                The number of input parameters you passed.
			  Optional.
    @param    url
                The url you want to call.
			  Optional, if initServiceURLs was called.
    @return success state.
*/
/**************************************************************************/
bool TR064::action(const String& service, const String& act, String params[][2],
                   int nParam,  const String& url) {
	// This is just a wrapper for the other action function,
	//   that's why it is so short :)
    deb_println("[TR064][action] with parameters", DEBUG_VERBOSE);
    String req[][2] = {{}};
    return action(service, act, params, nParam, req, 0, url);
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
			  Optional, if initServiceURLs was called.
    @return success state.
*/
/**************************************************************************/
bool TR064::action(const String& service, const String& act, String params[][2],
                   int nParam, String (*req)[2], int nReq, const String& url) {
    deb_println("[TR064][action] with extraction", DEBUG_VERBOSE);
	
	bool retry = false;
	
	// Do the action. Get the results.
	
	// It can sometimes happen that we have no or an old nonce, in which case
	//  we will want to re-try at least up to two times.
	// Attempt 1 fails because of no/old nonce, extracts Nonce
    if (action_raw(service, act, params, nParam, url)) {
		retry = !xmlTakeParams(req, nReq);
        if (!retry) {
            deb_println("[TR064][action] return parameters sucessful.", DEBUG_VERBOSE);
			retry = (_status == "unauthenticated");
        }
        deb_println("[TR064][action] Response status: "+ _status, DEBUG_INFO);
		
        if (retry) {
            if (_nonce == "" || _realm == "") {
				deb_println("[TR064][action] Error, request failed. Info:", DEBUG_ERROR);
                deb_println("     First attempt failed, but no nonce/realm found, this should not happen. Please report this.", DEBUG_ERROR);
				return false;
            }
			
			// Attempt 2 should always work;
			// From the previous (unauthenticated) request, we should now have a Nonce and realm,
			//   so we should be able to execute the action now
            if (action_raw(service, act, params, nParam, url)) {
				// Success.
				return xmlTakeParams(req, nReq);
			} else {
				deb_println("[TR064][action] Error, request failed ", DEBUG_ERROR);
				return false;
			}
        }
		// Success.
        return true;
    }
	deb_println("[TR064][action] Error, request failed ", DEBUG_ERROR);
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
    @param    protocol
                Transmission protocol to be used (http/https).
    @return success state.
*/
/**************************************************************************/
bool TR064::action_raw(const String& service, const String& act, String params[][2],
                       int nParam, const String& url) {
    // Generate the XML-envelop
    String serviceName = cleanOldServiceName(service);
    String xml = _requestStart + generateAuthXML();
	xml += "<s:Body><u:"+act+" xmlns:u=\"" + _servicePrefix + serviceName + "\">";
    // Add request-parameters to XML
    if (nParam > 0) {
        for (uint16_t i=0; i<nParam; ++i) {
            if (params[i][0] != "") {
                deb_println("[TR064][action_raw] Parameter: "+params[i][0]+" => "+params[i][1], DEBUG_VERBOSE);
                xml += "<"+params[i][0]+">"+params[i][1]+"</"+params[i][0]+">";
            }
        }
    }
    // Close the envelop
    xml += "</u:" + act + "></s:Body></s:Envelope>";
    
    // The SOAPACTION-header is in the format service#action
    String soapaction = _servicePrefix + serviceName+"#"+act;
    
    // Send the http-Request
    if (url != "") {
        return httpRequest(url, xml, soapaction, true);
    } else {
        return httpRequest(_services[_servicePrefix + serviceName], xml, soapaction, true);
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
String TR064::errorToString(int error) {
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
			return F("Parameter value in action not found.");
		case TR064_CODE_NOSUCHENTRY:
			return F("No Entry Found");
		case TR064_CODE_INTERNALERROR:
			return F("Internal Error, please check TR064 host's error log");
		case TR064_CODE_SECONDFACTORAUTHREQUIRED:
			return F("Action needs 2FA, the status code 866 (second factor authentication required)");
		case TR064_CODE_SECONDFACTORAUTHBLOCKED:
			return F("Second factor authentication blocked");
		case TR064_CODE_SECONDFACTORAUTHBUSY:
			return F("Second factor authentication busy");
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
    if (service.startsWith(_servicePrefix)) {
        return service.substring(strlen(_servicePrefix));
    }
    return service;
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
                Should the request be repeated, if it fails?
    @return success state.
*/
/**************************************************************************/
bool TR064::httpRequest(const String& url, const String& xml, const String& soapaction, bool retry) {
    if (url == "") {
        deb_println("[TR064][httpRequest] URL is empty, abort http request.", DEBUG_ERROR);
        return false;
    }
	
    deb_println("[TR064][httpRequest] prepare request to URL: " + _ip + ":" + _port + url, DEBUG_INFO);
    http.setReuse(true);

    http.begin(*tr064Client, _ip.c_str(), _port, url.c_str(), bool(_protocol == Protocol::useHttp));
	
	// http.setConnectTimeout not defined on ESP8266...
	#if defined(ESP32)
		http.setConnectTimeout(2000);
	#endif
    
    if (soapaction != "") {
        http.addHeader("CONTENT-TYPE", "text/xml");
        http.addHeader("SOAPACTION", soapaction);
    }
	
	// Place http request
    int httpCode=0;
    if (xml != "") {
        deb_println("[TR064][httpRequest] Posting XML with SOAPACTION: '" + soapaction + "'", DEBUG_INFO);
        deb_println("[TR064][httpRequest] ---------------------------------", DEBUG_VERBOSE);
        deb_println(xml, DEBUG_VERBOSE);
        deb_println("[TR064][httpRequest] ---------------------------------", DEBUG_VERBOSE);
        
        httpCode = http.POST(xml);
    } else {
        deb_println("[TR064][httpRequest] GET with SOAPACTION: '" + soapaction + "'", DEBUG_INFO);
        httpCode = http.GET();
    }

    deb_println("[TR064][httpRequest] HTTP response code: " + String(httpCode), DEBUG_INFO);
    // httpCode will be negative on error
    if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        
		// We assume we got a response, so we can check what the TR-064 host has to tell us.
		// If this causes problems, we can limit it to these codes:
		// if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_INTERNAL_SERVER_ERROR) {
        return true;
    } else {
        // Error
        // TODO: Proper error-handling? See also #12 on github
        String httperr = http.errorToString(httpCode).c_str();
        deb_println("[TR064][httpRequest] HTTP ERROR, message: '" + httperr + "'", DEBUG_ERROR);
	}
	
	// If it reaches here, something went wrong...
	if (retry) {
		deb_println("[TR064][httpRequest] HTTP error, trying again in 1s.", DEBUG_ERROR);
		delay(1000);
		return httpRequest(url, xml, soapaction, false);
	} else {
		deb_println("[TR064][httpRequest] HTTP error, giving up.", DEBUG_ERROR);
	}
    return false;
}


/**************************************************************************/
/*!
    @brief  Translates a string into its MD5 hash.
    @param  text
                Input string of which to calculcate the md5 hash.
    @return The calculated MD5 hash (as `String`).
*/
/**************************************************************************/
String TR064::md5String(const String& text) {
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
String TR064::byte2hex(byte number) {
    String Hstring = String(number, HEX);
    if (number < 16) {Hstring = "0" + Hstring;}
    return Hstring;
}


/**************************************************************************/
/*!
    @brief  Extract the content of an XML element with a certain tag.
	          Case-insensitive matching.
			  Note that this function will consume the entire http stream.
    @param    params
                The array you want to fill.
				`params[i][0]` should contain the search key, `params[i][1]`
				  will be filled with the associated values.
    @param    nParam
                The number of XML tags you want to extract (length of params)
    @return success state.
*/
/**************************************************************************/
bool TR064::xmlTakeParams(String (*params)[2], int nParam) {
    tr064Client->Stream::setTimeout(100);
	int foundParam = 0;
    while (tr064Client->connected()) {
        if (!http.connected()) {
            deb_println("[TR064][xmlTakeParams] http connection lost", DEBUG_INFO);
            return false;
        }
        if (tr064Client->find("<")) {
            const String htmltag = tr064Client->readStringUntil('>');
            //deb_println("[TR064][xmlTakeParams] htmltag: " + htmltag, DEBUG_VERBOSE);
            const String value = tr064Client->readStringUntil('<');
            
            if (nParam > 0) {
                for (uint16_t i=0; i < nParam; ++i) {
                    if (htmltag.equalsIgnoreCase(params[i][0])) {
                        params[i][1] = value;
                        deb_println("[TR064][xmlTakeParams] found request parameter: " + params[i][0] + " => " + params[i][1], DEBUG_INFO);
						++foundParam;
                    }
                }
            }
            if (htmltag.equalsIgnoreCase("Nonce")) {
                _nonce = value;
                deb_println("[TR064][xmlTakeParams] Extracted the nonce '" + _nonce + "' from the last request.", DEBUG_INFO);
            }
            if (_realm == "" && htmltag.equalsIgnoreCase("Realm")) {
                _realm = value;
            }
            if (htmltag.equalsIgnoreCase("Status")) {
                _status = value;
                _status.toLowerCase();
                deb_println("[TR064][xmlTakeParams] Response status: " + _status , DEBUG_VERBOSE);
            }
            if (htmltag.equalsIgnoreCase("errorCode")) {
                deb_println("[TR064][xmlTakeParams] TR064 error, errorCode: '" + value  + "'", DEBUG_WARNING);
                deb_println("[TR064][xmlTakeParams] TR064 error, errorCodeMessage: '" + errorToString(value.toInt())  + "'", DEBUG_WARNING);
            }
            if (htmltag.equalsIgnoreCase("errorDescription")) {
                deb_println("[TR064][xmlTakeParams] TR064 error, errorDescription: " + value, DEBUG_WARNING);
            }
        } else {			
			http.end();
			return (foundParam >= nParam);
		}
    }
	http.end();
    return (foundParam >= nParam);
}


/**************************************************************************/
/*!
    @brief  Extract the content of a single XML element with a certain tag.
	          Case-insensitive matching.
			  Note that this function will consume the http stream (until
			   it find the desired tag).
    @param    value
                Pointer to a string to be filled with the content of the XML tag.
    @param    needParam
                The name of the XML tag, you want the content of.
    @return found value
*/
/**************************************************************************/
bool TR064::xmlTakeParam(String& value, const String& needParam) {
    tr064Client->Stream::setTimeout(500);
	
	int c;
	int ends = 0;
	while (true) {
		c = tr064Client->read();
		// check if the next character is the start of a tag
		if (c == '<') {
			// read the name of the tag
			String htmltag = tr064Client->readStringUntil('>');
			
			// check if the tag is one we're interested in
			if (htmltag.equalsIgnoreCase(needParam)) {
				// read the content of the tag
				value = tr064Client->readStringUntil('<');
				deb_println("[TR064][xmlTakeParam] Found tag: " + htmltag + " => " + value, DEBUG_VERBOSE);
				// Now forward the stream to the end of the (closing) tag
				tr064Client->readStringUntil('>');
				return true;
			}
		} else if (c == -1) {
			++ends;
			if (ends > 50) {
				deb_println("[TR064][xmlTakeParam] End of file reached without finding " + needParam, DEBUG_VERBOSE);
				return false;
			}
		}
	}
	return false;
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