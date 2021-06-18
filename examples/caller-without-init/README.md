# Further information on the usage of caller.ino

The script is designed to place a call with a push of a button. Using the internal DECT call function one could use this as e.g. a doorbell. Don't forget to activate 'Click to call' in your browser [more](/Aypac/Arduino-TR-064-SOAP-Library/wiki/Q&A:-I-want-to-use-DECT-phones-with-this-library,-but-it-is-not-working!). If you want to all phones to ring, change the dialed number to \**9.

[Detailed (German) Discussion](http://www.ip-phone-forum.de/showthread.php?t=295676).

One can use an opto-coupler in order to use it with existing infrastrucutre (e.g. an existing doorbell), otherwise a regular switch can be used (see code). Schematics might be added in the future. If you need further guidance, want to provide your own example or find a bug, please open an issue here on Github.
