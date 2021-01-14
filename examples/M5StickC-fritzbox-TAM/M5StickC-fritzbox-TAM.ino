/*
   M5StickC as simple ON/OFF switch for the TAM

   https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AHA-HTTP-Interface.pdf
   https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/x_tam.pdf

*/

#include <M5StickC.h>
#include <tr064.h>


const char* WIFI_SSID = "your SSID here";
const char* WIFI_PASS = "the wifi password";
const char* FRITZBOX_AIN = "admin"; //admin if no username is used
const char* FRITZBOX_PASS = "einsElfELF!!11";
const String FRITZBOX_DOMAIN = "fritz.box";
const int PORT = 49000;
const unsigned long UPDATE_INTERVAL = 10000UL; //Check TAM switch every 10 seconds

unsigned long lastConnection = 0;

TR064 connection(PORT, FRITZBOX_DOMAIN, FRITZBOX_AIN, FRITZBOX_PASS);

void setup() {
  M5.begin();
  pinMode(M5_BUTTON_HOME, INPUT);

  Serial.begin( 115200 );
  if (Serial) Serial.print("ESP Board MAC Address:  ");
  if (Serial) Serial.println(WiFi.macAddress());
  M5.Lcd.fillScreen(WHITE);

  M5.Lcd.setRotation(3);
  M5.Lcd.setCursor(2, 10);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.print(WiFi.macAddress());
  M5.Lcd.setTextSize(12);

  if (connectWiFi()) {
    if (Serial) Serial.println( "WiFi connection established" );
    M5.Lcd.fillScreen(ORANGE);
    M5.Lcd.setCursor(15, 15);
    M5.Lcd.print("---");
    delay(100);
    if (Serial) Serial.println("Initialize TR-064 connection");
    connection.debug_level = DEBUG_WARNING; //DEBUG_VERBOSE; //0: None, 1: Errors, 2: Warning, 3: Info, 4: Verbose
    connection.init();
  }
}

void loop() {
  M5.update();
  yield();
  connectWiFi();
  unsigned long int now = millis();
  if ( (now - lastConnection) > UPDATE_INTERVAL ) {
    if (Serial) Serial.print("Routine TAM check: ");
    if (Serial) Serial.println(refreshScreen(getABStatus() == 1) ? "ON" : "OFF");
    lastConnection = millis();
  }
  if (M5.BtnA.wasPressed()) {
    ButtonACallback();
  }
}


bool connectWiFi() {
  if ( WiFi.status() == WL_CONNECTED ) {
    return true;
  }
  else {
    WiFi.mode( WIFI_STA );
    WiFi.begin( WIFI_SSID, WIFI_PASS );

    if ( WiFi.waitForConnectResult() != WL_CONNECTED ) {
      if (Serial) Serial.println( "WiFi connection failed" );
      M5.Lcd.fillScreen(ORANGE);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(15, 15);
      M5.Lcd.print("ERR");
      return false;
    }
    return true;
  }
}

int getABStatus() {
  String params[][2] = {"NewIndex", "0"};
  int a = 1;
  String req[][2] = {"NewEnable", "0"};
  int b = 1;
  connection.action("urn:dslforum-org:service:X_AVM-DE_TAM:1", "GetInfo", params, a, req, b);
  return req[0][1].toInt();
}

bool switchABStatus() {
  int currentValue = getABStatus();
  int newValue = (currentValue == 1) ? 0 : 1;
  String params[][2] = {{"NewIndex", "0"}, {"NewEnable", String(newValue)}};
  int a = 2;
  connection.action("urn:dslforum-org:service:X_AVM-DE_TAM:1", "SetEnable", params, a);
  return (getABStatus() == newValue) ? true : false;
}

void ButtonACallback() {
  if (Serial) Serial.print("Switch TAM Status: ");
  if (Serial) Serial.println((switchABStatus()) ? "success" : "fail");
  refreshScreen(getABStatus());
  return;
}

int refreshScreen(int value) {
  if (value == 1) {
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.fillScreen(TFT_DARKGREEN);
    M5.Lcd.setCursor(15, 15);
    M5.Lcd.print("ON");
  }
  else {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.fillScreen(TFT_RED);
    M5.Lcd.setCursor(15, 15);
    M5.Lcd.print("OFF");
  }
  return value;
}
