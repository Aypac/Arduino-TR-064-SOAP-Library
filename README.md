# Arduino-TR-064-SOAP-Library [![Build Status](https://travis-ci.com/Aypac/Arduino-TR-064-SOAP-Library.svg?branch=master)](https://travis-ci.com/Aypac/Arduino-TR-064-SOAP-Library)
Arduino library for the TR-064 protocol, most commonly used by Fritz!Box. [Definition of the Protocol](https://www.broadband-forum.org/technical/download/TR-064.pdf) and [Description of the protocol](https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AVM_TR-064_first_steps.pdf).

Has been developed on an ESP8266 and tested on an ESP32.

For illustration on the example, please also visit the [Wiki](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/wiki) and my [instructable](http://www.instructables.com/id/Who-Is-Home-Indicator-aka-Weasley-Clock-Based-on-T/).

If you need further guidance, please check out the [Wiki](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/wiki) if that doens't help, please don't hesistate to [open an issue here on Github](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues/new). Please create a pull request (or, if you don't know how, an issue) if you made something cool with this library and wwant to provide it as an example.

## Known routers with TR-064 interface
If you know more, please let me know!

* Almost all Fritz!Box routers. Tested devices:
  * Fritz!Box FON WLAN 7360
  * Fritz!Box 7490 (tested by Dirk Kaben)
  * DECT200 (tested by Oliver-André Urban)
* Some ZyXEL routers (eg. <a href="ftp://ftp.zyxel.nl/VMG4325-B10A/user_guide/VMG4325-B10A_.pdf">VMG4325 and VMG4380, see page 29</a>)

## Made it into the C'T!
In a German magazine, the C'T, there was [an article (in German)](https://www.heise.de/select/ct/2018/17/1534215254552977) on how to create a doorbell using the DECT phones and an ESP8266.

# TODO


Help is very welcome (especially for improving the library code). I'm not particularily good with C++. Also if you use the library to do something cool, please let me know - I'd love to add more examples to the Repo!

## Library

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

## Examples

* Add yield()'s
* Add a few more nice examples (eg turning on/off (guest) wifi, connection speed, WPS, etc)
* WhoIsHomeIndicator
  * Add nice GUI/Website to configure/manage the users/LEDs/MACs/known devices
  * Known Bug: After a few hours the known devices "freeze", needs restart
