#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#define E10_FREQ

#ifdef E10_FREQ
	#include "E10_Frequencies_001.h"
#endif


#define F10_FREQ

#ifdef F10_FREQ
	#include "F10_FrequencyRange_001.h"
#endif


void setup(){
	Serial.begin(115200);


	#ifdef E10_FREQ
		E10_init();
	#endif

	#ifdef F10_FREQ
		F10_init();
	#endif
}

void loop(){

	
	#ifdef E10_FREQ
		E10_run();
	#endif

	#ifdef F10_FREQ
		F10_run();
	#endif
}
