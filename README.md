# Arduino-TR-064-SOAP-Library
Arduino library for the TR-064 protocol, most commonly used by Fritz!Box. A [Description of the protocol](https://avm.de/fileadmin/user_upload/Global/Service/Schnittstellen/AVM_TR-064_first_steps.pdf).

Has been developed on an ESP8266 and was tested with a Fritz!Box FON WLAN 7360.

For illustration on the example, please also visit my [instructable](http://www.instructables.com/id/Who-Is-Home-Indicator-aka-Weasley-Clock-Based-on-T/).

# TODO
Help is very welcome (especially when improving the library code). I'm not a genius with C++.
##Library
*Test for different setups (i.e. other microcontroller and other routers)
*Make code more efficient
*Do some (better) error-handeling
	*Return proper errors
	*Re-initialize authorization to have a fresh Nonce
*Add some more commentary
*Create a sophisticated debug/verbose mode (and silence the non-debug mode!)

##Example
*Add nice GUI/Website to configure/manage the users/LEDs/MACs/known devices
