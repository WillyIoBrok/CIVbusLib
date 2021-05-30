/* 
	CIVclient.h - Library for a client device in ICOM's CI-V bus
	Created by Wilfried Dilling, DK8RW, May 30, 2021
	Released into the public domain
*/
#ifndef CIVclient_h
#define CIVclient_h


// Debugging:

// switching the debugging features on or off; Pls be careful, in the case of
// "log_CIVclient is defined" a significant amount of RAM space will be
// used in addition (due to logging).
// Requires access to the serial line,i.e. Serial communication via USB
// has to be initialized in the main program

//	#define log_CIVclient    			// switch on logging (command-structured, in and out)
																	// ringbuffer

//	#define debugWithoutMaster		// generate testpatterns without master 

// Selection of the serial port in use
// Note: there are certainly other arduino modules that have more than one serial
// HW port. If you own one, you can add it here in these statements.
// For me it's enough to differentiate between Uno, Nano, Pro mini, Pro versus Mega versus ESP32

// On processors with only one serial HW port, a software emulation is used
// see AltSoftSerial - documentation

#if defined(ARDUINO_AVR_UNO)||defined(ARDUINO_AVR_NANO)||defined(ARDUINO_AVR_MINI)||defined(ARDUINO_AVR_PRO)
  #define useAltSoftSerial
#elif defined(ARDUINO_AVR_MEGA2560)
  #define useSerial_1
#elif defined(ESP32)
  #define useSerial_2
#endif


#ifdef useAltSoftSerial
	#include <AltSoftSerial.h>
#endif


// general serial interface switches
#define CIV_BAUDRATE 19200

#define UART_TIMEOUT 1000

// maximum length+1 of sent / received messages
#define CIV_BUFFERSIZE 64

// structure of return values of readMsg
typedef struct {
  uint8_t retVal;
  uint8_t address;
  uint8_t cmd[5];
  uint8_t datafield[10];
  unsigned long value;
} CIVresult_t;


// time definitions in multiple of 1ms
#define t_msDelay_5ms 5

// time definitions based on no of loops

#define t_sendCmd       t_usLoop_10ms
#define t_waitForRadio  t_usLoop_100ms
#define t_readMsg       t_usLoop_40ms

constexpr uint8_t  t_usLoop = 50;

constexpr uint16_t t_usLoop_5ms   =   5000/t_usLoop;
constexpr uint16_t t_usLoop_10ms  =  10000/t_usLoop;
constexpr uint16_t t_usLoop_40ms  =  40000/t_usLoop;
constexpr uint16_t t_usLoop_100ms = 100000/t_usLoop;
constexpr uint16_t t_usLoop_250ms = 250000/t_usLoop;
constexpr uint16_t t_usLoop_300ms = 300000/t_usLoop;


//return codes used in CIV module/class
enum retVal_t :uint8_t {
	CIV_OK           =  0,
	CIV_OK_DAV       =  1,

	CIV_NOK          =  2,	// this is the border between good and bad and will be used as such
	CIV_HW_FAULT     =  3,
	CIV_BUS_BUSY     =  4,
	CIV_BUS_CONFLICT =  5,
	CIV_NO_MSG    	 =  6
};

// state of the CIV-bus
enum CIV_State_t:uint8_t {
	CIV_idle 		= 0,
	CIV_sync 		= 1,
	CIV_collect	=	2,
	CIV_stop		= 3
};
	
// CI-V (default-)addresses:
constexpr uint8_t CIV_ADDR_ALL    	= 0x00; // address "to all"; can be set in the radio under "Transceive Address"
constexpr uint8_t CIV_ADDR_MASTER 	= 0xE0; // (Default-)address of the Master
constexpr uint8_t CIV_ADDR_NONE 		= 0xF9; // this is an invalid CIV address

constexpr uint8_t CIV_ADDR_ROTOR 		= 0x81; // homebrew Rotor device (DK8RW)


// fixed ICOM key bytes
constexpr uint8_t C_START  = 0xFE;
constexpr uint8_t C_STOP   = 0xFD;
constexpr uint8_t C_OK     = 0xFB;
constexpr uint8_t C_NOK    = 0xFA;


// class definition
class CIVCLIENT {

public:

  // ctor
  CIVCLIENT(uint8_t myCIVaddr);
      
//------------------------------------------------------------------------
// public member functions

	//::::::::::::: initialisation of the class CIV
  void    setupp();


	//::::::::::::: read the traffic on the bus and return a message if client has been addressed
  CIVresult_t readMsg();

	
	//::::::::::::: write an answer from the client to the CI-V master or ALL others
  CIVresult_t writeMsg (const uint8_t deviceAddr, const uint8_t cmd_body[], const uint8_t cmd_data[]);


	//::::::::::::: logging

	#ifdef log_CIVclient
		#define logMaxEntries 18
		#define logMsgLength  20
		#define logNameLength 5

		static uint8_t   logEntry;
		static uint8_t   logBuffer	[logMaxEntries][logMsgLength];
    static uint8_t  	 logBufferState[logMaxEntries];
		static char      logBufferName	[logMaxEntries][logNameLength];
  #endif


  void logClear();
	void logNewEntry(uint8_t msg[],const char name[], const uint8_t state);
	void logNewInRing (uint8_t inByte);
	void logDisplayLine(uint8_t lineNo);
	void logDisplay();

private:
//------------------------------------------------------------------------
// private methods

	// low level serial access
	uint8_t 	serAvailable();
	uint8_t 	serRead();
	void 			serWrite(uint8_t ch);
  void 			serflushOutput();

//------------------------------------------------------------------------
// private variables

	uint8_t	rxBuffer[CIV_BUFFERSIZE];
	uint8_t	_myCIVaddr;
 
  CIVresult_t CIVresultBufS;

}; // end class CIVCLIENT


#endif
