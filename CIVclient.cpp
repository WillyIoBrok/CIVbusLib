/* 
	CIVclient.cpp - Library for a client device in ICOM's CI-V bus
	Created by Wilfried Dilling, DK8RW, May 30, 2021
	Released into the public domain
*/


#include <Arduino.h>

#include "CIVclient.h"

#if defined(useAltSoftSerial)
	AltSoftSerial CIVclient_SERIAL;
	constexpr uint8_t serialInUse =0;
#elif defined(useSerial_1)
	#define CIVclient_SERIAL Serial1
	constexpr uint8_t serialInUse =1;
#else
	#define CIVclient_SERIAL Serial2
	constexpr uint8_t serialInUse =2;
#endif



//ctor = constructor
CIVCLIENT::CIVCLIENT(uint8_t deviceAddr):
	_myCIVaddr(deviceAddr)
{
}

//------------------------------------------------------------------------
// public methods

//::::::::: initialize the HW /Interfaces
void CIVCLIENT::setupp() {
	CIVclient_SERIAL.begin(CIV_BAUDRATE);
 	CIVclient_SERIAL.setTimeout(UART_TIMEOUT);
}


//::::::::: 
CIVresult_t CIVCLIENT::readMsg() {

// read the data (complete commands) which are addressed either to myCIVaddr or to All
// return: received data

	uint8_t	inByte; uint16_t lpCounter = 0; 
  CIV_State_t CIV_State;

  uint8_t idx;
  uint8_t DstartIdx;
  uint8_t DstopIdx;
  unsigned long mul = 1;

  constexpr uint8_t CIV_C_LENGTH_2[]  {0x07,0x0E,0x13,0x14,0x15,0x16,0x19,0x1A,0x1B,0x1C,0x1E,0x21,0x27};

  CIVresult_t CIVresultL;

  CIVresultL.retVal       = CIV_NO_MSG;
  CIVresultL.address      = CIV_ADDR_NONE;
  CIVresultL.cmd[0]       = 0;
  CIVresultL.datafield[0] = 0;
  CIVresultL.value        = 0;

  #ifdef debugWithoutMaster

    uint8_t idx;
    // various test patterns of messages received
//    constexpr uint8_t rxBufDummy[] = { 6,C_START,C_START,CIV_ADDR_ROTOR,CIV_ADDR_MASTER,0x70,C_STOP};            // R_R
//    constexpr uint8_t rxBufDummy[] = { 6,C_START,C_START,CIV_ADDR_ROTOR,CIV_ADDR_MASTER,0x71,C_STOP};            // R_L
    constexpr uint8_t rxBufDummy[] = { 6,C_START,C_START,CIV_ADDR_ROTOR,CIV_ADDR_MASTER,0x72,C_STOP};            // R_A
//    constexpr uint8_t rxBufDummy[] = { 6,C_START,C_START,CIV_ADDR_ROTOR,CIV_ADDR_MASTER,0x73,C_STOP};            // R_S
//    constexpr uint8_t rxBufDummy[] = { 8,C_START,C_START,CIV_ADDR_ROTOR,CIV_ADDR_MASTER,0x74,0x02,0x34,C_STOP};  // R_M 0234 -> ok
//    constexpr uint8_t rxBufDummy[] = { 8,C_START,C_START,CIV_ADDR_ROTOR,CIV_ADDR_MASTER,0x74,0x05,0x46,C_STOP};  // R_M 0546 -> nok,must be 0..360
//    constexpr uint8_t rxBufDummy[] = { 6,C_START,C_START,CIV_ADDR_ROTOR,CIV_ADDR_MASTER,0x74,C_STOP};            // R_M -> nok, value mandatory
//    constexpr uint8_t rxBufDummy[] = { 6,C_START,C_START,CIV_ADDR_ROTOR,CIV_ADDR_MASTER,0x75,C_STOP};            // R_C
//    constexpr uint8_t rxBufDummy[] = { 6,C_START,C_START,CIV_ADDR_ROTOR,CIV_ADDR_MASTER,0x77,C_STOP};            // undefined cmd

		for (idx=0; idx<=rxBufDummy[0];idx++) rxBuffer[idx]=rxBufDummy[idx];
  
  #else
		// quickcheck, whether something has been received
		if (serAvailable()==0) {CIVresultL.retVal = CIV_NO_MSG; return CIVresultL;}

		//.... receive the message from the master(?) (exactly ONE message)

		lpCounter=0;CIV_State=CIV_idle; // we start from scratch ...

		while ((CIV_State!=CIV_stop) && (lpCounter<t_readMsg)) {

      lpCounter++;delayMicroseconds(t_usLoop);

			if (serAvailable()>0) {
				inByte = serRead();
				switch (CIV_State) {
			
					case CIV_idle:
						if (inByte==C_START) {				    	  // first Startbyte
							rxBuffer[0]=1; rxBuffer[1]=inByte; 
							CIV_State=CIV_sync;
						}
            else
							rxBuffer[0]=0;
					break;

					case CIV_sync:
						if (inByte==C_START) {				        // second Startbyte
							rxBuffer[0]=2; rxBuffer[2]=inByte; 
							CIV_State=CIV_collect;
						}
            else {
							rxBuffer[0]=0; CIV_State=CIV_idle;
						}
					break;

					case CIV_collect:										    // collect Data
            rxBuffer[0]++; 
            rxBuffer[rxBuffer[0]]=inByte;
            if (                                  // some plausibility checks ...
                (inByte==C_START)                 // erroneous Startbyte
                ||
                ( (rxBuffer[0]==3) &&         
                  !( (inByte==CIV_ADDR_ALL) ||    // Target-Address wrong
                     (inByte==_myCIVaddr)
                   )
                )
               )
              CIV_State = CIV_idle;               // discard the received bytes, wait for Start byte              

            if (inByte == C_STOP)                 // Stop byte received -> end of message
              CIV_State = CIV_stop;               // leave the while loop ...
					break;
					case CIV_stop:
					break;
				} // case state
			}	// if serAvailable
		}	// while loop

		if (lpCounter==t_readMsg) // timeout -> error: no complete message received(unexpected break of transmission)
			{logNewEntry(rxBuffer,"RX", CIV_NOK); CIVresultL.retVal = CIV_NOK; return CIVresultL;}

  #endif

  CIVresultL.retVal 		= CIV_OK; 			// at this point we assume everything is good
  CIVresultL.address    = rxBuffer[4];  // source address

  //............. check the received message for data and process those
  // it is assumed, that the rxBuffer has been correctly filled by the readMsg method before

  // Index              0       1     2     3     4     5     6     7     8     9     10     11   
  //                lengthbyte  FE    FE    81    E0   cmd    |     |           |      |     FD
  // 1 Byte cmd                                            D-Start  |           |      |
  // 2 Byte cmd                                                  D-Start        |      |
  // 4 Byte cmd                                                              D-Start  D-Stop=rxBuffer[0]-1

  // length of Cmd + Subcommands:
  // default: 1; if the command is in this list: 2
  // 0x1A may have 4 bytes (if cmd is 0x1A+0x05). This has to be covered extra in source code, since this is
  // currently (Jan 2021) the only 4 byte command ...


  // calculate the start of the datafield depending on command / subcommnd(s)
  DstartIdx = 6;    // 1 byte command

  if (rxBuffer[0] < DstartIdx) {CIVresultL.retVal=CIV_NOK; return CIVresultL;}  // no valid content in Buffer

  // if cmd is found in list -> 2 Byte command
  for (idx=0; idx<sizeof(CIV_C_LENGTH_2); idx++) {if (rxBuffer[5]==CIV_C_LENGTH_2[idx]) DstartIdx = 7;}
  if ((rxBuffer[5]==0x1A) && (rxBuffer[6]==0x05)) DstartIdx = 9;  // 4-Byte Command+Subcommand

  if (rxBuffer[0] < DstartIdx) {CIVresultL.retVal=CIV_NOK; return CIVresultL;}  // no valid content in Buffer

  DstopIdx = rxBuffer[0]-1;

  for (idx=5; idx<DstartIdx; idx++)                       							// load cmdfield of result
    CIVresultL.cmd[idx-4]=rxBuffer[idx];
  CIVresultL.cmd[0] = DstartIdx-5;

  for (idx = DstartIdx; idx <= DstopIdx; idx++)           							// load datafield of result
    CIVresultL.datafield[idx-DstartIdx+1] = rxBuffer[idx];
  CIVresultL.datafield[0] = DstopIdx-DstartIdx+1;

  if (CIVresultL.datafield[0]>0) {
    CIVresultL.retVal = CIV_OK_DAV; 																		// data are available

    mul = 1; CIVresultL.value = 0;                                    	// load value of result

    if ((DstopIdx-DstartIdx)==0)  {                                   	// 1 byte data (binary)
      CIVresultL.value = (unsigned long)rxBuffer[DstartIdx];
    }
 
    if ((DstopIdx-DstartIdx)==1)  {                                   	// 2 byte data, BCD,
      for (idx = DstopIdx; idx >= DstartIdx; idx--) {                 	// first byte sent is of highest order
        CIVresultL.value += (rxBuffer[idx] & 0x0f) * mul; mul *= 10;
        CIVresultL.value += (rxBuffer[idx] >> 4) * mul; mul *= 10;
      }
    }
  
    if ((DstopIdx-DstartIdx) == 4){                                   	// 5 byte data, BCD, 
      for (idx = DstartIdx; idx <= DstopIdx; idx++) {                 	// first byte sent is of lowest order
        CIVresultL.value += (rxBuffer[idx] & 0x0f) * mul; mul *= 10;
        CIVresultL.value += (rxBuffer[idx] >> 4) * mul; mul *= 10;
      }
    }
  }

  logNewEntry(rxBuffer,"RX", CIVresultL.retVal);
  
  return CIVresultL;

} // readMsg



