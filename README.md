# Arduino-TR-064-SOAP-Library [![Build Status](https://travis-ci.com/Aypac/Arduino-TR-064-SOAP-Library.svg?branch=master)](https://travis-ci.com/Aypac/Arduino-TR-064-SOAP-Library)
Arduino library to facilitate the use of the TR-064 protocol ([Definition](https://www.broadband-forum.org/technical/download/TR-064.pdf) and [Description](https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AVM_TR-064_first_steps.pdf)), most commonly used by Fritz!Box.

This library has been developed on an ESP8266 and tested on an ESP32.

To get started, I recommend to check out the [examples folder](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/tree/master/examples), the [Wiki](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/wiki) and the [instructable](http://www.instructables.com/id/Who-Is-Home-Indicator-aka-Weasley-Clock-Based-on-T/).

If you need further guidance, if you still have question please don't hesistate to [open an issue here on Github](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues/new). If you made something cool with this library I want to encourage you to provide it as an example. Please create a pull request (or, if you don't know how, [an issue](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues/new)) to do that.

## Installation and usage
Simply go to `Tools->Manage Libraries...` in the Arduino IDE and search for `TR-064`. Press install. The examples can then also be found in the IDE under `Examples->TR-064 SOAP Library`.

Don't forget that you also need to [add the board you are using to the IDE](https://learn.adafruit.com/add-boards-arduino-v164/setup); 
<details><summary>Example board URLs</summary>
	Please note, that these might not be the best options and can change at any time. If in doubt, do your own research :)
	<ul>
		<li>ESP8266 https://arduino.esp8266.com/stable/package_esp8266com_index.json</li>
		<li>ESP32   https://dl.espressif.com/dl/package_esp32_index.json</li>
	</ul>
</details>

## Known routers with TR-064 interface
If you know more/have tested a device not on the list, please let me know!

<ul>
	<li> Almost all Fritz!Box routers.
		<details><summary>Tested devices (click to expand)</summary>
			<ul>
				<li> Fritz!Box FON WLAN 7360</li>
				<li> Fritz!Box 7490 (tested by Dirk Kaben)</li>
				<li> Fritz!Box 7580</li>
				<li> Fritz!Box 7590</li>
				<li> FRITZ!DECT 200 (tested by Oliver-André Urban)</li>
				<li> FRITZ!DECT 210 (test by Thorsten Godau)</li>
				<li> Apparantly, pretty much all Fritz products...</li>
			</ul>
		</details>
	</li>
	<li> Some ZyXEL routers (eg. <a href="ftp://ftp.zyxel.nl/VMG4325-B10A/user_guide/VMG4325-B10A_.pdf">VMG4325, VMG8324, VMG8324-B10A and VMG4380, see page 29</a>)</li>
	<li> Please let me know if you tested a device not in this list [with an issue here on Github](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues/new)</li>
</ul>

## Press
 
 - The German magazine C'T, published [an article on how to create a doorbell using the DECT phones and an ESP8266 (in German)](https://www.heise.de/select/ct/2018/17/1534215254552977).
 - A similar article was also featured in Reichelt magazine (an electronics vendor) in [English](https://www.reichelt.com/magazin/en/build-smart-doorbell-arduino), [German](https://www.reichelt.de/magazin/how-to/smarte-tuerklingel) and [Dutch](https://www.reichelt.com/magazin/nl/zelf-een-slimme-deurbel-maken).
 

# TODO

Help is very welcome! I strongly encourage you to let me know if you came up with a cool use-case (I'd love to add more examples, see also list at the end of this file) and if you have any suggestions on how to improve the library code itself (I'm not particularily good with C++). Contributions to the [Wiki](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/wiki) are also very welcome!

I moved the todo's mostly into the [issue section here on Github](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues).

Some of the old issues are:

### Library

* Add yield()'s
* figure out the occasional crashes
* Test for different setups (i.e. other microcontroller and other routers)
* Make code more efficient
* Do some (better) error-handeling
  * Return proper errors
* Add some more comments
* Create a sophisticated debug/verbose mode (and silence the non-debug mode!)

### Examples

* Add yield()'s
* Add a few more nice examples
  * turning on/off (guest) wifi
  * read out and display connection speed
  * alternate WPS button
  * Emergency dial-up to relatives or emergency services
  * recognize when devices (like washing machines) are done (along the lines of [this](https://github.com/dl9sec/ArduinoSIP/tree/master/examples/LaundryNotifier)
  * Start devices through e.g. telegram messages (e.g. coffee machine, washing machine, ...)
* WhoIsHomeIndicator
  * Add nice GUI/Website to configure/manage the users/LEDs/MACs/known devices
  * Known Bug: After a few hours the known devices "freeze", needs restart
* Extend doorbell example to also send telegram messages :)


<table>
<tr><td>◽</td><td>◾</td><td>◽</td></tr>
<tr><td>◾</td><td>◽</td><td>◽</td></tr>
<tr><td>◾</td><td>◾</td><td>◾</td></tr>
</table>