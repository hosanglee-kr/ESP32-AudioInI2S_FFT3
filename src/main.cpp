#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

//#define DD10_FASTLED

#ifdef DD10_FASTLED
	#include "D10_FastLED_001.h"
#endif


//#define E10_FREQ

#ifdef E10_FREQ
	#include "E10_Frequencies_001.h"
#endif


//#define F10_FREQ

#ifdef F10_FREQ
	#include "F10_FrequencyRange_001.h"
#endif


//#define G10

#ifdef G10
	#include "G10_Basic-Visuals_001.h"
#endif


//#define H10

#ifdef H10
	#include "H10_Advanced-Visuals_001.h"
#endif


#define K10

#ifdef K10

	#include <Arduino.h>
	#include <TFT_eSPI.h>
	#include <freertos/FreeRTOS.h>
	#include <freertos/task.h>

	#include "K10/K10_Application.h"
	//#include "Application.h"

	Application *g_K10_App;
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

	#ifdef K10
		TFT_eSPI *display = new TFT_eSPI();
		display->begin();
		display->setRotation(1);

		g_K10_App = new Application(*display);
		g_K10_App->begin();
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

	#ifdef K10
		g_K10_App->loop();
	#endif
}