//::::::::: 
CIVresult_t CIVCLIENT::writeMsg (const uint8_t deviceAddr, const uint8_t cmd_body[], const uint8_t cmd_data[]) {

// this is the main function to write data to the master
// in client mode it is assumed, that this function is called almost immediately as an answer to
// every msg which has been received

	uint8_t idx;

  uint8_t txBuffer[CIV_BUFFERSIZE];


  CIVresult_t CIVresultL;

  CIVresultL.retVal        = CIV_OK;
  CIVresultL.address      = deviceAddr;
  CIVresultL.cmd[0]       = 0;
  CIVresultL.datafield[0] = 0;
  CIVresultL.value        = 0;          // will NOT be set in writeMsg!!!

  //............. build the complete command and write to txBuffer

  txBuffer[1] = C_START;txBuffer[2] = C_START;		// preamble into buffer
  txBuffer[3] = deviceAddr;                       // target address
  txBuffer[4] = _myCIVaddr;                       // source address
  txBuffer[0] = 4;                      					// length of data in use

  for (idx=1; idx<=cmd_body[0];idx++) {       		// command body into buffer
    txBuffer[0]++; txBuffer[txBuffer[0]] = cmd_body[idx];}

  for (idx=1; idx<=cmd_data[0];idx++) {       		// data part into buffer
    txBuffer[0]++; txBuffer[txBuffer[0]] = cmd_data[idx];}

  txBuffer[0]++; txBuffer[txBuffer[0]] = C_STOP;  // postamble into buffer


	//.............	let's get access to the CIV bus and write the command

  while (serAvailable()>0) {
    idx = serRead();
    if (idx==C_START) CIVresultL.retVal = CIV_BUS_BUSY;   // Startbyte found
    if (idx==C_STOP)  CIVresultL.retVal = CIV_OK;         // Stopbyte found 
  }
	if (CIVresultL.retVal==CIV_BUS_BUSY) {   // CIV bus is not available -> break
    logNewEntry(txBuffer,"CHK", CIVresultL.retVal); 
    return CIVresultL;
 	}

	for (idx=1; idx<=txBuffer[0];idx++) {serWrite(txBuffer[idx]);}

/* 
// left out because of the time consumption of this part; can be activeted 
// if the error messages are of special interest
 
  serflushOutput(); // wait, until the message really has been sent - this takes approx 5ms

	  //............. read the own command back, and check, whether the bytes were sent correctly
	  // there must be the complete command exactly as sent available in the rxBuffer

  rxBuffer[0]=0; waitCounter = 0;
  while ((rxBuffer[0]< txBuffer[0]) && (waitCounter<t_sendCmd)) { 
    waitCounter++; delayMicroseconds (t_usLoop);
    if (serAvailable()>0) {
      rxBuffer[0]++; rxBuffer[rxBuffer[0]] = serRead();
			// even if only one byte hasn't been sent correctly, the whole command is corrupted
      if (rxBuffer[rxBuffer[0]]!=txBuffer[rxBuffer[0]]) CIVresultL.retVal = CIV_BUS_CONFLICT;
    }
  }
	if (waitCounter>=t_sendCmd)           			// CIV bus is shortcut -> break
		{CIVresultL.retVal = CIV_HW_FAULT; logNewEntry(rxBuffer,"TX_S",CIVresultL.retVal); return CIVresultL;}
	else
		if (CIVresultL.retVal==CIV_BUS_CONFLICT)  	// CIV bus conflict -> break
			{logNewEntry(rxBuffer,"TX_C",CIVresultL.retVal); return CIVresultL;}
*/
  
  logNewEntry(txBuffer,"TXok",CIVresultL.retVal);

  for (idx=1; idx<=cmd_body[0];idx++) {           // command body into CIVresultL
    CIVresultL.cmd[0]++; CIVresultL.cmd[CIVresultL.cmd[0]] = cmd_body[idx];}

  for (idx=1; idx<=cmd_data[0];idx++) {           // data part into CIVresultL
    CIVresultL.datafield[0]++; CIVresultL.datafield[CIVresultL.datafield[0]] = cmd_data[idx];}

	return CIVresultL;

} // writeCmd

