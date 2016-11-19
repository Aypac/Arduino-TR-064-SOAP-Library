/*
  tr064.h - Library for communicating via TR-064 protocol
  (e.g. Fritz!Box)
  A descriptor of the protocol can be found here: https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AVM_TR-064_first_steps.pdf
  The latest Version of this library can be found here: http://github.com/Aypac
  Created by René Vollmer, November 2016
*/


#ifndef tr064_h
#define tr064_h

//#define USE_SERIAL Serial
#include "Arduino.h"
#include <ESP8266HTTPClient.h>

#define arr_len( x )  ( sizeof( x ) / sizeof( *x ) )

class TR064
{
  public:
    TR064(int port, String ip, String user, String pass);
    void init();
    String action(String service, String act);
    String action(String service, String act, String params[][2], int nParam);
    String action(String service, String act, String params[][2], int nParam, String (*req)[2], int nReq);
    String xmlTakeParam(String inStr,String needParam);
    String md5String(String s);
    String byte2hex(byte number);
  private:
    void initServiceURLs();
    void initNonce();
    String httpRequest(String url, String xml, String action);
    String generateAuthToken();
    String generateAuthXML();
    String findServiceURL(String service);
    String xmlTakeParami(String inStr,String needParam);
    String _ip;
    int _port;
    String _user;
    String _pass;
    String _realm; //To be requested from the router
    String _secretH; //to be generated
    String _nonce = "";
    const String _requestStart = "<?xml version=\"1.0\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">";
    const String _detectPage = "/tr64desc.xml";
    String _services[100][2];
};

#endif
