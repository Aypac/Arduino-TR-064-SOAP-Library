there is probably an error on loading the initial XML. as you can see the URL String has no Service "https://fritz.box:49443"
This should be : http://fritz.box:49000/upnp/control/x_voip

you can further Debug:

If you use wireshark you will probably see the correct httpResponse with the XML
https://www.it-techblog.de/fritzbox-und-wireshark-wlan-router-von-avm-monitoren-teil-1/11/2017/

you can debug HTTPClient as well. use HTTPClient in step 2

https://github.com/esp8266/Arduino/blob/master/doc/Troubleshooting/debugging.rst