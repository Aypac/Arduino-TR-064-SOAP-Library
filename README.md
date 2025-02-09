# Arduino TR-064 SOAP Library [![Linting](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/actions/workflows/lint.yml/badge.svg)](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/actions/workflows/lint.yml) ![Test compile](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/actions/workflows/arduino-test-compile.yml/badge.svg)
Arduino library to facilitate the use of the TR-064 SOAP protocol. With this you can get a lot of information from and control common routers and their peripherals, like the FRITZ!Box or some ZyXEL.

IMPORTANT NOTE: it seems there is a problem with newer Arduino IDE versions. **Please use IDE 1.8.18 and ESP8266 boards version not past/newer than 2.7.4**. Version for other boards has to be tested. I will try to fix this asap (Note from Feb 2025).

A few examples of what you can do with it include:
 - Retrieve devices currently connected to the WIFI or LAN
 - Retrieve statistics on internet data usage
 - Turn WiFi on or off (e.g. Guest WiFi)
 - Make connected DECT phones ring (e.g. as a doorbell replacement)
 - Turn on and off connected Telephone answering machines
 - Get amount of power passing through connected smart plugs and take decisions on it
 - Turn smart plugs on or off
 - See more under examples and at the end of this README

This library has been developed on an ESP8266 and tested on an ESP32 and with various hardware of [AVM FRITZ!OS](https://en.avm.de/).

To get started, I recommend to check out the [examples folder](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/tree/master/examples), the [Wiki](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/wiki) and the [instructable](http://www.instructables.com/id/Who-Is-Home-Indicator-aka-Weasley-Clock-Based-on-T/).

If you still have question please don't hesistate to [open an issue here on Github](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues/new). If you made something cool with this library please provide it as an example. Please create a pull request (or, if you don't know how, [an issue](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues/new)) to do that.

For details on the TR-064 SOAP protocol: [my "simple" explanation](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/wiki/How-does-the-TR-064-protocol-work%3F), the [definition](https://www.broadband-forum.org/technical/download/TR-064.pdf) and an official [description](https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AVM_TR-064_first_steps.pdf)).

## Installation and usage
Simply go to `Tools → Manage Libraries...` in the Arduino IDE and search for `TR064`. Press install. The examples can then also be found in the IDE under `Examples → TR064`.

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
	<li> Almost all FRITZ!Box routers.
		<details><summary>Tested devices (click to expand)</summary>
			<ul>
				<li> FRITZ!Box 5590</li>
				<li> FRITZ!Box FON WLAN 7360</li>
				<li> FRITZ!Box 7490 (tested by Dirk Kaben)</li>
				<li> FRITZ!Box 7580</li>
				<li> FRITZ!Box 7590</li>
				<li> FRITZ!Box 6590 Cable (tested by <a href='https://github.com/jipp'>Wolfgang (jipp)</a>)</li>
				<li> FRITZ!Box 5490 (<a href='https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues/21'>not finally confirmed</a>, tested by <a href='https://github.com/Paul760'>Paul760</a>)</li>
				<li> FRITZ!DECT 200 (tested by Oliver-André Urban)</li>
				<li> FRITZ!DECT 210 (test by Thorsten Godau)</li>
			</ul>
		</details>
	</li>
	<li> Some ZyXEL routers (eg. <a href="ftp://ftp.zyxel.nl/VMG4325-B10A/user_guide/VMG4325-B10A_.pdf">VMG4325, VMG8324, VMG8324-B10A and VMG4380, see page 29</a>)</li>
	<li> Please let me know if you tested a device not in this list <a href='https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues/new'>by creating an issue</a></li>
</ul>

## Press
 
 - The German magazine C'T, published [an article on how to create a doorbell using the DECT phones and an ESP8266 (in German)](https://www.heise.de/select/ct/2018/17/1534215254552977).
 - A similar article was also featured in Reichelt magazine (an electronics vendor) in [English](https://www.reichelt.com/magazin/en/build-smart-doorbell-arduino), [German](https://www.reichelt.de/magazin/how-to/smarte-tuerklingel) and [Dutch](https://www.reichelt.com/magazin/nl/zelf-een-slimme-deurbel-maken).
 

## Contribute

Help is very welcome! For example:
 - Check the [issue section on Github](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/issues) or the list below and see if you can help
 - Please let me know if you came up with a cool use-case (I'd love to add more examples, see also list at the end of this file)
 - Let me know if you tested it on hardware not listed here (i.e. other microcontroller and other routers)
 - If you have any suggestions on how to improve the library code itself (I'm not particularily good with C++).
 - Contributions to the [Wiki](https://github.com/Aypac/Arduino-TR-064-SOAP-Library/wiki) are also very welcome!

Some of the old issues are:

### Library

* Add yield()'s (where sensible)
* Error-handeling can always be improved
* Should some of the calls of the examples be included into the library?
* Get the DOXY to work again

### Examples

* Add yield()'s (where sensible)
* Add a few more nice examples
  * alternate WPS button
  * emergency dial-up to relatives or emergency services
  * Start devices through e.g. telegram messages (e.g. coffee machine, washing machine, ...)
* WhoIsHomeIndicator
  * Add nice GUI/Website to configure/manage the users/LEDs/MACs/known devices
* Extend doorbell example to also send telegram messages :)
* Do something with the FRITZ!DECT Thermostats

<hr />

<p align="justify" style="text-align:justify;">
	<table style="text-align:center;">
		<tr><td>◽</td><td>◾</td><td>◽</td></tr>
		<tr><td>◾</td><td>◽</td><td>◽</td></tr>
		<tr><td>◾</td><td>◾</td><td>◾</td></tr>
	</table>
</p>
