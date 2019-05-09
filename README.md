# Arduino-TR-064-SOAP-Library [![Build Status](https://travis-ci.com/Aypac/Arduino-TR-064-SOAP-Library.svg?branch=master)](https://travis-ci.com/Aypac/Arduino-TR-064-SOAP-Library)
Arduino library for the TR-064 protocol, most commonly used by Fritz!Box. [Definition of the Protocol](https://www.broadband-forum.org/technical/download/TR-064.pdf) and [Description of the protocol](https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AVM_TR-064_first_steps.pdf).

Has been developed on an ESP8266 and tested on an ESP32.

For illustration and examples, please also visit the [Wiki](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/wiki), my [instructable](http://www.instructables.com/id/Who-Is-Home-Indicator-aka-Weasley-Clock-Based-on-T/) and of course the [examples folder](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/tree/master/examples).

If you need further guidance, please check out the [Wiki](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/wiki) if that doesn't help, please don't hesistate to [open an issue here on Github](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues/new). Please create a pull request (or, if you don't know how, an issue) if you made something cool with this library and want to provide it as an example.

## Known routers with TR-064 interface
If you know more, please let me know!

<ul>
<li> Almost all Fritz!Box routers. <details><summary>Tested devices (click to expand)</summary><ul>
 <li> Fritz!Box FON WLAN 7360</li>
 <li> Fritz!Box 7490 (tested by Dirk Kaben)</li>
 <li> Fritz!Box 7580</li>
 <li> Fritz!Box 7590</li>
 <li> FRITZ!DECT200 (tested by Oliver-André Urban)</li>
 <li> FRITZ!DECT 210 (test by Thorsten Godau)</li>
 </ul>
</details></li>
<li> Some ZyXEL routers (eg. <a href="ftp://ftp.zyxel.nl/VMG4325-B10A/user_guide/VMG4325-B10A_.pdf">VMG4325 and VMG4380, see page 29</a>)</li>
 </ul>

## Press
 
 - The German magazine C'T, published [an article on how to create a doorbell using the DECT phones and an ESP8266 (in German)](https://www.heise.de/select/ct/2018/17/1534215254552977).
 - A similar article was also featured in Reichelt magazine (an electronics vendor) in [English](https://www.reichelt.com/magazin/en/build-smart-doorbell-arduino), [German](https://www.reichelt.de/magazin/how-to/smarte-tuerklingel) and [Dutch](https://www.reichelt.com/magazin/nl/zelf-een-slimme-deurbel-maken).
 

# TODO

I moved the todo's mostly into the [issue section here on Github](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues).

However, let me note, that help is very welcome (especially for improving the library code). I'm not particularly good with C++. Also if you use the library to do something cool, please let me know - I'd love to add more examples to the Repo!

Some of the old issues are:

## Library

* Add yield()'s
* figure out the occasional crashes
* Test for different setups (i.e. other microcontroller and other routers)
* Get a new Nonce/refresh if a connection fails
* Make code more efficient
* Do some (better) error-handeling
  * Return proper errors
  * Re-initialize authorization to have a fresh Nonce
* Add some more comments
* Create a sophisticated debug/verbose mode (and silence the non-debug mode!)

## Examples

* Add yield()'s
* Add a few more nice examples
  * turning on/off (guest) wifi
  * read out and display connection speed
  * alternate WPS button
  * Emergency dial-up to relatives or emergency services
  * recognize when devices (like washing machines) are done (along the lines of [this](https://github.com/dl9sec/ArduinoSIP/tree/master/examples/LaundryNotifier)
  * Start devices throught e.g. telegram messages (e.g. coffee machine, washing machine, ...)
* WhoIsHomeIndicator
  * Add nice GUI/Website to configure/manage the users/LEDs/MACs/known devices
  * Known Bug: After a few hours the known devices "freeze", needs restart
* Extend doorbell example to also send telegram messages :)
