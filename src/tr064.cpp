/*!
 * @file tr064.cpp
 *
 * @mainpage Library for communicating via TR-064 protocol (e.g. Fritz!Box)
 *
 * @section intro_sec Introduction
 * 
 * This library allows for easy communication of TR-064 (and possibly TR-069) enabled devices,
 * such as Routers, smartplugs, DECT telephones etc.
 * Details, examples and the latest Version of this library can be found <a href='https://github.com/Aypac/Arduino-TR-064-SOAP-Library'>on my Github page</a>.
 * A descriptor of the protocol can be found <a href='https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AVM_TR-064_first_steps.pdf'>here</a>.
 * 
 * Last updated: May 2019
 *
 * @section dependencies Dependencies
 *
 * This library depends on:
 *	MD5Builder
 *  ESP8266HTTPClient or HTTPClient, depending on the intended platform (ESP8266 or ESP32).
 *
 * @section author Author
 *
 * Written by René Vollmer "Aypac" in November 2016.
 *
 * @section license License
 *
 * MIT License, all text here must be included in any redistribution.
 *
 */

#include "tr064.h"



/**************************************************************************/
/*! 
    @brief  Library to easily make TR-064 calls. Do not construct this unless you have a working connection to the device!
*/
/**************************************************************************/
TR064::TR064(int port, String ip, String user, String pass) {
	_port = port;
	_ip = ip;
	_user = user;
	_pass = pass;
}

/**************************************************************************/
/*!
    @brief  Initializes the library. Needs to be explicitly called.
			There should already be a working connection to the device.
    @return void
*/
/**************************************************************************/
void TR064::init() {
	delay(100); // TODO: REMOVE (after testing, that it still works!)
	// Get a list of all services and the associated urls
	initServiceURLs();
	// Get the initial nonce and the realm
	initNonce();
	// Now we have everything to generate our hashed secret.
	if(Serial) Serial.println("Your secret is is: " + _user + ":" + _realm + ":" + _pass);
	_secretH = md5String(_user + ":" + _realm + ":" + _pass);
	if(Serial) Serial.println("Your secret is hashed: " + _secretH);
}

/**************************************************************************/
/*!
    @brief  Fetches a list of all services and the associated URLs for internal use.
    @return void
*/
/**************************************************************************/
void TR064::initServiceURLs() {
	/* TODO: We should give access to this data for users to inspect the
	 * possibilities of their device(s) - see #9 on Github.
	 */
	String inStr = httpRequest(_detectPage, "", "");
	int CountChar=7; //length of word "service"
	int i = 0;
	while (inStr.indexOf("<service>") > 0 || inStr.indexOf("</service>") > 0) {
		int indexStart=inStr.indexOf("<service>");
		int indexStop= inStr.indexOf("</service>");
		String serviceXML = inStr.substring(indexStart+CountChar+2, indexStop);
		String servicename = xmlTakeParam(serviceXML, "serviceType");
		String controlurl = xmlTakeParam(serviceXML, "controlURL");
		_services[i][0] = servicename;
		_services[i][1] = controlurl;
		++i;
		if(Serial) {
			Serial.printf("Service no %d:\t", i);
			Serial.flush();
			Serial.println(servicename + " @ " + controlurl);
		}
		inStr = inStr.substring(indexStop+CountChar+3);
	}
}

/**************************************************************************/
/*!
    @brief  Fetches the initial nonce and the realm for internal use.
    @return void
*/
/**************************************************************************/
void TR064::initNonce() {
	if(Serial) Serial.print("Geting the initial nonce and realm\n");
	// TODO: Is this request supported by all devices or should we use a different one here?
	String a[][2] = {{"NewAssociatedDeviceIndex", "1"}};
	action("urn:dslforum-org:service:WLANConfiguration:1", "GetGenericAssociatedDeviceInfo", a, 1);
	if(Serial) Serial.print("Got the initial nonce: " + _nonce + " and the realm: " + _realm + "\n");
}

