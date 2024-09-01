// FastLED.ino
// By Shea Ivey

// Reads I2S microphone data into g_D10_samples[], processes them into frequency buckets and then outputs it to an LED strip for viewing.
// Try wistling differrent tones to see which frequency buckets they fall into.

// TIP: uncomment renderBeatRainbow() for a music beat visualization.

#include <AudioInI2S.h>

#define G_D10_SAMPLE_SIZE 1024  // Buffer size of read samples
#define G_D10_SAMPLE_RATE 44100 // Audio Sample Rate

/* Required defines for audio analysis */
#define G_D10_BAND_SIZE 8 // powers of 2 up to 64, defaults to 8

#include <AudioAnalysis.h>
AudioAnalysis g_D10_AudioInfo;

// ESP32 S2 Mini
// #define MIC_BCK_PIN 4             // Clock pin from the mic.
// #define MIC_WS_PIN 39             // WS pin from the mic.
// #define MIC_DATA_PIN 5            // SD pin data from the mic.
// #define MIC_CHANNEL_SELECT_PIN 40 // Left/Right pin to select the channel output from the mic.

// ESP32 TTGO T-Display
#define G_D10_MIC_BCK_PIN 32			  // Clock pin from the mic.
#define G_D10_MIC_WS_PIN 25			  // WS pin from the mic.
#define G_D10_MIC_DATA_PIN 33			  // SD pin data from the mic.
#define G_D10_MIC_CHANNEL_SELECT_PIN 27 // Left/Right pin to select the channel output from the mic.

AudioInI2S g_D10_Mic(G_D10_MIC_BCK_PIN, G_D10_MIC_WS_PIN, G_D10_MIC_DATA_PIN, G_D10_MIC_CHANNEL_SELECT_PIN); // defaults to RIGHT channel.

int32_t g_D10_samples[G_D10_SAMPLE_SIZE]; // I2S sample data is stored here

#include "FastLED.h"
#define G_D10_NUM_LEDS 144 // 1 meter strip
#define G_D10_LED_PIN 13

#define G_D10_MAX_BRIGHTNESS 80 // save your eyes
#define G_D10_FRAME_RATE 30

CRGB g_D10_Leds[G_D10_NUM_LEDS];
unsigned long g_D10_nextFrame = 0;
unsigned long g_D10_tick = 0;



void D10_processSamples();
void D10_renderBasicTest();
void D10_renderBeatRainbow();

void D10_FastLED_init(){

	// FastLED setup
	FastLED.addLeds<WS2812B, G_D10_LED_PIN, GRB>(g_D10_Leds, G_D10_NUM_LEDS);
	FastLED.setBrightness(G_D10_MAX_BRIGHTNESS);
	FastLED.show();
}

void D10_Audio_init(){
	g_D10_Mic.begin(G_D10_SAMPLE_SIZE, G_D10_SAMPLE_RATE); // Starts the I2S DMA port.

	// audio analysis setup
	g_D10_AudioInfo.setNoiseFloor(10);	   // sets the noise floor
	g_D10_AudioInfo.normalize(true, 0, 255); // normalize all values to range provided.

	g_D10_AudioInfo.autoLevel(AudioAnalysis::ACCELERATE_FALLOFF, 1, 255, 1000); // set auto level falloff rate
	g_D10_AudioInfo.bandPeakFalloff(AudioAnalysis::EXPONENTIAL_FALLOFF, 1);	  // set the band peak fall off rate
	g_D10_AudioInfo.vuPeakFalloff(AudioAnalysis::ACCELERATE_FALLOFF, 1);		  // set the volume unit peak fall off rate

}

void D10_init(){
	D10_Audio_init();

	D10_FastLED_init();
}


void D10_run(){
	if (g_D10_nextFrame > millis()){
		return;
	}
	// enforce a predictable frame rate
	g_D10_nextFrame = millis() + (1000 / G_D10_FRAME_RATE);
	g_D10_tick++;

	D10_processSamples(); // does all the reading and frequency calculations

	/* RENDER MODES */
	D10_renderBasicTest(); // bands and volume unit visualization

	// D10_renderBeatRainbow(); // uncommnet this line for a music beat visualization
}

