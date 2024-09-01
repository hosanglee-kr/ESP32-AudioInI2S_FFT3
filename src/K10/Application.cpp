#include "Application.h"

#include <TFT_eSPI.h>

#ifdef AUDIOPROCESS_T1
    #include "K10/AudioProcessing/Processor.h"
#endif

#ifdef AUDIOPROCESS_T2
    #include "K10/AudioProcessT2/FFT_T2.h"
#endif


#include "K10/audio_input/ADCSampler.h"
#include "K10/audio_input/I2SMEMSSampler.h"
#include "K10/audio_input/I2SSampler.h"	 // I2SSampler.h"
#include "K10/UI/UI.h"
// #include "Processor.h"
#include "config.h"

// Task to process samples
void processing_task(void *param) {
	Application *application = (Application *)param;
	// just sit in a loop processing samples as quickly as possible
	while (true) {
		application->process_samples();
	}
}

Application::Application(TFT_eSPI &display) {
	m_window_size	= G_K10_WINDOW_SIZE;
	m_sample_buffer = (int16_t *)malloc(sizeof(int16_t) * G_K10_WINDOW_SIZE);
	m_ui			= new UI(display, m_window_size);

	#ifdef AUDIOPROCESS_T1
	    m_processor		= new Processor(m_window_size);
	#endif
	#ifdef AUDIOPROCESS_T2
	    m_processor		= new FFT_T2(m_window_size);
	#endif

	#ifdef G_K10_USE_I2S_MIC_INPUT
		m_sampler = new I2SMEMSSampler(I2S_NUM_0, i2s_mic_pins, i2s_mic_Config);
	#else
		m_sampler = new ADCSampler(ADC_UNIT_1, G_K10_ADC_MIC_CHANNEL, i2s_adc_config);
	#endif

	pinMode(G_K10_GPIO_BUTTON, INPUT_PULLUP);
}

void Application::begin() {
	// set up the processing
	TaskHandle_t processing_task_handle;
	xTaskCreatePinnedToCore(processing_task, "Processing Task", 4096, this, 2, &processing_task_handle, 0);

	// start sampling from i2s device
	m_sampler->start();
}

void Application::process_samples() {
	// grab the samples
	m_sampler->read(m_sample_buffer, G_K10_WINDOW_SIZE);
	m_processor->update(m_sample_buffer);
	m_ui->update(m_processor->m_fft_input, m_processor->m_energy);
}

void Application::loop() {
	if (digitalRead(G_K10_GPIO_BUTTON) == 0) {
		m_ui->toggle_display();
		// delay to allow for the touch to finish
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}