//::::::::::::: logging

#ifdef log_CIVclient
	uint8_t CIVCLIENT::logEntry=0;
	uint8_t CIVCLIENT::logBuffer[logMaxEntries][logMsgLength];
  uint8_t CIVCLIENT::logBufferState[logMaxEntries];
	char CIVCLIENT::logBufferName[logMaxEntries][logNameLength];
#endif


//.............
void CIVCLIENT::logClear() {
  #ifdef log_CIVclient
		uint8_t idx;
		for (idx=0; idx<logMaxEntries-1;idx++) { // clear all buffer entries
			logBuffer[idx][0]=0;
			logBufferName[idx][0]='\0';
		}
		logEntry = 0;
  #endif
}

//.............
void CIVCLIENT::logNewEntry(uint8_t msg[], const char name[], const uint8_t state) {

#ifdef log_CIVclient
	
	uint8_t idx;

  // copy the message into the log
  if (msg[0]>(logMsgLength-1)) msg[0] = (logMsgLength-1); // limit the msg to the length of the 
                                                          // log buffer
  for (idx=0;idx<=(msg[0]);idx++) {logBuffer[logEntry][idx] = msg[idx];}

  // copy the name of the entry into the log
  for (idx=0;idx<logNameLength; idx++)
	  logBufferName[logEntry][idx] = name[idx];
  logBufferName[logEntry][logNameLength-1] = '\0'; // just to be on the safe side ....

  // copy the state into the log
  logBufferState[logEntry] = state;
		
  if (logEntry<(logMaxEntries-1)) logEntry++;
	else logEntry=0;

#endif

}

