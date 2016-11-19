/*
  tr064.h - Library for communicating via TR-064 protocol
  (e.g. Fritz!Box)
  A descriptor of the protocol can be found here: https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AVM_TR-064_first_steps.pdf
  The latest Version of this library can be found here: http://github.com/Aypac
  Created by René Vollmer, November 2016
*/

#include "tr064.h"

//Do not construct this unless you have a working connection to the device!
TR064::TR064(int port, String ip, String user, String pass)
{
  _port = port;
  _ip = ip;
  _user = user;
  _pass = pass;
}

//DONT FORGET TO INIT!
void TR064::init() {
  //USE_SERIAL.begin(115200); //TODO: REMOVE
  delay(1000); //TODO: REMOVE
	//Get a list of all services and the associated urls
  initServiceURLs();
	//Get the initial nonce and the realm
  initNonce();
	//Now we have everything to generate out hased secret.
  //USE_SERIAL.println("Your secret is is: " + _user + ":" + _realm + ":" + _pass);
  _secretH = md5String(_user + ":" + _realm + ":" + _pass);
  //USE_SERIAL.println("Your secret is hashed: " + _secretH);
}

//Fetches a list of all services and the associated urls
void TR064::initServiceURLs() {
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
       //USE_SERIAL.printf("Service no %d:\t", i);
       //USE_SERIAL.flush();
       //USE_SERIAL.println(servicename + " @ " + controlurl);
       inStr = inStr.substring(indexStop+CountChar+3);
    }
}

//Fetches the initial nonce and the realm
void TR064::initNonce() {
    //USE_SERIAL.print("Geting the initial nonce and realm\n");
    String a[][2] = {{"NewAssociatedDeviceIndex", "1"}};
    action("urn:dslforum-org:service:WLANConfiguration:1", "GetGenericAssociatedDeviceInfo", a, 1);
    //USE_SERIAL.print("Got the initial nonce: " + _nonce + " and the realm: " + _realm + "\n");
}

//Returns the xml-header for authentification
String TR064::generateAuthXML() {
    String token;
    if (_nonce == "") { //If we do not have a nonce yet, we need to use a different header
       token="<s:Header><h:InitChallenge xmlns:h=\"http://soap-authentication.org/digest/2001/10/\" s:mustUnderstand=\"1\"><UserID>"+_user+"</UserID></h:InitChallenge ></s:Header>";
    } else { //Otherwise we produce an authorisation header
      token = generateAuthToken();
      token = "<s:Header><h:ClientAuth xmlns:h=\"http://soap-authentication.org/digest/2001/10/\" s:mustUnderstand=\"1\"><Nonce>" + _nonce + "</Nonce><Auth>" + token + "</Auth><UserID>"+_user+"</UserID><Realm>"+_realm+"</Realm></h:ClientAuth></s:Header>";
    }
    return token;
}

//Returns the authentification token based on the hashed secret and the last nonce.
String TR064::generateAuthToken() {
    String token = md5String(_secretH + ":" + _nonce);
    //USE_SERIAL.print("The auth token is " + token + "\n");
    return token;
}


//This function will call an action on the service.
String TR064::action(String service, String act) {
    //USE_SERIAL.println("action_2");
    String p[][2] = {{}};
    return action(service, act, p, 0);
}

