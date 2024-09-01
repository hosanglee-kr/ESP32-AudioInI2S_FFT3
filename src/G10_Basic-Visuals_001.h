/*
	TTGO-Display.ino
	By Shea Ivey

	Reads I2S microphone data into g_G10_samples[], processes them into frequency buckets and then outputs it to an 135x240 TFT screen for viewing.
	Try wistling differrent tones to see which frequency buckets they fall into.

	Short Press Button 1 to change views
	1. Frequency Bars
	2. Oscilloscope

	Short Press Button 2 to change number of band buckets
	1. 2 bands
	2. 4 bands
	3. 8 bands
	4. 16 bands
	5. 32 bands
	6. 64 bands
*/

#include <AudioInI2S.h>

#define G_G10_SAMPLE_SIZE 1024  // Buffer size of read samples 23.219ms @ 44100 sample rate
#define G_G10_SAMPLE_RATE 44100 // Audio Sample Rate
#define G_G10_FRAME_RATE 43	  // frame every samples 23.219ms

/* Required defines for audio analysis */
#define G_G10_BAND_SIZE 64 // powers of 2 up to 64, defaults to 8
#include <AudioAnalysis.h>
AudioAnalysis g_G10_audioInfo;

#define G_G10_MIC_BCK_PIN 32			  // Clock pin from the mic.
#define G_G10_MIC_WS_PIN 25			  // WS pin from the mic.
#define G_G10_MIC_DATA_PIN 33			  // SD pin data from the mic.
#define G_G10_MIC_CHANNEL_SELECT_PIN 27 // Left/Right pin to select the channel output from the mic.

AudioInI2S g_G10_mic(G_G10_MIC_BCK_PIN, G_G10_MIC_WS_PIN, G_G10_MIC_DATA_PIN, G_G10_MIC_CHANNEL_SELECT_PIN); // defaults to RIGHT channel.

int32_t g_G10_samples[G_G10_SAMPLE_SIZE]; // I2S sample data is stored here

// ESP32 TTGO Display
#define DISPLAY_TFT


#define BUTTON_PIN1 35 // change view button
#define BUTTON_PIN2 0  // clear display

#define G_G10_SCREEN_WIDTH 240  // TFT display width, in pixels
#define G_G10_SCREEN_HEIGHT 135 // TFT display height, in pixels

#include <TFT_eSPI.h>
#include <Adafruit_GFX.h>
#include "FastLED.h"
GFXcanvas16 g_G10_canvas(G_G10_SCREEN_WIDTH, G_G10_SCREEN_HEIGHT); // 240x135 pixel canvas
#ifndef TFT_DISPOFF
	#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
	#define TFT_SLPIN 0x10
#endif

#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS 5
#define TFT_DC 16
#define TFT_RST 23

#define TFT_BL 4									  // Display backlight control pin
TFT_eSPI g_G10_Tft = TFT_eSPI(G_G10_SCREEN_HEIGHT, G_G10_SCREEN_WIDTH); // Invoke custom library


unsigned long g_G10_nextFrame = 0;

void G10_processSamples();
void G10_renderFrequencies();
void G10_renderOscilloscope();

void G10_Audio_init(){
	g_G10_mic.begin(G_G10_SAMPLE_SIZE, G_G10_SAMPLE_RATE); // Starts the I2S DMA port.
	

	// audio analysis setup
	g_G10_audioInfo.setNoiseFloor(10);					 // sets the noise floor
	g_G10_audioInfo.normalize(true, 0, G_G10_SCREEN_HEIGHT - 1); // normalize all values to range provided.

	g_G10_audioInfo.autoLevel(AudioAnalysis::EXPONENTIAL_FALLOFF, .0001, 20, -1); // set auto level falloff rate
	g_G10_audioInfo.bandPeakFalloff(AudioAnalysis::EXPONENTIAL_FALLOFF, .1);		// set the band peak fall off rate
	g_G10_audioInfo.vuPeakFalloff(AudioAnalysis::EXPONENTIAL_FALLOFF, .01);		// set the volume unit peak fall off rate

	g_G10_audioInfo.setEqualizerLevels(1, 1, 1); // set the equlizer offsets

}

