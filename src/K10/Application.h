#pragma once

#include <driver/i2s.h>

// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>

//	#define G_K10_AUDIOPROCESS_T1
//	#define G_K10_AUDIOPROCESS_T2



#include "K10_config.h"

class UI;

#ifdef G_K10_AUDIOPROCESS_T1
    class FFT_T1;
#endif

#ifdef G_K10_AUDIOPROCESS_T2
    class FFT_T2;
#endif

//src/K10/AudioProcessT2/FFT_T2.h

class I2SSampler;
class TFT_eSPI;

class Application {
private:
	int			m_window_size;
	int16_t	   *m_sample_buffer;
	UI		   *m_ui;

        #ifdef G_K10_AUDIOPROCESS_T1
            FFT_T1  *m_processor;
        #endif

        #ifdef G_K10_AUDIOPROCESS_T2
           FFT_T2   *m_processor;
        #endif


	I2SSampler *m_sampler;

	void process_samples();

public:
	Application(TFT_eSPI &display);
	void begin();
	void loop();

	friend void processing_task(void *param);
};