//
/**************************************************************************/
/*!
    @brief  Generates and returns the XML-header for authentification.
    @return The XML-header as string.
*/
/**************************************************************************/
String TR064::generateAuthXML() {
	String token;
	if (_nonce == "" || _error) {
		// If we do not have a nonce yet, we need to use a different header
		token="<s:Header><h:InitChallenge xmlns:h=\"http://soap-authentication.org/digest/2001/10/\" s:mustUnderstand=\"1\"><UserID>"+_user+"</UserID></h:InitChallenge ></s:Header>";
	} else {
		// Otherwise we produce an authorisation header
		token = generateAuthToken();
		token = "<s:Header><h:ClientAuth xmlns:h=\"http://soap-authentication.org/digest/2001/10/\" s:mustUnderstand=\"1\"><Nonce>" + _nonce + "</Nonce><Auth>" + token + "</Auth><UserID>"+_user+"</UserID><Realm>"+_realm+"</Realm></h:ClientAuth></s:Header>";
	}
	return token;
}

/**************************************************************************/
/*!
    @brief  Returns the authentification token based on the hashed secret and the last nonce.
    @return The authentification token as string.
*/
/**************************************************************************/
String TR064::generateAuthToken() {
	String token = md5String(_secretH + ":" + _nonce);
	if(Serial) Serial.print("The auth token is " + token + "\n");
	return token;
}


/**************************************************************************/
/*!
    @brief  This function will call an action on the service of the device.
	        In order to understand how to construct such a call, please
			consult <a href='https://github.com/Aypac/Arduino-TR-064-SOAP-Library'>the Github page</a>.
    @param    service
              The name of the service you want to adress.
    @param    act
              The action you want to perform on the service.
    @return The response from the device.
*/
/**************************************************************************/
String TR064::action(String service, String act) {
	if(Serial) Serial.println("action_2");
	String p[][2] = {{}};
	return action(service, act, p, 0);
}


/**************************************************************************/
/*!
    @brief  This function will call an action on the service of the device
			with certain parameters.
	        In order to understand how to construct such a call, please
			consult <a href='https://github.com/Aypac/Arduino-TR-064-SOAP-Library'>the Github page</a>.
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
    if(Serial) Serial.println("action_1");

	// Generate the XML-envelop
    String xml = _requestStart + generateAuthXML() + "<s:Body><u:"+act+" xmlns:u='" + service + "'>";
	// Add request-parameters to XML
    if (nParam > 0) {
        for (int i=0;i<nParam;++i) {
			if (params[i][0] != "") {
                xml += "<"+params[i][0]+">"+params[i][1]+"</"+params[i][0]+">";
            }
        }
    }
	// Close the envelop
    xml += "</u:" + act + "></s:Body></s:Envelope>";
	// The SOAPACTION-header is in the format service#action
    String soapaction = service+"#"+act;
	
	
	// Reset error status.
	_error=false;
	
	// Send the http-Request
    String xmlR = httpRequest(findServiceURL(service), xml, soapaction);

	// Extract the Nonce for the next action/authToken.
    if (xmlR != "") {
		if (xmlTakeParam(xmlR, "Nonce") != "") {
			_nonce = xmlTakeParam(xmlR, "Nonce");
		}
		if (_realm == "" && xmlTakeParam(xmlR, "Realm") != "") {
			_realm = xmlTakeParam(xmlR, "Realm");
		}
    }
    return xmlR;
}


/**************************************************************************/
/*!
    @brief  This function will call an action on the service of the device
			with certain parameters and return values.
			It will fill the array `req` with the values of the assiciated
			return variables of the request.
	        In order to understand how to construct such a call, please
			consult <a href='https://github.com/Aypac/Arduino-TR-064-SOAP-Library'>the Github page</a>.
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
    if(Serial) Serial.println("action_3");
    String xmlR = action(service, act, params, nParam);
    String body = xmlTakeParam(xmlR, "s:Body");

    if (nReq > 0) {
        for (int i=0;i<nReq;++i) {
            if (req[i][0] != "") {
                req[i][1] = xmlTakeParam(body, req[i][0]);
            }
        }
    }
    return xmlR;
}

// 

/**************************************************************************/
/*!
    @brief  Helperfunction, which returns the (relative) URL for a service.
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
			  should the request be repeated with a new nonce, if it fails?
    @return The response from the device.
*/
/**************************************************************************/
String TR064::httpRequest(String url, String xml, String soapaction, bool retry) {
	HTTPClient http;

    if(Serial) Serial.print("[HTTP] begin: "+_ip+":"+_port+url+"\n");
    
    http.begin(_ip, _port, url);
    if (soapaction != "") {
		http.addHeader("CONTENT-TYPE", "text/xml"); //; charset=\"utf-8\"
		http.addHeader("SOAPACTION", soapaction);
    }
    //http.setAuthorization(fuser.c_str(), fpass.c_str());


    // start connection and send HTTP header
    int httpCode=0;
    if (xml != "") {
		if(Serial) Serial.println("\n\n\n"+xml+"\n\n\n");
		httpCode = http.POST(xml);
		if(Serial) Serial.print("[HTTP] POST... SOAPACTION: "+soapaction+"\n");
    } else {
		httpCode = http.GET();
		if(Serial) Serial.print("[HTTP] GET...\n");
    }

    
    String payload = "";
    // httpCode will be negative on error
    if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        if(Serial) Serial.printf("[HTTP] POST... code: %d\n", httpCode);

        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            payload = http.getString();
        }
    }
	String status = xmlTakeParam('Status', payload).toLowerCase();
	Serial.printf("[HTTP] status: "+status);
	if (httpCode <= 0 or status == "unauthenticated") {
		// Error
		// TODO: Proper error-handling? See also #12 on github
		if(Serial) Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
		if (retry) {
			_nonce = "";
			return httpRequest(url, xml, soapaction, false);
			if(Serial) Serial.printf("[HTTP] Trying again.");
		} else {
			if(Serial) Serial.printf("[HTTP] Giving up.");
			_error=true;
		}
    }
	
	// TODO: only print this if high debug priority
    if(Serial) Serial.println("\n\n\n"+payload+"\n\n\n");
    http.end();
    return payload;
}