void G10_TFT_init(){

	// TFT setup
	g_G10_Tft.init();
	g_G10_Tft.setRotation(1);
	g_G10_Tft.fillScreen(TFT_BLACK);
	g_G10_Tft.setTextSize(2);
	g_G10_Tft.setTextColor(TFT_WHITE);
	g_G10_Tft.setCursor(0, 0);
	g_G10_Tft.setTextDatum(MC_DATUM);
	g_G10_Tft.setTextSize(1);
	g_G10_Tft.setSwapBytes(true);
}

void G10_init(){
	G10_Audio_init();
	
	pinMode(BUTTON_PIN1, INPUT);
	pinMode(BUTTON_PIN2, INPUT);

}

float g_G10_frame = 0;
int g_G10_bandSize = G_G10_BAND_SIZE;

void G10_run(){
	static int mode = 0;
	if (g_G10_nextFrame > millis()){
		return;
	}

	g_G10_frame++;
	// enforce a predictable frame rate
	g_G10_nextFrame = millis() + (1000 / G_G10_FRAME_RATE);

	G10_processSamples(); // does all the reading and frequency calculations

	if (!digitalRead(BUTTON_PIN1))
	{
		while (!digitalRead(BUTTON_PIN1))
			;
		// change visual mode
		mode++;
		if (mode > 1)
		{
			mode = 0;
		}
	}

	if (!digitalRead(BUTTON_PIN2)){
		while (!digitalRead(BUTTON_PIN2))
			;
		// change number of frequency bands
		g_G10_bandSize *= 2;
		if (g_G10_bandSize > 64){
			g_G10_bandSize = 2;
		}
		g_G10_audioInfo.setBandSize(g_G10_bandSize);
	}

	/* RENDER MODES */
	switch (mode){
	case 1:
		G10_renderOscilloscope();
		break;
	case 0:
	default:
		G10_renderFrequencies(); // render raw samples
		break;
	}
}



void G10_processSamples(){
	g_G10_mic.read(g_G10_samples); // Stores the current I2S port buffer into samples.
	g_G10_audioInfo.computeFFT(g_G10_samples, G_G10_SAMPLE_SIZE, G_G10_SAMPLE_RATE);
	g_G10_audioInfo.computeFrequencies(g_G10_bandSize);
}

