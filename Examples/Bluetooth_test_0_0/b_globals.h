/*
Afu Ctrl - b_Globals  Global Variables; shall only be included into masterfile of the project
*/

#include <ICradio.h>


/* module wide variables ----------------------------------------------------*/

// Debugging ...
#ifdef debug
  unsigned long G_timemarker1;
  unsigned long G_timemarker1a;

  unsigned long G_timemarker2;
  unsigned long G_timemarker2a;
#endif

unsigned long G_CAT_freq = 0;

byte G_curr_sel_band_O = NUM_BANDS; 

byte G_IC705_state_O = RADIO_NDEF;

byte G_IC705_mode_O  = MODE_NDEF;


byte G_time[3] = {0, 0x20, 0x44};             // 20:44 (Laenge 0 verhindert, daß beim Arduinostart bei
                                              // laufendem IC7300 die 20:44 reingeschrieben werden)
byte G_date[5] = {0, 0x20, 0x20, 0x11, 0x06}; // 2020-11-06 (Laenge 0 verhindert, daß beim Arduinostart bei
                                              // laufendem IC7300 die 2020-11-06 reingeschrieben werden)
byte G_UTCdelta[4] = {3, 0x01,0x00,0x00};     // 1h vor (bei Winterzeit)


const char *DISPLAY_ON_OFF[6] = {
"OFF",
"ON ",
"OFF TR",     /* Transit up */
"ON TR",      /* Transit dn */
"ND",
"TG"
};

const char *DISPLAY_O_U[2] = {
"oben ",
"unten"  
};

/* timer  variables */
unsigned long time_current_baseloop;       /* temporary time of the baseloop entry for calculations */

unsigned long time_last_baseloop;          /* wird geschrieben, wenn die Baseloop wieder arbeitet */
