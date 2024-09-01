#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>


#define DD10_FASTLED

#ifdef DD10_FASTLED
	#include "D10_FastLED_001.h"
#endif


#define E10_FREQ

#ifdef E10_FREQ
	#include "E10_Frequencies_001.h"
#endif


#define F10_FREQ

#ifdef F10_FREQ
	#include "F10_FrequencyRange_001.h"
#endif


#define G10

#ifdef G10
	#include "G10_Basic-Visuals_001.h"
#endif


#define H10

#ifdef H10
	#include "H10_Advanced-Visuals_001.h"
#endif

void setup(){
	Serial.begin(115200);

	#ifdef DD10_FASTLED
		D10_init();
	#endif


	#ifdef E10_FREQ
		E10_init();
	#endif

	#ifdef F10_FREQ
		F10_init();
	#endif

	#ifdef G10
		G10_init();
	#endif

	#ifdef H10
		H10_init();
	#endif

}

void loop(){

	
	#ifdef DD10_FASTLED
		D10_run();
	#endif

	#ifdef E10_FREQ
		E10_run();
	#endif

	#ifdef F10_FREQ
		F10_run();
	#endif

	#ifdef G10
		G10_run();
	#endif

	#ifdef H10
		H10_run();
	#endif

}