//.............
void CIVCLIENT::logDisplayLine(uint8_t lineNo) {

#ifdef log_CIVclient

  uint8_t idx;
 
	for (idx=1;idx<=logBuffer[lineNo][0];idx++) {
		Serial.print(".");
		Serial.print(logBuffer[lineNo][idx],HEX);
	}
	Serial.print(" : ");
	Serial.print(logBufferName[lineNo]);
  Serial.print(" * ");
  Serial.print(logBufferState[lineNo]);
	Serial.println("");

#endif // log_CIVclient

}

//.............
void CIVCLIENT::logDisplay() {

#ifdef log_CIVclient

	uint8_t entry_idx;
	uint8_t display_idx=0;

	// display oldest lines first
	for (entry_idx = logEntry; entry_idx < logMaxEntries; entry_idx++) {
		if (logBuffer[entry_idx][0]>0) {
			logDisplayLine(entry_idx);
			display_idx++;
		}
	}
	// display latest lines last
	for (entry_idx = 0; entry_idx < logEntry; entry_idx++) {
		logDisplayLine(entry_idx);
		display_idx++;
	}
  if (display_idx>0) Serial.println("**");

#endif // log_CIVclient

}


//------------------------------------------------------------------------
// private methods

// in order to make the support of other serial-interfaces in the future easier,
// an extra set of internal CIV-bus access routines have been created

//::::::::: 
uint8_t CIVCLIENT::serAvailable() 		{return CIVclient_SERIAL.available();}

//::::::::: 
void CIVCLIENT::serWrite(uint8_t ch)  {CIVclient_SERIAL.write(ch);}

//::::::::: 
uint8_t CIVCLIENT::serRead() 			    {return CIVclient_SERIAL.read();}

//::::::::: 
void CIVCLIENT::serflushOutput(){
  #ifdef useAltSoftSerial
    CIVclient_SERIAL.flushOutput();
  #else
    CIVclient_SERIAL.flush();
  #endif
}

//------------------------------------------------------------------------
// private static variables
