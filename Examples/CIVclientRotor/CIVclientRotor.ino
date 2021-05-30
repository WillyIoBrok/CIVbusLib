/* 
 CIVclientRotor V0_3 - W.Dilling/DK8RW

This SW is intended to run on a (homebrew) client arduino. I have added a CIV-interface
to my rotator control device.

Basically the process is as follows:
1. watch continuously the CIV input and wait for incoming commands
2. if a command for this device is detected it will be decoded and
   an appropriate command will be sent to the rest of the system
3. give an ack (pos or neg) or a return value back to the CIV master

For testing purposes without CIV master please uncomment/activate the define
"#define debugWithoutMaster" ("CIVclient.h") and select the
test command of your choice in "readMsg" ("CIVclient.cpp")

*/

/* includes -----------------------------------------------------------------*/

#include "a_defines.h"

#include <CIVclient.h>

#include <CIVcmds.h>
#include <CIVcmdsRotor.h>

//-------------------------------------------------------------------------------
// create the civ object

  CIVCLIENT     civclient(CIV_ADDR_ROTOR);  // create the CIVclient-Interface object

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


//-----------------------------------------------------------------------------------------------
// those definitions/variables/functions are normally definen in the rest of the rotor sw system
// here in this example they are working as test-stubs

#define ROT_ROT_WINDOW 0
#define ROT_ROT_CCW 1
#define ROT_ROT_CW 2

int16_t rot_dir_current = 321; // current direction;  1° resolution ("Yaesugrad")
int16_t rot_dir_target = 47;  // target direction;   1° resolution ("Yaesugrad")

uint8_t rot_rot_mode;           // mode of rotorcontrol
uint8_t rot_ctrl_active;        // rotator is currently rotating

//---------------------------------------------------------------------------------------------
// check and get the status of the keys (+ simulated keys) for different testcases

keyPressed_t get_key() {

  keyPressed_t ret_val = NO_KEY_PRESSED;
  uint8_t inByte;
  
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
    if (inByte=='y') ret_val = KEY_Y_PRESSED;
    
  #endif

  return ret_val;
}

//---------------------------------------------------------------------------------------------

void displayResult(CIVresult_t CIVresultL) {
  uint8_t idx;
#ifdef debug
  Serial.print("State: ");Serial.print(CIVresultL.retVal);
  Serial.print(" Addr: ");Serial.println(CIVresultL.address,HEX);
  Serial.print(" cmd: ");
  for (idx=1;idx<CIVresultL.cmd[0]+1;idx++) {
    Serial.print(CIVresultL.cmd[idx],HEX);Serial.print(".");
  }
  Serial.println("");
  Serial.print(" data: ");
  for (idx=1;idx<CIVresultL.datafield[0]+1;idx++) {
    Serial.print(CIVresultL.datafield[idx],HEX);Serial.print(".");
  }
  // value is always "0" if CIVresultL is set by writeMsg
  Serial.println("");
  Serial.print(" value: ");Serial.println(CIVresultL.value);
  Serial.println("");
#endif
}

//---------------------------------------------------------------------------------------------
// send acknowledgement to the sender of the message
CIVresult_t posAck(uint8_t address) {
  return (civclient.writeMsg(address, CIV_C_OK, CIV_D_NIX));
}

CIVresult_t negAck(uint8_t address) {
  return (civclient.writeMsg(address, CIV_C_NOK, CIV_D_NIX));
}


