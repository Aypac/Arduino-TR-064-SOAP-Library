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


TR064::TR064() {
    _state = TR064_STATUS_INIT;
}
/**************************************************************************/
/*! 
    @brief  Library/Class to easily make TR-064 calls.
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
    _state = TR064_STATUS_INIT;
    debug_level = DEBUG_WARNING;    
    this->setServer(port, ip, user, pass, protocol, certificate);
}


/**************************************************************************/
/*!
    @brief  Initializes the library. Needs to be explicitly called.
            There should already be a working connection to the network
            ith the target device.
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
    Serial.println("Constructor!");
    this->_ip = ip;
    this->_port = port;
    this->_user = user;
    this->_pass = pass;
    _certificate = certificate;
    _protocol = protocol;
	useStreamingForXML = true;
    
    if (protocol == Protocol::useHttp) {
        wifiClient = &wifiSimpleClient;
         deb_println("[TR064][setServer] use Protokol http ", DEBUG_ERROR);
    } else {
        deb_println("[TR064][setServer] use Protokol https ", DEBUG_ERROR);
        if (protocol == Protocol::useHttpsInsec) {
            wifiSSLClient.setInsecure();
        } else {
            #if defined(ESP32)
                wifiSSLClient.setCACert(certificate);
            #endif
        }
        wifiClient = &wifiSSLClient;
    }
    _state = TR064_STATUS_SETTINGS;
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
    
    if (_state <= TR064_STATUS_INIT) {
        deb_println("[TR064][initServiceURLs]<error> Could not load service URLs, because setServer was not called before.", DEBUG_ERROR);
        return;
    }
    _state = TR064_STATUS_SETTINGS;
    deb_println("[TR064][initServiceURLs] getting the service detect page", DEBUG_INFO);
	bool reqSuccess = httpRequest(_detectPage, "", "", true);
    int i = 0;
    if (reqSuccess) {
        
        // Scan the XML stream for <serviceType> followed by <controlURL> XML tags.
        // Fill their content into the _service Map (dict).
        // Continue until you can't find any new <serviceType> XML tags.
        deb_println("[TR064][initServiceURLs] Searching the XML for serviceType", DEBUG_VERBOSE);
		
		if (useStreamingForXML) {
			// new method, only keeping small bits in memory.
			// This is more compatible with newer versions of the http and wifiClient libraries
           while (true) {
				String key;
				if (xmlTakeParamStream(key, "serviceType")) {
					if (key != "") {
						String controlurl;
                        if (xmlTakeParamStream(controlurl, "controlURL") ) {
							deb_println("[TR064][initServiceURLs] "+ String(i) + "\t" + key + " => " + controlurl, DEBUG_INFO);
							_services[key] = controlurl;
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
		} else {
			// old method, works better on some MCUs and older versions of the http library
			
			String inStr = http.getString();
			Serial.println(String(inStr.indexOf("<root")) + " - "+ String(inStr.indexOf("</root")));
			const int CountChar = 7; //length of word "service"
			
			int indexStart = inStr.indexOf("<service>");
			int indexStop = inStr.indexOf("</service>");
			Serial.println(String(indexStart) + " - "+ String(indexStop));
			while (indexStart >= 0 and indexStop > 0) {
				String serviceXML = inStr.substring(indexStart+CountChar+2, indexStop);
				String servicename = xmlTakeParamFull(serviceXML, "serviceType");
				String controlurl = xmlTakeParamFull(serviceXML, "controlURL");
				_services[servicename] = controlurl;
				inStr.remove(0, (indexStop+CountChar+2));
				++i;
				deb_println("[TR064][initServiceURLs] "+ String(i) + "\t" + servicename + " => " + controlurl, DEBUG_INFO);
				indexStart = inStr.indexOf("<service>");
				indexStop = inStr.indexOf("</service>");
			}
		}
    }
    http.end();
	if (reqSuccess and i == 0) {
		deb_println("[TR064][initServiceURLs] No services found. Try to use the opposite useStreamingForXML flag.", DEBUG_INFO);
	}
	if (!reqSuccess or i == 0) {
        deb_println("[TR064][initServiceURLs]<error> initServiceUrls failed.", DEBUG_ERROR);
    } else {
        deb_println("[TR064][initServiceURLs] Found "+ String(i) + " services in total.", DEBUG_INFO);
		_state = TR064_STATUS_SERVICES;
	}
}


/**************************************************************************/
/*!
    @brief  Generates and returns the XML-header for authentification.
    @return The XML-header (as `String`).
*/
/**************************************************************************/
String TR064::generateAuthXML() {
    String token;
    if (_nonce == "" or _realm == "") {
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
    deb_println("[TR064][generateAuthToken] Your secret is '" + secr + "'", DEBUG_INFO);
    _secretH = md5String(secr);
    deb_println("[TR064][generateAuthToken] Your hashed secret is '" + _secretH + "'", DEBUG_INFO);
    
    String token = md5String(_secretH + ":" + _nonce);
    deb_println("[TR064][generateAuthToken] With nonce "+ _nonce +", the auth token is '" + token + "'", DEBUG_INFO);
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
    deb_println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~[TR064][action]~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~", DEBUG_INFO);
    deb_println("[TR064][action] with extraction", DEBUG_VERBOSE);
    
    bool retry = false;
    
    // Do the action. Get the results.
    
    // It can sometimes happen that we have no or an old nonce, in which case
    //  we will want to re-try at least up to two times.
    // Attempt 1 fails because of no/old nonce, extracts Nonce
    if (action_raw(service, act, params, nParam, url)) {
		if (useStreamingForXML) {
			retry = !xmlTakeParamsStream(req, nReq);
		} else {
			retry = !xmlTakeParamsFull(req, nReq);
		}
        if (!retry) {
            deb_println("[TR064][action] return parameters sucessful.", DEBUG_VERBOSE);
            retry = (_status == "unauthenticated");
        }
        deb_println("[TR064][action] Response status: "+ _status, DEBUG_INFO);
		
        if (retry) {
            if (_nonce == "" or _realm == "") {
                deb_println("[TR064][action] Error, request failed. Info:", DEBUG_ERROR);
                deb_println("     First attempt failed, but no nonce/realm found, this should not happen. Please report this.", DEBUG_ERROR);
                return false;
            }
            
            // Attempt 2 should always work;
            // From the previous (unauthenticated) request, we should now have a Nonce and realm,
            //   so we should be able to execute the action now
            if (action_raw(service, act, params, nParam, url)) {
                // Success.
				if (useStreamingForXML) {
					bool b = xmlTakeParamsStream(req, nReq);
					http.end();
					return b;
				} else {
					bool b = xmlTakeParamsFull(req, nReq);
					http.end();
					return b;
				}
            } else {
                deb_println("[TR064][action] Error, request failed ", DEBUG_ERROR);
				http.end();
                return false;
            }
        }
        // Success.
        return true;
    }
	http.end();
    deb_println("[TR064][action]<error> Request failed ", DEBUG_ERROR);
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
                The serviceURL you want to call.
    @param    protocol
                Transmission protocol to be used (http/https).
    @return success state.
*/
/**************************************************************************/
bool TR064::action_raw(const String& service, const String& act, String params[][2],
                       int nParam, const String& url) {
    
    if (service.startsWith(_servicePrefixUPnP)) {
        // The SOAPACTION-header is in the format service#action
        return action_UPnP_raw(service, act, params,nParam,url);
    }
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
    
    // Empty nonce; this ensures that we request a new one explicitly,
    //   if we don't get a new one with this request.
    _nonce = "";
    
    // Send the http-Request
    if (url != "") {
        return httpRequest(url, xml, soapaction, true);
    } else {
        if (_state >= TR064_STATUS_SERVICES) {
            return httpRequest(_services[_servicePrefix + serviceName], xml, soapaction, true);
        } else {
            deb_println("[TR064][action_raw]<error> You need to init the library or specify a serviceURL!", DEBUG_ERROR);
	    return false;
        }
    }
    return false;
}
bool TR064::action_UPnP_raw(const String& service, const String& act, String params[][2],
                       int nParam, const String& url) {
    // Generate the XML-envelop
    String xml = _requestStart ;
    xml += "<s:Body><u:"+act+" xmlns:u=\"" + service + "\">";
    
    // Add request-parameters to XML
    if (nParam > 0) {
        for (uint16_t i=0; i<nParam; ++i) {
            if (params[i][0] != "") {
                deb_println("[TR064][action_UPnP_raw] Parameter: "+params[i][0]+" => "+params[i][1], DEBUG_VERBOSE);
                xml += "<"+params[i][0]+">"+params[i][1]+"</"+params[i][0]+">";
            }
        }
    }
    // Close the envelop
    xml += "</u:" + act + "></s:Body></s:Envelope>";
    
    // The SOAPACTION-header is in the format service#action
    String soapaction = service+"#"+act;
    
    // Empty nonce; this ensures that we request a new one explicitly,
    //   if we don't get a new one with this request.
    _nonce = "";
    
    // Send the http-Request
    if (url != "") {
        return httpRequest(url, xml, soapaction, true);
    } else {
        if (_state >= TR064_STATUS_SERVICES) {
            return httpRequest(service, xml, soapaction, true);
        } else {
            deb_println("[TR064][action_UPnP_raw]<error> You need to init the library or specify a serviceURL!", DEBUG_ERROR);
	    return false;
        }
    }
    return false;
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
bool TR064::httpRequest(const String& url, const String& xml,
                        const String& soapaction, bool retry) {
    if (url == "") {
        deb_println("[TR064][httpRequest] URL is empty, abort http request.", DEBUG_ERROR);
        return false;
    }
	
    bool nossl = bool(_protocol == Protocol::useHttp);
	String fullurl = "";
	if (nossl) {
		fullurl = "http://";
	} else {
		fullurl = "https://";
	}
	fullurl = fullurl + _ip.c_str() + ":" + String(_port) + url.c_str();
	
    deb_println("[TR064][httpRequest] prepare request to URL: " + fullurl, DEBUG_INFO);
	
    http.setReuse(true);
    //http.useHTTP10(true);
    
	if (useStreamingForXML) {
		http.begin(*wifiClient, _ip.c_str(), _port, url.c_str(), nossl);
	} else {
		http.begin(*wifiClient, fullurl);
	}
    
    deb_println("[TR064][httpRequest] wifiClient->Stream::getTimeout: " +  String(wifiClient->Stream::getTimeout()), DEBUG_INFO);

    // http.setConnectTimeout not defined on ESP8266...
    #if defined(ESP32)
        http.setConnectTimeout(2000);
    #endif
        wifiClient->Stream::setTimeout(500);
    
    if (soapaction != "") {
		http.addHeader("CONTENT-TYPE", "text/xml");
        http.addHeader("SOAPACTION", soapaction);
    }
    
    // Place http request
    int httpCode = 0;
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
        if (httpCode == HTTP_CODE_OK) {
            return true;
        }else{
            String req[][2] = {{"errorCode",""},{"errorDescription",""}};
            if (xmlTakeParamsStream(req, 2)) {                                
                if(req[0][1]!=""){
                    deb_println("[TR064][httpRequest] <TR064> Failed, errorCode: '" + req[0][1]  + "'", DEBUG_VERBOSE);                    
                    deb_println("[TR064][httpRequest] <TR064> Failed, message: '" + errorToString(req[0][1].toInt())  + "'", DEBUG_ERROR);
                    deb_println("[TR064][httpRequest] <Error> Failed, description: '" + req[1][1] + "'", DEBUG_VERBOSE);
                }                
            }
            return false;
        }
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
		http.end();
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
              Note that this function will load the entire response into memory.
    @param    params
                The array you want to fill.
                `params[i][0]` should contain the search key, `params[i][1]`
                  will be filled with the associated values.
    @param    nParam
                The number of XML tags you want to extract (length of params)
    @return success state.
*/
/**************************************************************************/
bool TR064::xmlTakeParamsFull(String (*params)[2], int nParam) {
	String xmlR = http.getString();
    String body = xmlTakeParamFull(xmlR, "s:Body");
    if (nParam > 0) {
        for (int i=0; i < nParam; ++i) {
			String value = xmlTakeParamFull(body, params[i][0]);
			params[i][1] = value;
        }
    }
	
	// Make sure to also extract the relevant parameters from the header
    String head = xmlTakeParamFull(xmlR, "s:Header");
	processGeneralXMLParam("Nonce", xmlTakeParamFull(head, "Nonce"));
	processGeneralXMLParam("Status", xmlTakeParamFull(head, "Status"));
	processGeneralXMLParam("errorCode", xmlTakeParamFull(head, "errorCode"));
	processGeneralXMLParam("errorDescription", xmlTakeParamFull(head, "errorDescription"));
	if (_realm == "") {
		processGeneralXMLParam("Realm", xmlTakeParamFull(head, "Realm"));
	}
    return true;
}

/*!
    @brief  Extract the content of an XML element with a certain tag.
              Case-insensitive matching.
    @param    xml
                XML text to be searched
    @param    needParam
                name of the tag to be looked for
    @return success state.
*/
/**************************************************************************/
String TR064::xmlTakeParamFull(String& xml, String needParam) {
	needParam.toLowerCase();
	String lxml = xml;
    lxml.toLowerCase();
    int indexStart = lxml.indexOf("<"+needParam+">");
    int indexStop = lxml.indexOf("</"+needParam+">");  
    if (indexStart > 0 || indexStop > 0) {
        int CountChar = needParam.length();
		String value = xml.substring(indexStart+CountChar+2, indexStop);
		deb_println("[TR064][xmlTakeParamsFull] found request parameter: " + needParam + " => " + value, DEBUG_INFO);
        return value;
    }
    //TODO: Proper error-handling? See also #12 on github
	deb_println("[TR064][xmlTakeParamsFull] did not find request parameter: " + needParam, DEBUG_WARNING);
    return "";    
}

/**************************************************************************/
/*!
    @brief  Extract the content of an XML element with a certain tag.
              Case-insensitive matching.
              Note that this function will consume the entire http stream->
    @param    params
                The array you want to fill.
                `params[i][0]` should contain the search key, `params[i][1]`
                  will be filled with the associated values.
    @param    nParam
                The number of XML tags you want to extract (length of params)
    @return success state.
*/
/**************************************************************************/
bool TR064::xmlTakeParamsStream(String (*params)[2], int nParam) {
    int foundParam = 0;
    WiFiClient *stream = http.getStreamPtr();    
    stream->Stream::setTimeout(40);
    
    while (stream->available()) {

        if (!http.connected()) {
            deb_println("[TR064][xmlTakeParamsStream] http connection lost", DEBUG_INFO);
            return false;
        }
        if (stream->find("<")) {
            const String htmltag = stream->readStringUntil('>');
           // deb_println("[TR064][xmlTakeParamsStream] htmltag: " + htmltag, DEBUG_VERBOSE);
            const String value = stream->readStringUntil('<');
            
			deb_println("[TR064][xmlTakeParamsStream]parameter: " + htmltag + " => " + value, DEBUG_INFO);
            // check if this is a tag we're interested in
			processGeneralXMLParam(htmltag, value);
            if (nParam > 0) {
                for (uint16_t i=0; i < nParam; ++i) {
                    if (htmltag.equalsIgnoreCase(params[i][0])) {
                        params[i][1] = value;
                        deb_println("[TR064][xmlTakeParamsStream] found request parameter: " + params[i][0] + " => " + params[i][1], DEBUG_INFO);
                        ++foundParam;
						// break; ///< might make it a bit faster, but not well-tested!
                    }
                }
            }
        } else {            
            return (foundParam >= nParam);
        }
    }    
    return (foundParam >= nParam);
}


/**************************************************************************/
/*!
    @brief  Extract the content of a single XML element with a certain tag.
              Case-insensitive matching.
              Note that this function will partially consume the http stream
			    (until it find the desired tag).
    @param    value
                Pointer to a string to be filled with the content of the XML tag.
    @param    needParam
                The name of the XML tag, you want the content of.
    @return found value
*/
/**************************************************************************/
bool TR064::xmlTakeParamStream(String& value, const String& needParam) {
    WiFiClient *stream = http.getStreamPtr();    
    stream->Stream::setTimeout(40);

    if(!stream->available()) {
            deb_println("[TR064][xmlTakeParamStream] stream not available", DEBUG_INFO);
            return false;                      
        }
    int c;
    int ends = 0;
    while (true) {
		c = stream->read();
		if (c != -1) {
			if (c != '<') {
				stream->readStringUntil('<');
			}
			
			// read the name of the tag
			String htmltag = stream->readStringUntil('>');
				
			if (htmltag.charAt(0) != '/') {
				// read the content of the tag
				//deb_println(htmltag, DEBUG_VERBOSE);
				
				// check if this is the tag we're interested in
				if (htmltag.equalsIgnoreCase(needParam)) {
					value = stream->readStringUntil('<');
					deb_println("[TR064][xmlTakeParamStream] Found tag: " + htmltag + " => " + value, DEBUG_VERBOSE);
					return true;
				}
				ends = 0;
			}
		} else {
			++ends;
			delay(50);
			if (ends > 10) {
				break;
			}
		}
    }
	
	deb_println("[TR064][xmlTakeParamStream] End of file reached without finding " + needParam, DEBUG_VERBOSE);
    return false;
}

void TR064::processGeneralXMLParam(const String& htmltag, const String& value) {
	if (htmltag.equalsIgnoreCase("Nonce")) {
		_nonce = value;
		deb_println("[TR064][processGeneralXMLParam] Extracted the nonce '" + _nonce + "' from the last request.", DEBUG_INFO);
	} else if (htmltag.equalsIgnoreCase("Status")) {
		_status = value;
		_status.toLowerCase();
		deb_println("[TR064][processGeneralXMLParam] Response status: " + _status , DEBUG_VERBOSE);
	} else if (htmltag.equalsIgnoreCase("errorCode")) {
		deb_println("[TR064][processGeneralXMLParam] TR064 error, errorCode: '" + value  + "'", DEBUG_WARNING);
		deb_println("[TR064][processGeneralXMLParam] TR064 error, errorCodeMessage: '" + errorToString(value.toInt())  + "'", DEBUG_WARNING);
	} else if (htmltag.equalsIgnoreCase("errorDescription")) {
		deb_println("[TR064][processGeneralXMLParam] TR064 error, errorDescription: " + value, DEBUG_WARNING);
	} else if (_realm == "" && htmltag.equalsIgnoreCase("Realm")) {
		_realm = value;
	}
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