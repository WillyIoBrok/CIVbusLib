# CIVbusLib
Arduino library for accessing ICOM's Amateur Radios and also homebrew devices
via ICOM CI-V Bus ("onewire" bus or via Bluetooth for the IC-705).

Version: 0.6

Date: 30-05-2021

Devices Supported:
* basically all Arduino devices with at least one (better two) serial HW ports
* tested on:
* ATMega328P  (Arduino UNO and Pro Mini)
* ATMega2560	(Arduino Mega 2560)
* ESP32				(with this processor, also Bluetooth is possible)

SW for the CI-V master:
It can be used in two different ways:
1. use of the class CIV only (include CIVmaster.h) for 
   direct access to the CIV-Bus via read and write methods
2. use of the class "ICradio" in addition to "CIV" (include CIVmaster.h and ICradio.h). 
   In this case you have access to some higher level controls of the radios which I found to be useful.
   
The command subset regarding ICOM's radios as required for the master is stored in CIVcmds.h

SW for a CI-V client:
- Use of the class CIVclient by including CIVclient.h
- A sample command set (homebrew) for an antenna rotator can be found in CIVcmdsRotor.h
- An example can be found in CIVclientRotor, a test program for this client running on CIVmaster can be found
  under CIVrotorTest

Using CIVmaster and CIVclient is mutually exclusive - only one of them may be used on a single processor 

### Notes:
Getting started is easy!
Copy the complete directory "CIVbusLib" with all files included (pretty small size, though)
into the directory "...\...\Arduino\libraries\" on your PC.
That's all, you are ready to go from the SW side.

IF you have an Arduino with more than one serial Interfaces (ATMega2560), use the Pins TX1 and RX1
(or TX2 and RX2 on ESP32 ! ) as the interface to the CI-V bus
If you have an Arduino UNO, NANO, PRO or PRO MINI etc. there is only one serial interface available which
is used for the connection to the USB-bus. Therefore we have to use the Arduino pins 8 and 9 to connect
to the CI-V bus (see documentation of AltSoftSerial)
Switching between these three HW-possibilities will be done automatically in CIVmaster/CIVclient based on the processor type 
(ATMega328P, ATMega2560 or ESP32) in use.
If you are using an ESP32, you have in addition the possibility to choose during setup between the Hardware Serial 2 and the Bluethooth connection.

Best way to start is to read the documentation and the comments in the ...\CIVbusLib\examples\... and let those
testprograms run. They should be self explaining, hopefully ...