//This function will call an action on the service.
//With params you set the arguments for the action
//e.g. String params[][2] = {{ "arg1", "value1" }, { "arg2", "value2" }};
String TR064::action(String service, String act, String params[][2], int nParam) {
    //USE_SERIAL.println("action_1");
	//Generate the xml-envelop
    String xml = _requestStart + generateAuthXML() + "<s:Body><u:"+act+" xmlns:u='" + service + "'>";
	//add request-parameters to xml
    if (nParam > 0) {
        for (int i=0;i<nParam;++i) {
	    if (params[i][0] != "") {
                xml += "<"+params[i][0]+">"+params[i][1]+"</"+params[i][0]+">";
            }
        }
    }
	//close the envelop
    xml += "</u:" + act + "></s:Body></s:Envelope>";
	//The SOAPACTION-header is in the format service#action
    String soapaction = service+"#"+act;



	//Send the http-Request
    String xmlR = httpRequest(findServiceURL(service), xml, soapaction);


	//Extract the Nonce for the next action/authToken.
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


//This function will call an action on the service.
//With params you set the arguments for the action
//e.g. String params[][2] = {{ "arg1", "value1" }, { "arg2", "value2" }};
//Will also fill the array req with the values of the assiciated return variables of the request.
//e.g. String req[][2] = {{ "resp1", "" }, { "resp2", "" }};
//will be turned into req[][2] = {{ "resp1", "value1" }, { "resp2", "value2" }};
String TR064::action(String service, String act, String params[][2], int nParam, String (*req)[2], int nReq) {
    //USE_SERIAL.println("action_3");
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

//Returns the (relative) url for a service
String TR064::findServiceURL(String service) {
    for (int i=0;i<arr_len(_services);++i) {
	if (_services[i][0] == service) {
            return _services[i][1];
        }
    }
    return ""; //Service not found error! TODO: Proper error-handling?
}


//Puts a http-Request to the given url (relative to _ip on _port)
// - if specified POSTs xml and adds soapaction as header field.
// - otherwise just GETs the url
String TR064::httpRequest(String url, String xml, String soapaction) {
    HTTPClient http;

    //USE_SERIAL.print("[HTTP] begin: "+_ip+":"+_port+url+"\n");
    
    http.begin(_ip, _port, url);
    if (soapaction != "") {
      http.addHeader("CONTENT-TYPE", "text/xml"); //; charset=\"utf-8\"
      http.addHeader("SOAPACTION", soapaction);
    }
    //http.setAuthorization(fuser.c_str(), fpass.c_str());


    // start connection and send HTTP header
    int httpCode=0;
    if (xml != "") {
      //USE_SERIAL.println("\n\n\n"+xml+"\n\n\n");
      httpCode = http.POST(xml);
      //USE_SERIAL.print("[HTTP] POST... SOAPACTION: "+soapaction+"\n");
    } else {
      httpCode = http.GET();
      //USE_SERIAL.print("[HTTP] GET...\n");
    }

    
    String payload = "";
    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        //USE_SERIAL.printf("[HTTP] POST... code: %d\n", httpCode);

        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            payload = http.getString();
        }
    } else {
      //Error
	//TODO: Proper error-handling?
      //USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
      //TODO: _nonce="";
    }

    //USE_SERIAL.println("\n\n\n"+payload+"\n\n\n");
    http.end();
    return payload;
}



//----------------------------
//----- Helper-functions -----
//----------------------------




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

String TR064::byte2hex(byte number){
  String Hstring = String(number, HEX);
  if (number < 16){Hstring = "0" + Hstring;}
  return Hstring;
}


//Extract the content of an XML tag - case-insensitive
//Not recommend to use directly/as default, since XML is case-sensitive by definition
//Is made to be used as backup.
String TR064::xmlTakeParami(String inStr,String needParam) {
	//TODO: Give warning?
  needParam.toLowerCase();
  String instr = inStr;
  instr.toLowerCase();
  int indexStart = instr.indexOf("<"+needParam+">");
  int indexStop = instr.indexOf("</"+needParam+">");  
  if (indexStart > 0 || indexStop > 0) {
     int CountChar=needParam.length();
     return inStr.substring(indexStart+CountChar+2, indexStop);
  }
	//TODO: Proper error-handling?
  return "";
}


//Extract the content of an XML tag
//If you cannot find it case-sensitive, look case insensitive
String TR064::xmlTakeParam(String inStr, String needParam) {
   int indexStart = inStr.indexOf("<"+needParam+">");
   int indexStop = inStr.indexOf("</"+needParam+">");  
   if (indexStart > 0 || indexStop > 0) {
     int CountChar=needParam.length();
     return inStr.substring(indexStart+CountChar+2, indexStop);
   }
   //As backup
   return xmlTakeParami(inStr, needParam);
}
