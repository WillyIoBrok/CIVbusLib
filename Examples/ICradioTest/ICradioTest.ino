/* 
CIVbusLib ICradioTest V0_6 - W.Dilling/DK8RW

Pressing "e" + "return" switches on a IC7300 connected via CIV-bus 
and prints out some status info.

Pressing "a" + "return" switches off a IC7300 connected via CIV-bus 
and prints out some status info.

Pressing "t" + "return" toggles the power on state of a IC7300 connected
via CIV-bus and prints out some status info.

Pressing "v" + "return" sets a IC7300 connected via CIV-bus to Voice mode

Pressing "d" + "return" sets a IC7300 connected via CIV-bus to data mode

Pressing "f" + "return" gets the frequency of a IC7300 connected via CIV-bus
(Note: if this command returns "0", the internal frequency of instance IC7300
 hasn't been updated. This would be chnged by turning the main tuning know 
 on the radio or by changing bands)

Pressing "l" + "return" gives a readout of the CIV logbuffer

Please ensure, that "#define debug" in "defines.h" is activated/uncommented,
otherwise you will see nothing in serial monitor!

In order to get the best information from the test (CIV logbuffer),
please activate(uncomment) "#define log_CIV" in file civ.h
in the library CIVbuslib first !
This is deactivated by default due to save data memory.

*/

/* includes -----------------------------------------------------------------*/

#include "a_defines.h"

#include <CIVmaster.h>
#include <CIVcmds.h>
#include <ICradio.h>

//-------------------------------------------------------------------------------
// create the civ and ICradio objects in use

//#define useIC7100
#define useIC7300
//#define useIC9700

CIV     civ;  // create the CIV-Interface object first (mandatory for the use of ICradio)

#ifdef useIC7100
  ICradio IC7100(TypeIC7100,CIV_ADDR_7100);
#endif

#ifdef useIC7300
  ICradio IC7300(TypeIC7300,CIV_ADDR_7300);
#endif

#ifdef useIC9700
  ICradio IC9700(TypeIC9700,CIV_ADDR_9700);
#endif

//-------------------------------------------------------------------------------


// global values for Date / Time (systemwide), which must be set somehow 
// in the mainprogram. 
// Since IC7300 tends to loose it's clock information (at least mine does it), class ICradio sets the clock 
// automatically after switchig on the device and needs the time / date information
uint8_t G_time[3]       = {2, 0x20, 0x44};              // 20:44
uint8_t G_date[5]       = {4, 0x20, 0x20, 0x11, 0x06};  // 2020-11-06
uint8_t G_UTCdelta[4]   = {3, 0x01,0x00,0x00};          // we are 1h ahead (in winter time)


//-------------------------------------------------------------------------------

uint8_t lpCnt = 0;
CIVresult_t CIVresultL;

radioOnOff_t mainState = RADIO_OFF;

#define BASELOOP_TICK 10 
unsigned long time_current_baseloop;
unsigned long time_last_baseloop;


#ifdef debug
  unsigned long G_timemarker1;
  unsigned long G_timemarker1a;
#endif

//---------------------------------------------------------------------------------------------
// check and get the status of the keys (+ simulated keys) for different testcases

keyPressed_t get_key() {

  keyPressed_t ret_val = NO_KEY_PRESSED;
  uint8_t inByte=0;
  
  if (digitalRead(P_KEY_BLK)==0) {  // black HW-key (if available)
    delayMicroseconds(5000);                                   // wait for 5ms (key debouncing)
    if (digitalRead(P_KEY_BLK)==0) ret_val=KEY_BLK_PRESSED;
  }
  else
    if (digitalRead(P_KEY_RED)==0) {  // red HW-key (if available)
      delayMicroseconds(5000);                                 // wait for 5ms (key debouncing)
      if (digitalRead(P_KEY_RED)==0) ret_val=KEY_RED_PRESSED;
    }

  #ifdef debug
    if (Serial.available()>0)  inByte = Serial.read();

    if (inByte=='r') ret_val = KEY_RED_PRESSED;
    if (inByte=='b') ret_val = KEY_BLK_PRESSED;
    if (inByte=='v') ret_val = KEY_VOICE_PRESSED;
    if (inByte=='d') ret_val = KEY_DATA_PRESSED;
    if (inByte=='e') ret_val = KEY_EIN_PRESSED;
    if (inByte=='a') ret_val = KEY_AUS_PRESSED;
    if (inByte=='t') ret_val = KEY_TOGGLE_PRESSED;
    if (inByte=='f') ret_val = KEY_FREQ_PRESSED;
    if (inByte=='l') ret_val = KEY_LOG_PRESSED;
    if (inByte=='x') ret_val = KEY_X_PRESSED;
    
  #endif


  return ret_val;
}

//---------------------------------------------------------------------------------------------
// different test cases