void D10_processSamples(){
	g_D10_Mic.read(g_D10_samples); // Stores the current I2S port buffer into g_D10_samples.
	g_D10_AudioInfo.computeFFT(g_D10_samples, G_D10_SAMPLE_SIZE, G_D10_SAMPLE_RATE);
	g_D10_AudioInfo.computeFrequencies(G_D10_BAND_SIZE);
}

void D10_renderBasicTest(){

	float *bands = g_D10_AudioInfo.getBands();
	float *peaks = g_D10_AudioInfo.getPeaks();
	int vuMeter = g_D10_AudioInfo.getVolumeUnit();
	int vuMeterPeak = g_D10_AudioInfo.getVolumeUnitPeak();
	int vuMeterPeakMax = g_D10_AudioInfo.getVolumeUnitPeakMax();

	g_D10_Leds[G_D10_BAND_SIZE] = CRGB(0, 0, 0);

	// equilizer first G_D10_BAND_SIZE
	for (int i = 0; i < G_D10_BAND_SIZE; i++){
		g_D10_Leds[i] = CHSV(i * (200 / G_D10_BAND_SIZE), 255, (int)peaks[i]);
	}

	// volume unit meter rest of leds
	uint8_t vuLed = (uint8_t)map(vuMeter, 0, vuMeterPeakMax, G_D10_BAND_SIZE + 1, G_D10_NUM_LEDS - 1);
	uint8_t vuLedPeak = (uint8_t)map(vuMeterPeak, 0, vuMeterPeakMax, G_D10_BAND_SIZE + 1, G_D10_NUM_LEDS - 1);
	for (int i = G_D10_BAND_SIZE + 1; i < G_D10_NUM_LEDS; i++){
		g_D10_Leds[i] = CRGB(0, 0, 0);
		if (i < vuLed){
			g_D10_Leds[i] = i > (G_D10_NUM_LEDS - ((G_D10_NUM_LEDS - G_D10_BAND_SIZE) * 0.2)) ? CRGB(50, 0, 0) : CRGB(0, 50, 0);
		}
		if (i == vuLedPeak){
			g_D10_Leds[i] = CRGB(50, 50, 50);
		}
	}

	FastLED.show();
}

void D10_renderBeatRainbow(){
	float *bands = g_D10_AudioInfo.getBands();
	float *peaks = g_D10_AudioInfo.getPeaks();
	int peakBandIndex = g_D10_AudioInfo.getBandMaxIndex();
	int peakBandValue = g_D10_AudioInfo.getBand(peakBandIndex);
	static int beatCount = 0;

	bool beatDetected = false;
	bool clapsDetected = false;
	// beat detection
	if (peaks[0] == bands[0] && peaks[0] > 0){ // new peak for bass must be a beat
		beatCount++;
		beatDetected = true;
	}
	if (peakBandIndex >= G_D10_BAND_SIZE / 2 && peakBandValue > 0){
		clapsDetected = true;
	}

	for (int i = 0; i < G_D10_NUM_LEDS; i++){
		g_D10_Leds[i] = blend(g_D10_Leds[i], CRGB(0, 0, 0), 100); // fade to black over time

		// bass/beat = rainbow
		if (beatDetected){
			if (random(0, 10 - ((float)peaks[1] / (float)255 * 10.0)) == 0){
				g_D10_Leds[i] = CHSV((beatCount * 10) % 255, 255, 255);
			}
		}

		// claps/highs = white twinkles
		if (clapsDetected){
			if (random(0, 40 - ((float)peakBandIndex / (float)G_D10_BAND_SIZE * 10.0)) == 0){
				g_D10_Leds[i] = CRGB(255, 255, 255);
			}
		}
	}

	FastLED.show();
}