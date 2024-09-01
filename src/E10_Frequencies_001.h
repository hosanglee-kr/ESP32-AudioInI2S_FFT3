
// Frequencies.ino
// By Shea Ivey

// Reads I2S microphone data into g_E10_samples[],
// processes them into frequency buckets 
// and then outputs it to the serial plotter for viewing.
// Try wistling differrent tones to see which frequency buckets they fall into.

// TIP: uncomment the g_E10_audioInfo.autoLevel() to see how the loud and quiet noises are handled.


#include <AudioInI2S.h>

#define 	G_E10_SAMPLE_SIZE 	1024	// Buffer size of read samples
#define 	G_E10_SAMPLE_RATE 	44100 	// Audio Sample Rate

// Required defines for audio analysis 
#define 	G_E10_BAND_SIZE 8 			// powers of 2 up to 64, defaults to 8

#include <AudioAnalysis.h>
AudioAnalysis g_E10_audioInfo;

// ESP32 S2 Minisamples
// #define G_E10_MIC_BCK_PIN 4             // Clock pin from the mic
// #define G_E10_MIC_WS_PIN 39             // WS pin from the mic
// #define G_E10_MIC_DATA_PIN 5            // SD pin data from the mic
// #define G_E10_MIC_CHANNEL_SELECT_PIN 40 // Left/Right pin to select the channel output from the mic

// ESP32 TTGO T-Display
#define G_E10_MIC_BCK_PIN 32			// Clock pin from the mic
#define G_E10_MIC_WS_PIN 25				// WS pin from the mic
#define G_E10_MIC_DATA_PIN 33			// SD pin data from the mic
#define G_E10_MIC_CHANNEL_SELECT_PIN 27 // Left/Right pin to select the channel output from the mic

AudioInI2S g_E10_mic(G_E10_MIC_BCK_PIN, G_E10_MIC_WS_PIN, G_E10_MIC_DATA_PIN, G_E10_MIC_CHANNEL_SELECT_PIN); // defaults to RIGHT channel.

int32_t g_E10_samples[G_E10_SAMPLE_SIZE]; // I2S sample data is stored here

void E10_init(){

	g_E10_mic.begin(G_E10_SAMPLE_SIZE, G_E10_SAMPLE_RATE, I2S_NUM_0); // Starts the I2S DMA port.

	// audio analysis setup
	g_E10_audioInfo.setNoiseFloor(10);		 // sets the noise floor
	g_E10_audioInfo.normalize(true, 0, 255); // normalize all values to range provided.

	// g_E10_audioInfo.autoLevel(AudioAnalysis::ACCELERATE_FALLOFF, 1); // uncomment this line to set auto level falloff rate
	g_E10_audioInfo.bandPeakFalloff(AudioAnalysis::EXPONENTIAL_FALLOFF, 0.05); // set the band peak fall off rate
	g_E10_audioInfo.vuPeakFalloff(AudioAnalysis::EXPONENTIAL_FALLOFF, 0.05);   // set the volume unit peak fall off rate
}

void E10_Audio_Process(){
	g_E10_mic.read(g_E10_samples); // Stores the current I2S port buffer into g_E10_samples.

	g_E10_audioInfo.computeFFT(g_E10_samples, G_E10_SAMPLE_SIZE, G_E10_SAMPLE_RATE);
	g_E10_audioInfo.computeFrequencies(G_E10_BAND_SIZE);

}

void E10_Audio_Print_Band(){
	float *bands 		= g_E10_audioInfo.getBands();
	float *peaks 		= g_E10_audioInfo.getPeaks();

	float vuMeter 		= g_E10_audioInfo.getVolumeUnit();
	float vuMeterPeak 	= g_E10_audioInfo.getVolumeUnitPeak();

	// Send data to serial plotter
	for (int i = 0; i < G_E10_BAND_SIZE; i++){
		Serial.printf("%dHz:%.1f,", g_E10_audioInfo.getBandName(i), peaks[i]);
	}

	// also send the vu meter data
	Serial.printf("vuValue:%.1f,vuPeak:%.2f", vuMeter, vuMeterPeak);

	Serial.println();
}

void E10_run(){
	E10_Audio_Process();

	E10_Audio_Print_Band();
}
