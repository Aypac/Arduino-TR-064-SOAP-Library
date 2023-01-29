// Wifi network name (SSID) for the microcontroller to log into
// (of the router with the TR-064 interface)
#define WIFI_SSID     "WLANSID"
// Password of the same Wifi
#define WIFI_PASS     "XXXXXXXXXXXXXXXXXXXXX"

// Username for the TR-064 host (which is e.g. a router like the FRITZ!Box)
//  Some routers use a default of "admin".
#define TR_USER       "admin"

// Password for the TR-064 host.
//  If you did not create a seperate account,
//   this should be the same as you use on the web-login.
#define TR_PASS       "admin"


// The IP-adress of the TR-064 host. 
//   Often is 192.168.178.1, if it does not work, check the
//      manual of your TR-064 host.
#define TR_IP         "192.168.178.1"

// Shall we use http or https to access the TR-064 host?
// 0 = http, 1 = httpsInsec, 2 = https
// Please set your own TR_ROOT_CERT and TR_PORT accordingly
//   below if you chose 1 or 2!
#define TR_PROTOCOL   1
// Which port do we use for accessing the TR-064 host?
// Defaults for most FRITZ!Box'es:
//     49000 for http
//     49443 for htts
#define TR_PORT       49000
/******************************************************************
Setting the variable below is required if you want to use HTTPS
(as opposed to HTTP, please chose accordingly above) to access
your TR-064 host. Below is just a dummy example, it won't work
with your TR-064 host!

Example on how to get it from a FRITZ!Box:
  1) In the UI of the FRITZ!Box, click on category "Internet"
  2) Click on sub-category "Permit Access"
  3) Click on tab "FRITZ!Box Services"
  4) Press button "Download Certificate"
  5) Open the downloaded `boxcert.cer` file with a text-editor
  6) Copy it's contents below between the quotes
  7) Add backslashes (\) to the end of each line (as seen below)
******************************************************************/
#define TR_ROOT_CERT "-----BEGIN CERTIFICATE-----\
mIuEDzcCAvegAwIBAgIUPHgFuZAGY/8lL4AshsmtNB20OD0wDQYJKoZIhvcNAQEL\
BQAwJzElMCMGA1UEAxMcYWxnZXJpamUuaG9tZWxpbnV4c2VydmVyLm9yZzAeFw0y\
MzAxnTMxOTMyNDRaFw0zODAxMTUxOTMyNDRaMCcxJTAjBgNVBAMTHGFsZ2VyaWpl\
LmhvbWVsaW51eHNlCnZlci5vcmcwggEiMA0GCSqOSIb3DQEBAQUAA4IBDwAwggEK\
AoIaAQCRLbZKrRpx72pinim51N6qyfz4a9s7WlMfKBx7QZC2pEjw4IwrO/lWmVu1\
jN4I/esohQhMCko2vnOrc4gy64J4s7jjrdAEV1PL88k4YFHd589ecv6oKGlV3LcU\
LM19nEEDxUcd4lzBiGhsUJCS6kCphITjSMDUAg7G8YwphTNSTmqSIGZd4Ze3v51W\
Z75vtqSYhxV8xShud33pwPQ95irOjcs2owm/MJwW36BnS+Pfi/kHVIvvnPR87G7i\
F2dRIz1X/PzdMWnnpYEFJqutzUbQFT4dz6zLZJIgtl7VMxVCHx8p+mzQMJPX6S4u\
F8J2VPS18T0k1fkvJTk3pJUiYGB7Ag+BAAGjggExMIIBLTAdBgNVHQ4EFgQU8tOa\
YY/UrHlfrowfHnKnyY5Ut/wwYgYDVR0jBAswWYAU8tOaYY/UrHlfRowfHnKnyY5U\
t/yhK6QpMCcxJTAjBgN5BAMTHGFsZ2VyaWplLmhvbWVsaW51eHNlcnZlci5vcmeC\
FDx4BbmQBmP/JS+aLIbJrTQdtDg9MAwGA1UdEwQFMAMBAf8wgZkGA1UdEQSBkTCB\
joIcYWxnZXJpamUuaG9tZWxpbnV4c2VydmVyLm9yZ4IcMDFwb2Z3dGNibjY1ZjY1\
OC5teWZyaXR6Lm5ldIIJZnLpdsouYm94gg1ad3cuZnJpdHouYm94ggtteWZyaXR6\
LmJveIIPd3d3Lm15ZnJpdHouYm94gglmcml0ei5uYXOCDXd3dy5mcml0ei5uYXMw\
DQYJKoZIhvcNAQELBQADggEBAE2iwGljdxLuZW/BJ9noXWO0+tL2qOaYYewftDbB\
zuh9tAHRB8lyVqKh7F8EqVsSGtrVEhxldEnt7y15aI1u2o1klAFVQwoojUvqwqso\
BRc/t+YMnQ+6N2JOIdwP6/7VkTVes3XWxnoYtzGKDH9a16bGwTism60pKJCSCeCr\
VPmac688VopwxqR6/g/1xcfwxo5a1tua3OsVADAGFFaekuRyvsPvIVL1DPKMY4gV\
xKsTIndDleK8QrryIZSx+5311b4Kzw8UbyEcuYayjP7YmRZ5kv8NfTLKR/btEHka\
3HOVmhpb3xRjBLcgSmsvfd2ndH2TQqROLmvNOFjA9x072hc=\
-----END CERTIFICATE-----"