// ----------------------------
// ----- Helper-functions -----
// ----------------------------

/**************************************************************************/
/*!
    @brief  Translates a String into it's MD5 hash.
    @param  text
			input string of which to calculcate the md5 hash.
    @return String containing the MD5 hash
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
	for ( byte i = 0; i < 16; i++) hash += byte2hex(bbuff[i]);
	return hash;   
}

/**************************************************************************/
/*!
    @brief  Translates a byte number into a hex number.
    @param    number
              byte number
    @return hex number
*/
/**************************************************************************/
String TR064::byte2hex(byte number){
	String Hstring = String(number, HEX);
	if (number < 16){Hstring = "0" + Hstring;}
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
			  The XML tag, you want the content of.
    @return The content of the requested XML tag.
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
			  The XML tag, you want the content of.
    @return The content of the requested XML tag.
*/
/**************************************************************************/
String TR064::xmlTakeSensitiveParam(String inStr, String needParam) {
	return _xmlTakeParam(inStr, needParam);
}

/**************************************************************************/
/*!
    @brief  Extract the content of an XML element with a certain tag, with
			case-insensitive matching.
			Not recommend to use directly/as default, since XML is case-sensitive
			by definition/specification, this is just made to be used as backup,
			if the case-sensitive method failed.
    @param    inStr
              The XML from which to extract.
    @param    needParam
			  The XML tag, you want the content of.
    @return The content of the requested XML tag.
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
    @brief  Underlying function to extract the content of an XML element with
	a certain tag.
    @param    inStr
              The XML from which to extract.
    @param    needParam
			  The XML tag, you want the content of.
    @return The content of the requested XML tag.
*/
/**************************************************************************/
String TR064::_xmlTakeParam(String inStr, String needParam) {
   int indexStart = inStr.indexOf("<"+needParam+">");
   int indexStop = inStr.indexOf("</"+needParam+">");  
   if (indexStart > 0 || indexStop > 0) {
		int CountChar=needParam.length();
		return inStr.substring(indexStart+CountChar+2, indexStop);
   }
	//TODO: Proper error-handling? See also #12 on github
	return "";
}

