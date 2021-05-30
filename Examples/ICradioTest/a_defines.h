#ifndef defines_h
#define defines_h

/* 
ICradio_test - a_Defines.h
*/

// Global compile switches ===================================================================================

#define VERSION_STRING "CIVbusLib ICradioTest V0_6 21/05/30"

// common switches -----------------------------------

// if defined debug messages on the serial line will be generated
#define debug 

// some general defines ----------------------------------

enum onOff_t:uint8_t {
	OFF = 0,
	ON  = 1,
  UNDEF
};

enum keyPressed_t:uint8_t {
	NO_KEY_PRESSED 		= 0,
	KEY_BLK_PRESSED,
	KEY_RED_PRESSED,
  KEY_EIN_PRESSED,
  KEY_AUS_PRESSED,
  KEY_VOICE_PRESSED,
  KEY_DATA_PRESSED,
  KEY_TOGGLE_PRESSED,
  KEY_FREQ_PRESSED,
  KEY_LOG_PRESSED,
  KEY_X_PRESSED
};

// Mapping of portpins to function ===========================================================================

#define P_KEY_BLK      2 
#define P_KEY_RED      3

#define P_STATUS_LED   4

#define P_POWER_ON    12
#define P_INT_LED     13

// Debugging ...

#ifdef debug
  #define SET_TIME_MARKER1 G_timemarker1 = micros();
  #define EVAL_TIME_MARKER1 G_timemarker1a = micros();Serial.print("t1:  ");Serial.println(G_timemarker1a-G_timemarker1);
#endif

#ifndef debug
  #define SET_TIME_MARKER1
  #define EVAL_TIME_MARKER1
#endif

#endif // #ifndef defines_h