//==========  General initialization  of  the device  =========================================
void setup() {

  pinMode(P_POWER_ON,   OUTPUT);      // power supply switch
  pinMode(P_STATUS_LED, OUTPUT);      // internal LED

  pinMode(P_KEY_BLK, INPUT_PULLUP);
  pinMode(P_KEY_RED, INPUT_PULLUP);

  civclient.setupp();                       // initialize the civ object/module
                                      // and the ICradio objects

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
  uint8_t   idx;
  int16_t   antDirL;
  uint8_t   antDirLbuf[3]={02,0x02,0x17};
  bool      printAnswer;


  time_current_baseloop = millis();
  
  if ((time_current_baseloop - time_last_baseloop) > BASELOOP_TICK) {

//---------------------------------------------------------------------------------------------

    keyCmd = get_key();

    CIVresultL.retVal = CIV_NO_MSG;

    CIVresultL = civclient.readMsg();

    printAnswer = false;
    if (keyCmd==KEY_X_PRESSED) {  // check the data which have been received
      displayResult(CIVresultL);
      printAnswer = true;
    }

    if (CIVresultL.address == CIV_ADDR_MASTER) { // only messages from the master will be processed

      if (CIVresultL.retVal<CIV_NOK)  { //------------------- message OK or OK_DAV
        switch (CIVresultL.cmd[1]) {
          case CIV_C_R_R[1]:                                // rotate right (CW)
            rot_rot_mode=ROT_ROT_CW;
            CIVresultL = posAck(CIVresultL.address);
            #ifdef debug
              Serial.println("R_R");
            #endif
          break;
          case CIV_C_R_L[1]:                                // rotate left  (CCW)
            rot_rot_mode=ROT_ROT_CCW;
            CIVresultL = posAck(CIVresultL.address);
            #ifdef debug
              Serial.println("R_L");
            #endif
          break;
          case CIV_C_R_A[1]:                                // stop rotation
            rot_rot_mode=ROT_ROT_WINDOW;
            CIVresultL = posAck(CIVresultL.address);
            #ifdef debug
              Serial.println("R_A ");
            #endif
          break;
          case CIV_C_R_S[1]:                                // stop everything
            rot_rot_mode=ROT_ROT_WINDOW;
            rot_dir_target = rot_dir_current;               // ... stop active "M" command
                                                            // if necessary
            CIVresultL = posAck(CIVresultL.address);
            #ifdef debug
              Serial.println("R_S");
            #endif
          break;
          case CIV_C_R_M[1]:                                // set wanted antenna direction
            if (CIVresultL.retVal!=CIV_OK_DAV) {             // if no data sent -> error !
              CIVresultL = negAck(CIVresultL.address);
            } else  {
              if (CIVresultL.value<= 360) {                   // only values 0 ... 360deg allowed
                rot_rot_mode=ROT_ROT_WINDOW;                  // stop left- / right rotation
                rot_dir_target = CIVresultL.value;
                rot_ctrl_active=true;
                CIVresultL = posAck(CIVresultL.address);
              } else
                CIVresultL = negAck(CIVresultL.address);
            }
            #ifdef debug
              Serial.print(rot_dir_target);
              Serial.println(" R_M ");
            #endif

          break;
          case CIV_C_R_C[1]:                                // get current antenna direction
          
            antDirL = rot_dir_current;
            antDirLbuf[1] = antDirL/100; antDirL = antDirL%100;
            antDirLbuf[2] = ((antDirL/10)<<4) | (antDirL%10); 

            CIVresultL = civclient.writeMsg(CIVresultL.address, CIV_C_R_C, antDirLbuf );

            #ifdef debug
//             Serial.print(rot_dir_current);
              Serial.print(" R_C"); Serial.print("   ");
              for (idx=1; idx<=antDirLbuf[0];idx++) {
                Serial.print (antDirLbuf[idx],HEX);
                Serial.print (".");
              }
              Serial.println("");

              civclient.logDisplay();
          
            #endif

          break;
          default:                                  // command not known
            if (CIVresultL.address == CIV_ADDR_MASTER)
              CIVresultL = negAck(CIVresultL.address);
        }
      }
      else if (CIVresultL.retVal==CIV_NOK){ //-------------- message not OK -> negative ack
        CIVresultL = negAck(CIVresultL.address);
      }

    }
    
    if (printAnswer) {
      printAnswer = false;
      displayResult(CIVresultL);
    }

    // use "l",if "#define log_CIV" in file civ.h is active
    if (keyCmd==KEY_LOG_PRESSED) civclient.logDisplay();

//---------------------------------------------------------------------------------------------
    lpCnt++;
    time_last_baseloop = time_current_baseloop;
	} // if BASELOOP_TICK
  
} // end loop
