
	// FrequenciesRange.ino
	// By Shea Ivey

	// Reads I2S microphone data into g_F10_samples[], processes them into frequency buckets and then outputs it to the serial plotter for viewing.
	// Try wistling differrent tones to see which frequency buckets they fall into.

	// TIP: uncomment the g_F10_audioInfo.autoLevel() to see how the loud and quiet noises are handled.

	// OUTPUT EXAMPLE:
	// maxFrequency:   775, vuRaw:   164.23, vuNorm: 186.12, vuPeak: 199.68, bassRaw:     0.00, bassNorm:   0.00, bassPeak:   8.48, midRaw:   164.23, midNorm: 215.05, midPeak: 230.73, highRaw:     0.00, highNorm:   0.00, highPeak:   0.00
	// maxFrequency:   775, vuRaw:    93.24, vuNorm: 105.67, vuPeak: 186.12, bassRaw:     0.00, bassNorm:   0.00, bassPeak:   0.00, midRaw:    93.24, midNorm: 122.10, midPeak: 215.05, highRaw:     0.00, highNorm:   0.00, highPeak:   0.00
	// maxFrequency:   775, vuRaw:   196.48, vuNorm: 222.67, vuPeak: 222.67, bassRaw:    15.58, bassNorm:  20.40, bassPeak:  20.40, midRaw:   180.90, midNorm: 236.89, midPeak: 236.89, highRaw:     0.00, highNorm:   0.00, highPeak:   0.00
	// maxFrequency:   775, vuRaw:   214.23, vuNorm: 242.78, vuPeak: 242.78, bassRaw:     0.00, bassNorm:   0.00, bassPeak:  18.54, midRaw:   214.23, midNorm: 255.00, midPeak: 255.00, highRaw:     0.00, highNorm:   0.00, highPeak:   0.00
	// maxFrequency:   775, vuRaw:   188.86, vuNorm: 214.04, vuPeak: 242.79, bassRaw:     7.65, bassNorm:   9.11, bassPeak:   9.11, midRaw:   181.21, midNorm: 215.70, midPeak: 255.00, highRaw:     0.00, highNorm:   0.00, highPeak:   0.00
	// ... looping



#include <AudioInI2S.h>



#define G_F10_SAMPLE_SIZE 1024  // Buffer size of read samples
#define G_F10_SAMPLE_RATE 44100 // Audio Sample Rate

#include <AudioFrequencyAnalysis.h>
AudioFrequencyAnalysis g_F10_audioInfo;

// ESP32 S2 Mini
// #define G_F10_MIC_BCK_PIN 4             // Clock pin from the mic.
// #define G_F10_MIC_WS_PIN 39             // WS pin from the mic.
// #define G_F10_MIC_DATA_PIN 5            // SD pin data from the mic.
// #define G_F10_MIC_CHANNEL_SELECT_PIN 40 // Left/Right pin to select the channel output from the mic.

// ESP32 TTGO T-Display
#define G_F10_MIC_BCK_PIN 32			  // Clock pin from the mic.
#define G_F10_MIC_WS_PIN 25			  // WS pin from the mic.
#define G_F10_MIC_DATA_PIN 33			  // SD pin data from the mic.
#define G_F10_MIC_CHANNEL_SELECT_PIN 27 // Left/Right pin to select the channel output from the mic.

AudioInI2S g_F10_Mic(G_F10_MIC_BCK_PIN, G_F10_MIC_WS_PIN, G_F10_MIC_DATA_PIN, G_F10_MIC_CHANNEL_SELECT_PIN); // defaults to RIGHT channel.

int32_t g_F10_samples[G_F10_SAMPLE_SIZE]; // I2S sample data is stored here

FrequencyRange g_F10_vuMeter(0, 20000);
FrequencyRange g_F10_bass(0, 249);
FrequencyRange g_F10_mid(250, 1499);
FrequencyRange g_F10_high(1500, 16000);

void F10_init(){
	g_F10_Mic.begin(G_F10_SAMPLE_SIZE, G_F10_SAMPLE_RATE); // Starts the I2S DMA port.

	// audio analysis setup
	g_F10_audioInfo.setNoiseFloor(1); // sets the noise floor

	g_F10_vuMeter._inIsolation = true; // isolates the vu meters min/max from the rest of the frequency ranges

	// register the frequency ranges to g_F10_audioInfo
	g_F10_audioInfo.addFrequencyRange(&g_F10_vuMeter);
	g_F10_audioInfo.addFrequencyRange(&g_F10_bass);
	g_F10_audioInfo.addFrequencyRange(&g_F10_mid);
	g_F10_audioInfo.addFrequencyRange(&g_F10_high);
}

void F10_run(){

	g_F10_Mic.read(g_F10_samples); // Stores the current I2S port buffer into g_F10_samples.
	g_F10_audioInfo.loop(g_F10_samples, G_F10_SAMPLE_SIZE, G_F10_SAMPLE_RATE);

	// also send the vu meter data
	Serial.printf("maxFrequency: %4d, ", g_F10_vuMeter.getMaxFrequency()); // returns the frequency with the highest amplitude
	Serial.printf("vuRaw: %8.2f, vuNorm: %6.2f, vuPeak: %6.2f, ", g_F10_vuMeter.getValue(), g_F10_vuMeter.getValue(0, 255), g_F10_vuMeter.getPeak(0, 255));
	Serial.printf("bassRaw: %8.2f, bassNorm: %6.2f, bassPeak: %6.2f, ", g_F10_bass.getValue(), g_F10_bass.getValue(0, 255), g_F10_bass.getPeak(0, 255));
	Serial.printf("midRaw: %8.2f, midNorm: %6.2f, midPeak: %6.2f, ", g_F10_mid.getValue(), g_F10_mid.getValue(0, 255), g_F10_mid.getPeak(0, 255));
	Serial.printf("highRaw: %8.2f, highNorm: %6.2f, highPeak: %6.2f", g_F10_high.getValue(), g_F10_high.getValue(0, 255), g_F10_high.getPeak(0, 255));

	Serial.println();
}
