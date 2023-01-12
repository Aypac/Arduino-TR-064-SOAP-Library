// Wifi network name (SSID) for the microcontroller to log into
// (of the router with the TR-064 interface)
#define WIFI_SSID		"WLANSID"
// Password of the same Wifi
#define WIFI_PASS		"XXXXXXXXXXXXXXXXXXXXX"

// Username for the TR-064 host (which is e.g. a router like the FRITZ!Box)
//  Some routers use a default of "admin".
#define TR_USER			"admin"

// Password for the TR-064 host.
//  If you did not create a seperate account,
//   this should be the same as you use on the web-login.
#define TR_PASS			"admin"

#define TR_PORT			49000

// The IP-adress of the TR-064 host. 
//   Often is 192.168.178.1, if it does not work, check the manual of your TR-064 host.
#define TR_IP			"192.168.178.1"