// test of radio on/off and radio DC power state
void testRadioOnOff (uint8_t key) {

  radioOnOff_t radioState;
  
  if (key==KEY_TOGGLE_PRESSED) { // OFF
    civ.logClear();
    Serial.print (time_current_baseloop);
    Serial.println (" KEY_TOGGLE*");
    radioState = IC7300.setDCPower(RADIO_TOGGLE,time_current_baseloop);
  }
  else {                                                      // update IC7300 state
    radioState = IC7300.getAvailability();
  }

  
  if (key==KEY_EIN_PRESSED) { // ON
    civ.logClear();
    Serial.print (time_current_baseloop);
    Serial.println (" KEY_EIN*");
    SET_TIME_MARKER1;
    radioState = IC7300.setDCPower(RADIO_ON,time_current_baseloop);
    EVAL_TIME_MARKER1;
    // -> returns RADIO_ON (ON->ON) or RADIO_OFF_TR (OFF->ON)
    civ.logDisplay();
  }

  if (key==KEY_AUS_PRESSED) { // OFF
    civ.logClear();
    Serial.print (time_current_baseloop);
    Serial.println (" KEY_AUS");
    SET_TIME_MARKER1;
    radioState = IC7300.setDCPower(RADIO_OFF,time_current_baseloop);
    EVAL_TIME_MARKER1;
    civ.logDisplay();
  }    // -> returns RADIO_OFF (OFF->OFF)or RADIO_ON_TR (ON->OFF)


  if (key==NO_KEY_PRESSED) {
//    Serial.println ("No Key");
    radioState = IC7300.getAvailability();
  }

  if (radioState != mainState) {
//    Serial.print (time_current_baseloop);
//    Serial.print (" Radiostate "); Serial.println (radioState);
    mainState = radioState;
  }
  
}

//-----------------------------------------------------------------
// test of switching the modes of an IC7300
// use: press the "v" or "d" key on your computer keyboard
// if the logging is enabled, you can see the commands by pressing "l"


void mode_sequences(uint8_t key) {

  if (key==KEY_VOICE_PRESSED) {

    civ.logClear();

    IC7300.setMode(MODE_VOICE);

  }

  if (key==KEY_DATA_PRESSED) {

    civ.logClear();

    IC7300.setMode(MODE_DATA);

  }

}

//==========  General initialization  of  the device  =========================================
void setup() {

  #ifdef debug                        // initialize the serial interface (for debug messages)
    Serial.begin(19200);
    Serial.println("");
    Serial.println (VERSION_STRING);
  #endif

  pinMode(P_POWER_ON,   OUTPUT);      // power supply switch
  pinMode(P_STATUS_LED, OUTPUT);      // internal LED

  pinMode(P_KEY_BLK, INPUT_PULLUP);
  pinMode(P_KEY_RED, INPUT_PULLUP);

  civ.setupp();                       // initialize the civ object/module
                                      // and the ICradio objects
	#ifdef useIC7100
		IC7100.setupp(millis());
	#endif
  
  #ifdef useIC7300
		IC7300.setupp(millis());          // registering of the CIV-address in civ is done
  #endif                              // automatically in this step

	#ifdef useIC9700
		IC9700.setupp(millis());
	#endif

  time_current_baseloop = millis();
  time_last_baseloop = time_current_baseloop;
  
}

//============================  main  procedure ===============================================
void loop() {

  keyPressed_t keyCmd;

  time_current_baseloop = millis();
  
  if ((time_current_baseloop - time_last_baseloop) > BASELOOP_TICK) {

//---------------------------------------------------------------------------------------------
// calling the loop function of each radio in use as often as possible (approx. 10ms) is mandatory !
// However, this should not happen for all radios at the same time. Therefore they are 
// separated by one lpCnt-tick
//
// In addition, the first call after boot up for each radio shall be separated by a
// time difference of apprx. 300ms (so the ID-query / check, whether the radio is switched on
// is run in different timeslots for each radio - remember, the CI-V bus is only one HW wire!
//
// However, these precautions are only in order to be on the safe side ... if you decide to call 
// them immediately one after the other should work as well (but has not been tested!) 

		#ifdef useIC7100
      if ((lpCnt%3) == 0) 
        if (time_current_baseloop>t_RadioCheck +200) IC7100.loopp(time_current_baseloop);
		#endif

		#ifdef useIC7300
      if ((lpCnt%3) == 1) 
        if (time_current_baseloop>t_RadioCheck +500) IC7300.loopp(time_current_baseloop);
		#endif

		#ifdef useIC9700
      if ((lpCnt%3) == 2) 
        if (time_current_baseloop>t_RadioCheck +800) IC9700.loopp(time_current_baseloop);
		#endif


    keyCmd = get_key();


//---------------------------------------------------------------------------------------------
// different test cases

    testRadioOnOff (keyCmd);  // use "e", "a" or "t"

    mode_sequences (keyCmd);  // use "v" or "d"

    if (keyCmd==KEY_FREQ_PRESSED) {
      Serial.print("Frequency: "); Serial.println (IC7300.getFrequency());
    }

    // use "l",if "#define log_CIV" in file civ.h is active
    if (keyCmd==KEY_LOG_PRESSED) civ.logDisplay();

//---------------------------------------------------------------------------------------------
    lpCnt++;
    time_last_baseloop = time_current_baseloop;
	} // if BASELOOP_TICK
  
} // end loop
