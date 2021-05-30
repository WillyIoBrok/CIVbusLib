/* 
CIVbuslib CIVTest V0_6 - W.Dilling/DK8RW

Entering "f" + "CR" on the keyboard starts a query for frequency 
of a IC7300 which is connected via CIV-bus and prints out some status info.

Tuning the frequency on the IC7300 gives a printout as well

Entering "l" + "CR" gives a readout of the CIV logbuffer


Please ensure, that "#define debug" in "defines.h" is activated/uncommented,
otherwise you will see nothing in serial monitor!

In order to get the best information from the test (CIV-logbuffer),
please activate(uncomment) "#define log_CIV" in file CIVmaster.h
in the library CIVbuslib first !
This is switched off by default due to saving data memory reasons

*/

/* includes -----------------------------------------------------------------*/

#include "a_defines.h"

#include <CIVmaster.h>
#include <CIVcmds.h>

//-------------------------------------------------------------------------------
// create the civ object

  CIV     civ;  // create the CIV-Interface object

//-------------------------------------------------------------------------------

uint8_t lpCnt = 0;
CIVresult_t CIVresultL;

#define BASELOOP_TICK 10 
unsigned long time_current_baseloop;
unsigned long time_last_baseloop;


#ifdef debug
  unsigned long G_timemarker1;
  unsigned long G_timemarker1a;
#endif

//---------------------------------------------------------------------------------------------
// check and get the status of the keys (+ simulated keys from the keyboard) 
// for different testcases

keyPressed_t get_key() {

  keyPressed_t ret_val = NO_KEY_PRESSED;
  uint8_t inByte = 0;

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

//==========  General initialization  of  the device  =========================================
void setup() {

  pinMode(P_POWER_ON,   OUTPUT);      // power supply switch
  pinMode(P_STATUS_LED, OUTPUT);      // internal LED

  pinMode(P_KEY_BLK, INPUT_PULLUP);
  pinMode(P_KEY_RED, INPUT_PULLUP);

  civ.setupp();                       // initialize the civ object/module
                                      // and the ICradio objects

  civ.registerAddr(CIV_ADDR_7300);    // tell civ, that this is a valid address to be used

  #ifdef debug                        // initialize the serial interface (for debug messages)
    Serial.begin(19200);
    Serial.println("");
    Serial.println (VERSION_STRING);
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
// different test cases

    keyCmd = get_key();

    if (keyCmd==KEY_FREQ_PRESSED) {
      CIVresultL = civ.writeMsg (CIV_ADDR_7300, CIV_C_F_READ, CIV_D_NIX, CIV_wChk);
      Serial.print("retVal of writeMsg: "); Serial.println(CIVresultL.retVal);
/*
      //give the radio some time to answer version 1:
      delay(20);
      CIVresultL = civ.readMsg(CIV_ADDR_7300);
      Serial.print("retVal: ");      Serial.print(CIVresultL.retVal);
      Serial.print(" Frequency: "); Serial.println(CIVresultL.value);
*/
    }

    // give the radio some time to answer - version 2:  
    CIVresultL = civ.readMsg(CIV_ADDR_7300);
    if (CIVresultL.retVal<=CIV_NOK) {  // valid answer received !
      Serial.print("retVal: ");      Serial.print(CIVresultL.retVal);
      if (CIVresultL.retVal==CIV_OK_DAV) // Data available
        Serial.print(" Frequency: "); Serial.println(CIVresultL.value);
    }

    // use "l",if "#define log_CIV" in file civ.h is active
    if (keyCmd==KEY_LOG_PRESSED) civ.logDisplay();

//---------------------------------------------------------------------------------------------
    lpCnt++;
    time_last_baseloop = time_current_baseloop;
	} // if BASELOOP_TICK
  
} // end loop
