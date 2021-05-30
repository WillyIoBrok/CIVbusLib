/* 
	ICradio.h - Library for communication via ICOM's CI-V bus
	Created by Wilfried Dilling, DK8RW, May 30, 2021
	Released into the public domain

	Control layer for ICOM's radios
*/
#ifndef ICradio_h
#define ICradio_h


// some timing definitions (based on ms)
#define t_waitForAnswer 100
#define t_RadioCheck    1800

constexpr long unsigned t_radio_OFF_TR[4] = {
	5000, // boot time of the IC7100
	5000, // boot time of the IC7300
	6500, // boot time of the IC9700
	4000 	// boot time of the IC705
};


// Radio IDs used in radio module/class
enum radioType_t:uint8_t {
	TypeIC7100=0,
	TypeIC7300,
	TypeIC9700,
	TypeIC705,
  TypeICnone
};


// states of radio's DC-Power (on/Off State)
enum radioOnOff_t:uint8_t {
  RADIO_OFF = 0,
  RADIO_ON  = 1,
  RADIO_OFF_TR,     // transit from OFF to ON
  RADIO_ON_TR,      // transit from ON to OFF
  RADIO_NDEF,       // don't know
  RADIO_TOGGLE
};

// modes of radio
enum radioMode_t:uint8_t {
	MODE_NDEF = 0,
	MODE_VOICE,
	MODE_DATA
};

// class definition
class ICradio {

public:

// ctor = constructor
  ICradio(radioType_t thisRadio, uint8_t myCIVaddr);
      
//------------------------------------------------------------------------
// public member functions

	//::::::::::::: initialisation of the class ICradio
  void    setupp(unsigned long currentTime);

  //::::::::::::: get and process the CIV-answers from the radio
  void    getNewMsg();

  //::::::::::::: this method 
	void		loopp(unsigned long currentTime);

  //::::::::::::: check, wether radio is switched on and connected
  radioOnOff_t getAvailability();
	
  //::::::::::::: switch radio ON/OFF
  radioOnOff_t setDCPower(radioOnOff_t onOff,unsigned long currentTime);

  //::::::::::::: get radio mode
	radioMode_t getMode();

  //::::::::::::: switch radio mode
	radioMode_t setMode(radioMode_t mode);

  //::::::::::::: set date_time
	void setDateTime();

  //::::::::::::: get operating frequency of the radio
	unsigned long getFrequency();

  //::::::::::::: set/get the CI-V address
  void setCIVaddr(uint8_t myCIVaddr);
	
  uint8_t getCIVaddr();
	
	

private:
//------------------------------------------------------------------------
// private methods


//------------------------------------------------------------------------
// private variables

	radioType_t     _radioType;
  uint8_t         _radioAddr;
  radioMode_t     _radioMode;
  radioOnOff_t    _radioOnOffState;
  bool            _waitForAnswer;
  bool            _waitForIDquery;
  
  unsigned long   _frequency;

  radioMode_t     _sequMode;
  const uint8_t   *_sequPntr;
  uint8_t         _sequCmdIdx;
  uint8_t         _sequNoOfCmds;
  uint8_t         _sequNoOfNOKs;

  unsigned long   _ts_lastIDquery;
  unsigned long   _ts_waitForAnswer;
  unsigned long   _ts_lastOnCmd;

}; // end class ICradio


#endif