void G10_renderFrequencies(){
	g_G10_audioInfo.normalize(true, 0, G_G10_SCREEN_HEIGHT - 1); // normalize all values to range provided.
	float *bands = g_G10_audioInfo.getBands();
	float *peaks = g_G10_audioInfo.getPeaks();
	float *bandEq = g_G10_audioInfo.getEqualizerLevels();
	float vuMeter = g_G10_audioInfo.getVolumeUnit();
	float vuMeterPeak = g_G10_audioInfo.getVolumeUnitPeak();

	float bass = g_G10_audioInfo.getBass();
	float mid = g_G10_audioInfo.getMid();
	float treble = g_G10_audioInfo.getTreble();

	float bassPeak = g_G10_audioInfo.getBassPeak();
	float midPeak = g_G10_audioInfo.getMidPeak();
	float treblePeak = g_G10_audioInfo.getTreblePeak();

	g_G10_canvas.fillScreen(0x0000);
	// equilizer first G_G10_BAND_SIZE
	int offset = 0;
	int BAND_WIDTH = (G_G10_SCREEN_WIDTH - 36) / g_G10_audioInfo.getBandSize();
	int HALF_SCREEN = (float)G_G10_SCREEN_HEIGHT / 2.0;
	for (int i = 0; i < g_G10_audioInfo.getBandSize(); i++){
		// band frequency
		g_G10_canvas.fillRect(offset, (G_G10_SCREEN_HEIGHT - 1) - bands[i], BAND_WIDTH - 1, G_G10_SCREEN_HEIGHT + 2, 0xFFFFFF);
		g_G10_canvas.drawLine(offset, (G_G10_SCREEN_HEIGHT - 1) - peaks[i], offset + BAND_WIDTH - 2, (G_G10_SCREEN_HEIGHT - 1) - peaks[i], 0xFFFFFF);
		offset += BAND_WIDTH;
	}
	offset = 0;
	for (int i = 0; i < g_G10_audioInfo.getBandSize(); i++){
		// equlizer curve
		if (i != g_G10_audioInfo.getBandSize() - 1){
			g_G10_canvas.drawLine(
				offset + (BAND_WIDTH / 2),							 // x1
				(G_G10_SCREEN_HEIGHT - 1) - (HALF_SCREEN * bandEq[i]),	 // y1
				offset + BAND_WIDTH - 1 + (BAND_WIDTH / 2),			 // x2
				(G_G10_SCREEN_HEIGHT - 1) - (HALF_SCREEN * bandEq[i + 1]), // y1
				g_G10_Tft.color565(127, 127, 127)							 // color
			);
		}

		offset += BAND_WIDTH;
	}

	offset = G_G10_SCREEN_WIDTH - 34;
	g_G10_canvas.fillRect(offset, (G_G10_SCREEN_HEIGHT - 1) - bass, 3, G_G10_SCREEN_HEIGHT + 2, g_G10_Tft.color565(255, 0, 0));
	g_G10_canvas.drawLine(offset, (G_G10_SCREEN_HEIGHT - 1) - bassPeak, offset + 3, (G_G10_SCREEN_HEIGHT - 1) - bassPeak, 0xFFFFFF);

	offset += 4;
	g_G10_canvas.fillRect(offset, (G_G10_SCREEN_HEIGHT - 1) - mid, 3, G_G10_SCREEN_HEIGHT + 2, g_G10_Tft.color565(255, 255, 0));
	g_G10_canvas.drawLine(offset, (G_G10_SCREEN_HEIGHT - 1) - midPeak, offset + 3, (G_G10_SCREEN_HEIGHT - 1) - midPeak, 0xFFFFFF);

	offset += 4;
	g_G10_canvas.fillRect(offset, (G_G10_SCREEN_HEIGHT - 1) - treble, 3, G_G10_SCREEN_HEIGHT + 2, g_G10_Tft.color565(0, 255, 0));
	g_G10_canvas.drawLine(offset, (G_G10_SCREEN_HEIGHT - 1) - treblePeak, offset + 3, (G_G10_SCREEN_HEIGHT - 1) - treblePeak, 0xFFFFFF);

	// draw VU meter on right
	offset += 10;
	g_G10_canvas.fillRect(offset, (G_G10_SCREEN_HEIGHT - 1) - vuMeter, 12, G_G10_SCREEN_HEIGHT + 2, g_G10_Tft.color565(0, 255, 0));
	g_G10_canvas.drawLine(offset, (G_G10_SCREEN_HEIGHT - 1) - vuMeterPeak, offset + 12, (G_G10_SCREEN_HEIGHT - 1) - vuMeterPeak, 0xFFFFFF);

	g_G10_Tft.pushImage(0, 0, G_G10_SCREEN_WIDTH, G_G10_SCREEN_HEIGHT, g_G10_canvas.getBuffer());
}

void G10_renderOscilloscope(){
	g_G10_audioInfo.normalize(true, 0, G_G10_SCREEN_HEIGHT); // normalize all values to range provided.
	int triggerIndex = g_G10_audioInfo.getSampleTriggerIndex();
	g_G10_canvas.fillScreen(0x0000);
	int stepSize = (G_G10_SAMPLE_SIZE / 1.5) / G_G10_SCREEN_WIDTH;
	for (int i = triggerIndex + stepSize, x0 = 1; x0 < G_G10_SCREEN_WIDTH && i < G_G10_SAMPLE_SIZE; i += stepSize, x0++){
		int x1 = x0 - 1;
		int y0 = g_G10_audioInfo.getSample(i);
		int y1 = g_G10_audioInfo.getSample(i - stepSize);
		g_G10_canvas.drawLine(x0, y0, x1, y1, g_G10_Tft.color565(255, 255, 255));
	}
	g_G10_Tft.pushImage(0, 0, G_G10_SCREEN_WIDTH, G_G10_SCREEN_HEIGHT, g_G10_canvas.getBuffer());
}
