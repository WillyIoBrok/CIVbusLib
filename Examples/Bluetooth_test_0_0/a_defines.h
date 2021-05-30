#ifndef defines_h
#define defines_h

/* 
AfuCtrl_test - a_Defines.h
*/

// Global compile switches ===================================================================================

#define VERSION_STRING "Bluetooth_test V0_0 21/05/30"

// common switches -----------------------------------

// if defined debug messages on the serial line will be generated
#define debug 

// some general defines ----------------------------------

enum onOff_t:uint8_t {
	OFF = 0,
	ON  = 1,
  NDEF
};

enum stnPosition_t:uint8_t {
  STN_UPSTAIRS = 0,
  STN_DOWNSTAIRS
};

enum keyPressed_t:uint8_t {
  NO_KEY_PRESSED    = 0,
  KEY_1_PRESSED,
  KEY_2_PRESSED
};




#define NUM_BANDS 24   /* Number of Bands (depending on the radio) */

// Mapping of portpins to function ===========================================================================

#define P_INT_LED          2
#define P_CONN_RADIO_LED   4
#define P_CONN_MQTT_LED   14
#define P_Key_1           39
#define P_Key_2           36


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
