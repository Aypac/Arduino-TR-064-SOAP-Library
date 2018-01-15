# Arduino-TR-064-SOAP-Library
Arduino library for the TR-064 protocol, most commonly used by Fritz!Box. A [Description of the protocol](https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AVM_TR-064_first_steps.pdf).

Has been developed on an ESP8266 and was tested with a Fritz!Box FON WLAN 7360.

For illustration on the example, please also visit my [instructable](http://www.instructables.com/id/Who-Is-Home-Indicator-aka-Weasley-Clock-Based-on-T/).

If you need further guidance, provide your own example or find a bug, please open an issue here on Github or head over to the [instructable](http://www.instructables.com/id/Who-Is-Home-Indicator-aka-Weasley-Clock-Based-on-T/) and ask there.

##Known routers with TR-064 interface
If you know more, please let me know!

* Almost all Fritz!Box routers. Tested devices:
  * Fritz!Box FON WLAN 7360
  * Fritz!Box 7490 (tested by Dirk Kaben)
  * DECT200 (tested by Oliver-André Urban)
* Some ZyXEL routers [eg. ftp://ftp.zyxel.nl/VMG4325-B10A/user_guide/VMG4325-B10A_.pdf page 29 ]


# TODO


Help is very welcome (especially for improving the library code). I'm not particularily good with C++. Also if you use the library to do something cool, please let me know - I'd love to add more examples to the Repo!

##Library

* Add yield()'s
* figure out the occasional crashes
* Test for different setups (i.e. other microcontroller and other routers)
* Get a new Nonce if a connection fails
* Make code more efficient
* Do some (better) error-handeling
  * Return proper errors
  * Re-initialize authorization to have a fresh Nonce
* Add some more commentary
* Create a sophisticated debug/verbose mode (and silence the non-debug mode!)
* Add a keywords.txt , as described here: https://www.arduino.cc/en/Hacking/LibraryTutorial

##Examples

* Add yield()'s
* Add a few more nice examples (eg turning on/off (guest) wifi, connection speed, WPS, etc)
* WhoIsHomeIndicator
  * Add nice GUI/Website to configure/manage the users/LEDs/MACs/known devices
  * Known Bug: After a few hours the known devices "freeze", needs restart
