#define SECRET_WIFI_SSID "WLANSID" // Wifi network name (SSID)

#define SECRET_WIFI_PASSWORD  "XXXXXXXXXXXXXXXXXXXXX" //Das Passwort Ihres WLANs

//Der Benutzername für Ihre Fritzbox. Wenn Sie keine separaten Benutzer angelegt haben, lautet dieser "admin".
#define SECRET_FUSER  "homechecker"

//Das Passwort für Ihre Fritzbox
#define SECRET_FPASS "this_shouldBEaDecentPassword!"

//Die IP-Adresse Ihrer Fritzbox. Ab Werk lautet diese 192.168.178.1.
//#define  SECRET_IP  "192.168.178.1"
#define  SECRET_IP  "fritz.box"

const char *myfritzbox_root_ca =
"-----BEGIN CERTIFICATE-----\n"
"MIID2DCCAsCgAwIBAgIJAOReByhZW+7gMA0GCSqGSIb3DQEBCwUAMCcxJTAjBgNV\n"
"BAMTHGhzbHk5eHh3ODd2bWt5YncubXlmcml0ei5uZXQwHhcNMTkxMDI4MTAyMzE1\n"
"WhcNMzgwMTE1MTAyMzE1WjAnMSUwIwYDVQQDExxoc2x5OXh4dzg3dm1reWJ3Lm15\n"
"ZnJpdHoubmV0MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuCbT+qAv\n"
"scEcYZco6Gl9SHzinr3VCLsTCkibcuGt/FdsLLCcdGHfLb2NX9mMBF+BwYRoFuXt\n"
"HVx6O5FrlvIHDECw+uQ+vfzFMbpm6b1lvgce/8rll/NgCTirwDW6wS9iy7CtAlSm\n"
"+nqZdoVqZvcWInrckn7n7p/ZdCM2U2hQ1cNZkJtxXvc/aKL9Lutj28J0J6XxJDFC\n"
"viPKENz6+fd+B5dwhmbfRPABgPTS/mqb2vCzwNNtvhnkvPRskqc6QSckdm3/HIop\n"
"iKQP/Ao1lnB4V/tDOf7VhTvVok6pA1D2ccJA/HNAYCvw9/fFoKtAxbnVFXI0+Bls\n"
"UifYdnCcUkqtDwIDAQABo4IBBTCCAQEwHQYDVR0OBBYEFMnzFscyTefQr+dtqMmW\n"
"vAWi/GfxMFcGA1UdIwRQME6AFMnzFscyTefQr+dtqMmWvAWi/GfxoSukKTAnMSUw\n"
"IwYDVQQDExxoc2x5OXh4dzg3dm1reWJ3Lm15ZnJpdHoubmV0ggkA5F4HKFlb7uAw\n"
"DAYDVR0TBAUwAwEB/zB5BgNVHREEcjBwghxoc2x5OXh4dzg3dm1reWJ3Lm15ZnJp\n"
"dHoubmV0gglmcml0ei5ib3iCDXd3dy5mcml0ei5ib3iCC215ZnJpdHouYm94gg93\n"
"d3cubXlmcml0ei5ib3iCCWZyaXR6Lm5hc4INd3d3LmZyaXR6Lm5hczANBgkqhkiG\n"
"9w0BAQsFAAOCAQEAk8nOxqt1SVEK+N9hT3whwt94shwepQFi0k+oBt3QUpm8Z1OV\n"
"ipQ4ERUSicVGnHTEBXzxbUaEXuyTAYmaKBnyErR6GjNmp5YNPvlIPBJVku/p8412\n"
"Y2Thn3YLhqpPG4HIkhD0E+tIh98WZbgwtQc7horRPqkaIBaBdRzi0pHJplRQeXPM\n"
"knj/XioZvpnd3eMsocHBaAOOjzAOToVjFz9yS4woGaVVYFqYnj6KeJ0JOT9aehjv\n"
"+Zr7KKh3XDhhBF43/TncYKPqm5uOLHlITivzQ8BTH0pPUujQwa0j+szGftuBjjHw\n"
"xMX1RtE24A1Pi28qtRu/DbA1nbsj4gy4ymh4vA==\n"
"-----END CERTIFICATE-----";