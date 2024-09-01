/*
	TTGO-Display.ino
	By Shea Ivey

	Reads I2S microphone data into g_H10_samples[], processes them into frequency buckets and then outputs it to an 135x240 TFT screen for viewing.
	Try wistling differrent tones to see which frequency buckets they fall into.

	Short Press Button 1 to change views
	1. Frequency Bars
	2. Oscilloscope
	3. Oscilloscope Circle
	4. Frequency Radar
	5. Triangles
	6. Spectrograph
	7. Band Matrix
	8. Apple Bars

	Long Press Button 1 go back to previous view

	Short Press Button 2 to change number of band buckets
	1. 2 bands
	2. 4 bands
	3. 8 bands
	4. 16 bands
	5. 32 bands
	6. 64 bands

	Long Button 2 to change fade out effect
	1. No Effect
	2. fade out to black
	3. fade out rainbow
	4. fade up and down from center rainbow
	5. fade in to center rainbow
	6. fade out all directions from center rainbow
*/

#include <AudioInI2S.h>

#define G_H10_SAMPLE_SIZE 1024  // Buffer size of read samples 23.219ms @ 44100 sample rate
#define G_H10_SAMPLE_RATE 44100 // Audio Sample Rate
#define G_H10_FRAME_RATE 120	  // every frame 8ms

/* Required defines for audio analysis */
#define G_H10_BAND_SIZE 64 // powers of 2 up to 64, defaults to 8
#include <AudioAnalysis.h>
AudioAnalysis g_H10_AudioInfo;


#define G_H10_MIC_BCK_PIN 32			  // Clock pin from the mic.
#define G_H10_MIC_WS_PIN 25			  // WS pin from the mic.
#define G_H10_MIC_DATA_PIN 33			  // SD pin data from the mic.
#define G_H10_MIC_CHANNEL_SELECT_PIN 27 // Left/Right pin to select the channel output from the mic.

#define G_H10_BUTTON_PIN1 35 // change view button
#define G_H10_BUTTON_PIN2 0  // clear display

AudioInI2S g_H10_Mic(G_H10_MIC_BCK_PIN, G_H10_MIC_WS_PIN, G_H10_MIC_DATA_PIN, G_H10_MIC_CHANNEL_SELECT_PIN); // defaults to RIGHT channel.

int32_t g_H10_samples[G_H10_SAMPLE_SIZE]; // I2S sample data is stored here

// ESP32 TTGO Display
#define DISPLAY_TFT

#define G_H10_SCREEN_WIDTH 240  // TFT display width, in pixels
#define G_H10_SCREEN_HEIGHT 135 // TFT display height, in pixels
#include <TFT_eSPI.h>
#include <Adafruit_GFX.h>
#include "FastLED.h"
GFXcanvas16 g_H10_Canvas(G_H10_SCREEN_WIDTH, G_H10_SCREEN_HEIGHT); // 240x135 pixel canvas
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
TFT_eSPI g_H10_Tft = TFT_eSPI(G_H10_SCREEN_HEIGHT, G_H10_SCREEN_WIDTH); // Invoke custom library

unsigned long g_H10_nextFrame = 0;



void H10_clearDisplay();
void H10_processSamples();
void H10_renderFrequencies();
void H10_renderTriangles();
void H10_renderBassMidTrebleLines();
void H10_renderFrequencyLines();
void H10_renderEnergyLines();
void H10_renderHeightHistory();
void H10_renderAppleFrequencies();
void H10_renderRadarFrequencies();
void H10_renderFrequenciesHeatmap();
void H10_renderFrequenciesMatrix();
void H10_renderOscilloscope();
void H10_renderCircleOscilloscope();

uint16_t H10_getValueColor(float value, float min, float max);


void H10_init(){

	g_H10_Mic.begin(G_H10_SAMPLE_SIZE, G_H10_SAMPLE_RATE); // Starts the I2S DMA port.
	pinMode(G_H10_BUTTON_PIN1, INPUT);
	pinMode(G_H10_BUTTON_PIN2, INPUT);

	// audio analysis setup
	g_H10_AudioInfo.setNoiseFloor(5);						 // sets the noise floor
	g_H10_AudioInfo.normalize(true, 0, G_H10_SCREEN_HEIGHT - 1); // normalize all values to range provided.

	g_H10_AudioInfo.autoLevel(AudioAnalysis::EXPONENTIAL_FALLOFF, .001, 20, -1); // set auto level falloff rate
	g_H10_AudioInfo.bandPeakFalloff(AudioAnalysis::EXPONENTIAL_FALLOFF, 1);	   // set the band peak fall off rate
	g_H10_AudioInfo.vuPeakFalloff(AudioAnalysis::EXPONENTIAL_FALLOFF, .01);	   // set the volume unit peak fall off rate

	g_H10_AudioInfo.setEqualizerLevels(1, 1, 1); // set the equlizer offsets

	// TFT setup
	g_H10_Tft.init();
	g_H10_Tft.setRotation(1);
	g_H10_Tft.fillScreen(TFT_BLACK);
	g_H10_Tft.setTextSize(2);
	g_H10_Tft.setTextColor(TFT_WHITE);
	g_H10_Tft.setCursor(0, 0);
	g_H10_Tft.setTextDatum(MC_DATUM);
	g_H10_Tft.setTextSize(1);
	g_H10_Tft.setSwapBytes(true);
}

float g_H10_frame = 0;
int g_H10_bandSize = G_H10_BAND_SIZE;
int g_H10_visualMode = 0;
int g_H10_clearMode = 0;

void H10_run(){
	if (g_H10_nextFrame > millis()){
		return;
	}
	g_H10_frame++;
	// enforce a predictable frame rate
	g_H10_nextFrame = millis() + (1000 / G_H10_FRAME_RATE);

	H10_processSamples(); // does all the reading and frequency calculations

	if (!digitalRead(G_H10_BUTTON_PIN1)){
		unsigned long t = millis();
		while (!digitalRead(G_H10_BUTTON_PIN1))
			;
		t = millis() - t;
		if (t < 250)
		{ // short press
			g_H10_visualMode++;
			if (g_H10_visualMode > 10)
			{
				g_H10_visualMode = 0;
			}
		}
		else
		{ // long press
			g_H10_visualMode--;
			if (g_H10_visualMode < 0)
			{
				g_H10_visualMode = 10;
			}
		}
	}

	if (!digitalRead(G_H10_BUTTON_PIN2))
	{
		unsigned long t = millis();
		while (!digitalRead(G_H10_BUTTON_PIN2))
			;
		t = millis() - t;
		if (t < 250)
		{ // short press
			g_H10_bandSize *= 2;
			if (g_H10_bandSize > 64)
			{
				g_H10_bandSize = 2;
			}
			g_H10_AudioInfo.setBandSize(g_H10_bandSize);
		}
		else
		{ // long press
			g_H10_clearMode++;
			if (g_H10_clearMode > 5)
			{
				g_H10_clearMode = 0;
			}
		}
	}

	/* RENDER MODES */
	switch (g_H10_visualMode){
	case 1:
		H10_renderAppleFrequencies(); // renders all the bands similar to Apple music visualization bars
		break;
	case 2:
		H10_renderOscilloscope(); // render raw samples
		break;
	case 3:
		H10_renderCircleOscilloscope();
		break;
	case 4:
		H10_renderRadarFrequencies();
		break;
	case 5:
		H10_renderBassMidTrebleLines();
		break;
	case 6:
		H10_renderEnergyLines();
		break;
	case 7:
		H10_renderTriangles();
		break;
	case 8:
		H10_renderFrequenciesHeatmap();
		break;
	case 9:
		H10_renderHeightHistory();
		break;
	case 10:
		H10_renderFrequenciesMatrix();
		break;
	case 0:
	default:
		H10_renderFrequencies();
		break;
	}
}


void H10_processSamples(){
	g_H10_Mic.read(g_H10_samples); // Stores the current I2S port buffer into samples.
	g_H10_AudioInfo.computeFFT(g_H10_samples, G_H10_SAMPLE_SIZE, G_H10_SAMPLE_RATE);
	g_H10_AudioInfo.computeFrequencies(g_H10_bandSize);
}

void H10_renderFrequencies(){
	g_H10_AudioInfo.normalize(true, 0, G_H10_SCREEN_HEIGHT - 1); // normalize all values to range provided.
	float *bands = g_H10_AudioInfo.getBands();
	float *peaks = g_H10_AudioInfo.getPeaks();
	float *bandEq = g_H10_AudioInfo.getEqualizerLevels();
	float vuMeter = g_H10_AudioInfo.getVolumeUnit();
	float vuMeterPeak = g_H10_AudioInfo.getVolumeUnitPeak();

	float bass = g_H10_AudioInfo.getBass();
	float mid = g_H10_AudioInfo.getMid();
	float treble = g_H10_AudioInfo.getTreble();

	float bassPeak = g_H10_AudioInfo.getBassPeak();
	float midPeak = g_H10_AudioInfo.getMidPeak();
	float treblePeak = g_H10_AudioInfo.getTreblePeak();

	g_H10_Canvas.fillScreen(0x0000);
	// equilizer first G_H10_BAND_SIZE
	int offset = 0;
	int BAND_WIDTH = (G_H10_SCREEN_WIDTH - 36) / g_H10_AudioInfo.getBandSize();
	int HALF_SCREEN = (float)G_H10_SCREEN_HEIGHT / 2.0;
	for (int i = 0; i < g_H10_AudioInfo.getBandSize(); i++)
	{
		g_H10_Canvas.fillRect(offset, (G_H10_SCREEN_HEIGHT - 1) - bands[i], BAND_WIDTH - 1, G_H10_SCREEN_HEIGHT + 2, H10_getValueColor(bands[i], 0, G_H10_SCREEN_HEIGHT - 1));
		g_H10_Canvas.drawLine(offset, (G_H10_SCREEN_HEIGHT - 1) - peaks[i], offset + BAND_WIDTH - 2, (G_H10_SCREEN_HEIGHT - 1) - peaks[i], 0xFFFFFF);
		offset += BAND_WIDTH;
	}
	offset = 0;
	for (int i = 0; i < g_H10_AudioInfo.getBandSize(); i++)
	{
		// equlizer curve
		if (i != g_H10_AudioInfo.getBandSize() - 1)
		{
			g_H10_Canvas.drawLine(
				offset + (BAND_WIDTH / 2),							 // x1
				(G_H10_SCREEN_HEIGHT - 1) - (HALF_SCREEN * bandEq[i]),	 // y1
				offset + BAND_WIDTH - 1 + (BAND_WIDTH / 2),			 // x2
				(G_H10_SCREEN_HEIGHT - 1) - (HALF_SCREEN * bandEq[i + 1]), // y1
				g_H10_Tft.color565(25, 25, 25)							 // color
			);
		}

		offset += BAND_WIDTH;
	}

	offset = G_H10_SCREEN_WIDTH - 34;
	g_H10_Canvas.fillRect(offset, (G_H10_SCREEN_HEIGHT - 1) - bass, 3, G_H10_SCREEN_HEIGHT + 2, g_H10_Tft.color565(255, 0, 0));
	g_H10_Canvas.drawLine(offset, (G_H10_SCREEN_HEIGHT - 1) - bassPeak, offset + 3, (G_H10_SCREEN_HEIGHT - 1) - bassPeak, 0xFFFFFF);

	offset += 4;
	g_H10_Canvas.fillRect(offset, (G_H10_SCREEN_HEIGHT - 1) - mid, 3, G_H10_SCREEN_HEIGHT + 2, g_H10_Tft.color565(255, 255, 0));
	g_H10_Canvas.drawLine(offset, (G_H10_SCREEN_HEIGHT - 1) - midPeak, offset + 3, (G_H10_SCREEN_HEIGHT - 1) - midPeak, 0xFFFFFF);

	offset += 4;
	g_H10_Canvas.fillRect(offset, (G_H10_SCREEN_HEIGHT - 1) - treble, 3, G_H10_SCREEN_HEIGHT + 2, g_H10_Tft.color565(0, 255, 0));
	g_H10_Canvas.drawLine(offset, (G_H10_SCREEN_HEIGHT - 1) - treblePeak, offset + 3, (G_H10_SCREEN_HEIGHT - 1) - treblePeak, 0xFFFFFF);

	// draw VU meter on right
	offset += 10;
	g_H10_Canvas.fillRect(offset, (G_H10_SCREEN_HEIGHT - 1) - vuMeter, 12, G_H10_SCREEN_HEIGHT + 2, g_H10_Tft.color565(0, 255, 0));
	g_H10_Canvas.drawLine(offset, (G_H10_SCREEN_HEIGHT - 1) - vuMeterPeak, offset + 12, (G_H10_SCREEN_HEIGHT - 1) - vuMeterPeak, 0xFFFFFF);

	g_H10_Tft.pushImage(0, 0, G_H10_SCREEN_WIDTH, G_H10_SCREEN_HEIGHT, g_H10_Canvas.getBuffer());
}

void H10_renderTriangles(){
	float raw[3] = {g_H10_AudioInfo.getBass(), g_H10_AudioInfo.getMid(), g_H10_AudioInfo.getTreble()};
	float peak[3] = {g_H10_AudioInfo.getBassPeak(), g_H10_AudioInfo.getMidPeak(), g_H10_AudioInfo.getTreblePeak()};
	H10_clearDisplay();
	g_H10_AudioInfo.normalize(true, 0, 20); // normalize all values to range provided.
	float radius;
	float step = (360.0 / (3)) * PI / 180.0;
	static float offsetAngle = (90) * PI / 180.0;
	offsetAngle += raw[0] * 0.005;
	offsetAngle -= raw[2] * 0.0075;
	float x0, x1, x_first;
	float y0, y1, y_first;
	float cx = G_H10_SCREEN_WIDTH / 2;
	float cy = G_H10_SCREEN_HEIGHT / 2;
	int OFFSET = 43;
	for (float angle = offsetAngle, i = 0; angle - offsetAngle <= 2 * PI && i < 3; angle += step, i++)
	{
		radius = 5 + peak[(int)i];
		x0 = cx + radius * cos(angle);
		y0 = cy + radius * sin(angle);
		if (i > 0)
		{
			for (int jx = -2; jx <= 2; jx++)
			{
				for (int jy = -1; jy <= 1; jy++)
				{
					g_H10_Canvas.drawLine(x0 + (jx * OFFSET), y0 + (jy * OFFSET), x1 + (jx * OFFSET), y1 + (jy * OFFSET), g_H10_Tft.color565(255, 255, 255));
				}
			}
		}
		else
		{
			x_first = x0;
			y_first = y0;
		}

		x1 = x0;
		y1 = y0;
	}
	for (int jx = -2; jx <= 2; jx++)
	{
		for (int jy = -1; jy <= 1; jy++)
		{
			g_H10_Canvas.drawLine(x0 + (jx * OFFSET), y0 + (jy * OFFSET), x_first + (jx * OFFSET), y_first + (jy * OFFSET), g_H10_Tft.color565(255, 255, 255));
		}
	}
	// g_H10_Canvas.drawLine(x0, y0, x_first, y_first, g_H10_Tft.color565(255, 255, 255));

	g_H10_Tft.pushImage(0, 0, G_H10_SCREEN_WIDTH, G_H10_SCREEN_HEIGHT, g_H10_Canvas.getBuffer());
}

void H10_renderBassMidTrebleLines(){
	g_H10_AudioInfo.normalize(true, 0, 1); // normalize all values to range provided.
	float values[3] = {g_H10_AudioInfo.getBass(), g_H10_AudioInfo.getMid(), g_H10_AudioInfo.getTreble()};
	int x0, x1, y0, y1;
	float wStep = G_H10_SCREEN_WIDTH / 2;
	H10_clearDisplay();
	for (int i = 0; i < 3 - 1; i++)
	{
		x0 = (float)i * wStep;
		y0 = values[i] * (G_H10_SCREEN_HEIGHT / 4.0);
		x1 = (float)(i + 1.0) * wStep;
		y1 = values[i + 1] * (G_H10_SCREEN_HEIGHT / 4.0);
		g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 + y0, x1, G_H10_SCREEN_HEIGHT / 2 + y1, g_H10_Tft.color565(255, 255, 255));
		g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 - y0, x1, G_H10_SCREEN_HEIGHT / 2 - y1, g_H10_Tft.color565(255, 255, 255));
		g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 + y0 + 1, x1, G_H10_SCREEN_HEIGHT / 2 + y1 + 1, g_H10_Tft.color565(255, 255, 255));
		g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 - y0 - 1, x1, G_H10_SCREEN_HEIGHT / 2 - y1 - 1, g_H10_Tft.color565(255, 255, 255));

		for (int j = 2; j < 10; j += 1)
		{
			y0 = values[i] * (G_H10_SCREEN_HEIGHT / 4.0) * (j) + 10;
			y1 = values[i + 1] * (G_H10_SCREEN_HEIGHT / 4.0) * (j) + 10;
			g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 + y0, x1, G_H10_SCREEN_HEIGHT / 2 + y1, g_H10_Tft.color565(values[0] * 255, values[1] * 255, values[2] * 255));
			g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 - y0, x1, G_H10_SCREEN_HEIGHT / 2 - y1, g_H10_Tft.color565(values[0] * 255, values[1] * 255, values[2] * 255));
		}
	}

	g_H10_Tft.pushImage(0, 0, G_H10_SCREEN_WIDTH, G_H10_SCREEN_HEIGHT, g_H10_Canvas.getBuffer());
}

void H10_renderFrequencyLines(){
	g_H10_AudioInfo.normalize(true, 0, 1); // normalize all values to range provided.
	float *values = g_H10_AudioInfo.getPeaks();
	float lowMidHigh[3] = {g_H10_AudioInfo.getBass(), g_H10_AudioInfo.getMid(), g_H10_AudioInfo.getTreble()};
	int x0, x1, y0, y1;
	float wStep = (float)G_H10_SCREEN_WIDTH / ((float)g_H10_AudioInfo.getBandSize() - 1.0);
	H10_clearDisplay();
	for (int i = 0; i < g_H10_AudioInfo.getBandSize() - 1; i++)
	{
		x0 = (float)i * wStep;
		y0 = values[i] * (G_H10_SCREEN_HEIGHT / 4.0);
		x1 = (float)(i + 1.0) * wStep;
		y1 = values[i + 1] * (G_H10_SCREEN_HEIGHT / 4.0);
		g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 + y0, x1, G_H10_SCREEN_HEIGHT / 2 + y1, g_H10_Tft.color565(255, 255, 255));
		g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 - y0, x1, G_H10_SCREEN_HEIGHT / 2 - y1, g_H10_Tft.color565(255, 255, 255));
		g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 + y0 + 1, x1, G_H10_SCREEN_HEIGHT / 2 + y1 + 1, g_H10_Tft.color565(255, 255, 255));
		g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 - y0 - 1, x1, G_H10_SCREEN_HEIGHT / 2 - y1 - 1, g_H10_Tft.color565(255, 255, 255));

		for (int j = 2; j < 5; j += 1)
		{
			y0 = values[i] * (G_H10_SCREEN_HEIGHT / 4.0) * (j) + 20;
			y1 = values[i + 1] * (G_H10_SCREEN_HEIGHT / 4.0) * (j) + 20;
			g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 + y0, x1, G_H10_SCREEN_HEIGHT / 2 + y1, g_H10_Tft.color565(lowMidHigh[0] * 255, lowMidHigh[1] * 255, lowMidHigh[2] * 255));
			g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 - y0, x1, G_H10_SCREEN_HEIGHT / 2 - y1, g_H10_Tft.color565(lowMidHigh[0] * 255, lowMidHigh[1] * 255, lowMidHigh[2] * 255));
		}
	}

	g_H10_Tft.pushImage(0, 0, G_H10_SCREEN_WIDTH, G_H10_SCREEN_HEIGHT, g_H10_Canvas.getBuffer());
}

void H10_renderEnergyLines(){
	g_H10_AudioInfo.normalize(true, 0, 1.0f); // normalize all values to range provided.
	float raw[3] = {g_H10_AudioInfo.getBass(), g_H10_AudioInfo.getMid(), g_H10_AudioInfo.getTreble()};
	float peak[3] = {g_H10_AudioInfo.getBassPeak(), g_H10_AudioInfo.getMidPeak(), g_H10_AudioInfo.getTreblePeak()};
	float vuPeak = g_H10_AudioInfo.getVolumeUnit();
	uint16_t c565 = g_H10_Tft.color565(peak[0] * 255, peak[1] * 255, peak[2] * 255);
	int x0, x1, y0, y1, cx, cy;
	float value;
	int wStep = G_H10_SCREEN_WIDTH / 4;
	H10_clearDisplay();
	// //for(int j = 2; j >= 0; j--) {
	//   for (int i = 0; i < 10; i++)
	//   {
	//     // if(j == 0) {
	//     //   c565 = g_H10_Tft.color565(peak[0] * 255, 0, 0);
	//     // }
	//     // else if (j == 1)
	//     // {
	//     //   c565 = g_H10_Tft.color565(0, peak[1] * 255, 0);
	//     // }
	//     // else
	//     // {
	//     //   c565 = g_H10_Tft.color565(0, 0, peak[2] * 255);
	//     // }

	//     //value = peak[j];
	//     value = vuPeak;
	//     cx = (G_H10_SCREEN_WIDTH / 2) + ((i) * (wStep * (value)));
	//     cy = 50.0 * value;
	//     g_H10_Canvas.drawLine(cx, G_H10_SCREEN_HEIGHT / 2 - cy, cx, G_H10_SCREEN_HEIGHT / 2 + cy, c565);
	//     cx = (G_H10_SCREEN_WIDTH / 2) - ((i) * (wStep * (value)));
	//     g_H10_Canvas.drawLine(cx, G_H10_SCREEN_HEIGHT / 2 - cy, cx, G_H10_SCREEN_HEIGHT / 2 + cy, c565);
	//   }
	// //}

	for (int i = 0; i < 10; i++)
	{
		// rifht side
		c565 = H10_getValueColor(peak[1] + peak[2], 0, 2);
		cx = (G_H10_SCREEN_WIDTH / 2) + (i * (wStep * ((peak[0] + peak[1]) + 0.25f)));
		cy = 25;
		// x0 = cx + wStep * 0.5 * (peak[2] + peak[1]);
		// g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 - cy, x0, G_H10_SCREEN_HEIGHT / 2 + cy, c565);
		// x0 = cx - wStep * 0.5 * (peak[2] + peak[1]);
		// g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 - cy, x0, G_H10_SCREEN_HEIGHT / 2 + cy, c565);
		g_H10_Canvas.drawLine(cx, G_H10_SCREEN_HEIGHT / 2 - cy, cx, G_H10_SCREEN_HEIGHT / 2 + cy, H10_getValueColor(peak[0] + peak[1] + peak[2], 0, 1));

		// left side
		cx = (G_H10_SCREEN_WIDTH / 2) - (i * (wStep * ((peak[0] + peak[1]) + 0.25f)));
		// x0 = cx + wStep * 0.5 * (peak[2] + peak[1]);
		// g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 - cy, x0, G_H10_SCREEN_HEIGHT / 2 + cy, c565);
		// x0 = cx - wStep * 0.5 * (peak[2] + peak[1]);
		// g_H10_Canvas.drawLine(x0, G_H10_SCREEN_HEIGHT / 2 - cy, x0, G_H10_SCREEN_HEIGHT / 2 + cy, c565);

		g_H10_Canvas.drawLine(cx, G_H10_SCREEN_HEIGHT / 2 - cy, cx, G_H10_SCREEN_HEIGHT / 2 + cy, H10_getValueColor(peak[0] + peak[1] + peak[2], 0, 1));
	}

	g_H10_Tft.pushImage(0, 0, G_H10_SCREEN_WIDTH, G_H10_SCREEN_HEIGHT, g_H10_Canvas.getBuffer());
}

void H10_renderHeightHistory(){
	g_H10_AudioInfo.normalize(true, 0, 255); // normalize all values to range provided.
	float *values = g_H10_AudioInfo.getBands();
	float vu = g_H10_AudioInfo.getVolumeUnit();
	float bassMidHigh[3] = {g_H10_AudioInfo.getBass(), g_H10_AudioInfo.getMid(), g_H10_AudioInfo.getTreble()};

	uint16_t *buffer = g_H10_Canvas.getBuffer();
	uint16_t c565;
	uint16_t step = 1;
	// shift value to the left
	for (int y = 0; y < G_H10_SCREEN_HEIGHT; y++)
	{
		for (int x = step + G_H10_SCREEN_WIDTH % step; x < G_H10_SCREEN_WIDTH; x += step)
		{
			c565 = buffer[y * G_H10_SCREEN_WIDTH + x];
			buffer[(y * G_H10_SCREEN_WIDTH) + x - 1] = c565;
		}
	}
	float radius;
	float x0, x1;
	float y0, y1;

	g_H10_Canvas.drawLine(G_H10_SCREEN_WIDTH - 1, 0, G_H10_SCREEN_WIDTH - 1, G_H10_SCREEN_HEIGHT, 0);

	if (g_H10_AudioInfo.getBandSize() == 32)
	{ // vu
		step = (G_H10_SCREEN_HEIGHT) / 1.5;
		x0 = G_H10_SCREEN_WIDTH - 1;
		x1 = G_H10_SCREEN_WIDTH - 1;
		y0 = ((G_H10_SCREEN_HEIGHT - step) / 2) + ((float)(step / 2.0) * (1.0 - (float)(vu / 255.0)));
		y1 = ((G_H10_SCREEN_HEIGHT - step) / 2) + step - ((float)(step / 2.0) * (1.0 - (float)(vu / 255.0)));
		c565 = H10_getValueColor(vu, 0, 255);
		g_H10_Canvas.drawLine(x0, y0, x1, y1, c565);
	}
	else if (g_H10_AudioInfo.getBandSize() == 64)
	{ // low mid high
		step = (G_H10_SCREEN_HEIGHT / 3);
		for (int i = 0; i < 3; i++)
		{
			x0 = G_H10_SCREEN_WIDTH - 1;
			x1 = G_H10_SCREEN_WIDTH - 1;
			y0 = (3 - 1 - i) * step + ((float)(step / 2.0) * (1.0 - (float)(bassMidHigh[i] / 255.0)));
			y1 = (3 - 1 - i) * step + step - ((float)(step / 2.0) * (1.0 - (float)(bassMidHigh[i] / 255.0)));
			c565 = H10_getValueColor(bassMidHigh[i], 0, 255);
			g_H10_Canvas.drawLine(x0, y0, x1, y1, c565);
		}
	}
	else
	{
		step = (G_H10_SCREEN_HEIGHT / g_H10_AudioInfo.getBandSize());
		for (int i = 0; i < g_H10_AudioInfo.getBandSize(); i++)
		{
			x0 = G_H10_SCREEN_WIDTH - 1;
			x1 = G_H10_SCREEN_WIDTH - 1;
			y0 = (g_H10_AudioInfo.getBandSize() - 1 - i) * step + ((float)(step / 2.0) * (1.0 - (float)(values[i] / 255.0)));
			y1 = (g_H10_AudioInfo.getBandSize() - 1 - i) * step + step - ((float)(step / 2.0) * (1.0 - (float)(values[i] / 255.0)));
			c565 = H10_getValueColor(values[i], 0, 255);
			g_H10_Canvas.drawLine(x0, y0, x1, y1, c565);
		}
	}
	g_H10_Tft.pushImage(0, 0, G_H10_SCREEN_WIDTH, G_H10_SCREEN_HEIGHT, g_H10_Canvas.getBuffer());
}

void H10_renderAppleFrequencies(){
	g_H10_AudioInfo.normalize(true, 0, G_H10_SCREEN_HEIGHT / 2 - 1); // normalize all values to range provided.
	float *bands = g_H10_AudioInfo.getBands();
	float *peaks = g_H10_AudioInfo.getPeaks();
	float *bandEq = g_H10_AudioInfo.getEqualizerLevels();
	float vuMeter = g_H10_AudioInfo.getVolumeUnit();
	float vuMeterPeak = g_H10_AudioInfo.getVolumeUnitPeak();

	float bass = g_H10_AudioInfo.getBass();
	float mid = g_H10_AudioInfo.getMid();
	float treble = g_H10_AudioInfo.getTreble();

	float bassPeak = g_H10_AudioInfo.getBassPeak();
	float midPeak = g_H10_AudioInfo.getMidPeak();
	float treblePeak = g_H10_AudioInfo.getTreblePeak();

	H10_clearDisplay();
	// equilizer first g_H10_AudioInfo.getBandSize()
	int BAND_WIDTH = (G_H10_SCREEN_WIDTH - 36) / g_H10_AudioInfo.getBandSize();
	int START_OFFSET = (G_H10_SCREEN_WIDTH - (BAND_WIDTH * g_H10_AudioInfo.getBandSize())) / 2;
	int HALF_SCREEN = (float)G_H10_SCREEN_HEIGHT / 2.0;

	int offset = START_OFFSET;
	for (int i = 0; i < g_H10_AudioInfo.getBandSize(); i++)
	{
		// band frequency
		CHSV hsv = CHSV(i * (230 / g_H10_AudioInfo.getBandSize()), 255, 255);
		CRGB c = hsv;
		uint16_t c16 = g_H10_Tft.color565(255, 255, 255); // g_H10_Tft.color565(c.r, c.g, c.b); // colored bars
		int h = peaks[i];
		int w = max((BAND_WIDTH / 2), 1);
		g_H10_Canvas.fillRoundRect(offset + ((BAND_WIDTH - w) / 2), (HALF_SCREEN)-h, w, h + 1, w / 4, c16);
		g_H10_Canvas.fillRoundRect(offset + ((BAND_WIDTH - w) / 2), (HALF_SCREEN) + 1, w, h + 1, w / 4, c16);
		g_H10_Canvas.fillRect(offset + ((BAND_WIDTH - w) / 2), (HALF_SCREEN) - (h / 2), w, h, c16);
		offset += BAND_WIDTH;
	}
	offset = START_OFFSET;
	for (int i = 0; i < g_H10_AudioInfo.getBandSize(); i++)
	{
		// equlizer curve
		if (i != g_H10_AudioInfo.getBandSize() - 1)
		{
			// upper
			g_H10_Canvas.drawLine(
				offset + (BAND_WIDTH / 2),							   // x1
				(HALF_SCREEN - 1) - (HALF_SCREEN / 2 * bandEq[i]),	   // y1
				offset + BAND_WIDTH - 1 + (BAND_WIDTH / 2),			   // x2
				(HALF_SCREEN - 1) - (HALF_SCREEN / 2 * bandEq[i + 1]), // y1
				g_H10_Tft.color565(25, 25, 25)							   // color
			);
			// lower
			g_H10_Canvas.drawLine(
				offset + (BAND_WIDTH / 2),								// x1
				(HALF_SCREEN - 1) - (HALF_SCREEN / 2 * -bandEq[i]),		// y1
				offset + BAND_WIDTH - 1 + (BAND_WIDTH / 2),				// x2
				(HALF_SCREEN - 1) - (HALF_SCREEN / 2 * -bandEq[i + 1]), // y1
				g_H10_Tft.color565(25, 25, 25)								// color
			);
		}

		offset += BAND_WIDTH;
	}

	g_H10_Tft.pushImage(0, 0, G_H10_SCREEN_WIDTH, G_H10_SCREEN_HEIGHT, g_H10_Canvas.getBuffer());
}

void H10_renderRadarFrequencies(){
	g_H10_AudioInfo.normalize(true, 0, G_H10_SCREEN_HEIGHT / 2); // normalize all values to range provided.
	float *peaks = g_H10_AudioInfo.getPeaks();
	H10_clearDisplay();
	float radius;
	float step = (360.0 / (g_H10_AudioInfo.getBandSize())) * PI / 180.0;
	float offsetAngle = (90) * PI / 180.0;
	float x0, x1, x_first;
	float y0, y1, y_first;
	float cx = G_H10_SCREEN_WIDTH / 2;
	float cy = G_H10_SCREEN_HEIGHT / 2;

	for (float angle = offsetAngle, i = 0; angle - offsetAngle <= 2 * PI && i < g_H10_AudioInfo.getBandSize(); angle += step, i++)
	{
		radius = peaks[(int)i];
		x0 = cx + radius * cos(angle);
		y0 = cy + radius * sin(angle);
		if (i > 0)
		{
			g_H10_Canvas.drawLine(x0, y0, x1, y1, g_H10_Tft.color565(255, 255, 255));
		}
		else
		{
			x_first = x0;
			y_first = y0;
		}

		x1 = x0;
		y1 = y0;
	}
	g_H10_Canvas.drawLine(x0, y0, x_first, y_first, g_H10_Tft.color565(255, 255, 255));

	g_H10_Tft.pushImage(0, 0, G_H10_SCREEN_WIDTH, G_H10_SCREEN_HEIGHT, g_H10_Canvas.getBuffer());
}

void H10_renderFrequenciesHeatmap(){
	g_H10_AudioInfo.normalize(true, 0, 255); // normalize all values to range provided.
	float *values = g_H10_AudioInfo.getBands();

	uint16_t *buffer = g_H10_Canvas.getBuffer();
	uint16_t c565;
	// shift value to the right
	for (int y = 0; y < G_H10_SCREEN_HEIGHT; y++)
	{
		for (int x = 1; x < G_H10_SCREEN_WIDTH; x++)
		{
			c565 = buffer[y * G_H10_SCREEN_WIDTH + x];
			buffer[(y * G_H10_SCREEN_WIDTH) + x - 1] = c565;
		}
	}
	float radius;
	float step = (G_H10_SCREEN_HEIGHT / g_H10_AudioInfo.getBandSize());
	float x0, x1;
	float y0, y1;

	g_H10_Canvas.drawLine(G_H10_SCREEN_WIDTH - 1, 0, G_H10_SCREEN_WIDTH - 1, G_H10_SCREEN_HEIGHT, 0);
	for (int i = 0; i < g_H10_AudioInfo.getBandSize(); i++)
	{
		x0 = G_H10_SCREEN_WIDTH - 1;
		x1 = G_H10_SCREEN_WIDTH - 1;
		y0 = (g_H10_AudioInfo.getBandSize() - 1 - i) * step;
		y1 = (g_H10_AudioInfo.getBandSize() - 1 - i) * step + step;
		c565 = H10_getValueColor(values[i], 0, 255);
		g_H10_Canvas.drawLine(x0, y0, x1, y1, c565);
	}

	g_H10_Tft.pushImage(0, 0, G_H10_SCREEN_WIDTH, G_H10_SCREEN_HEIGHT, g_H10_Canvas.getBuffer());
}

void H10_renderFrequenciesMatrix(){
	g_H10_AudioInfo.normalize(true, 0, 255); // normalize all values to range provided.
	float *values = g_H10_AudioInfo.getPeaks();

	uint16_t *buffer = g_H10_Canvas.getBuffer();
	uint16_t c565;

	// shift value to the right
	H10_clearDisplay();
	float radius;
	int stepX, wTiles;
	int stepY, hTiles;
	float x0, x1;
	float y0, y1;

	switch (g_H10_AudioInfo.getBandSize())
	{
	case 2:
		wTiles = 2;
		hTiles = 1;
		break;
	case 4:
		wTiles = 2;
		hTiles = 2;
		break;
	case 8:
		wTiles = 4;
		hTiles = 2;
		break;
	case 16:
		wTiles = 4;
		hTiles = 4;
		break;
	case 32:
		wTiles = 8;
		hTiles = 4;
		break;
	case 64:
		wTiles = 8;
		hTiles = 8;
		break;
	}

	stepX = G_H10_SCREEN_WIDTH / wTiles;
	stepY = G_H10_SCREEN_HEIGHT / hTiles;
	for (int y = 0; y < hTiles; y++)
	{
		for (int x = 0; x < wTiles; x++)
		{
			int i = y * hTiles + x;
			if (values[i] > 80)
			{
				x0 = x * stepX;
				y0 = y * stepY;
				g_H10_Canvas.fillRect(x0, y0, stepX - 1, stepY - 1, H10_getValueColor(values[i], 0, 255));
			}
		}
	}
	g_H10_Tft.pushImage(0, 0, G_H10_SCREEN_WIDTH, G_H10_SCREEN_HEIGHT, g_H10_Canvas.getBuffer());
}

void H10_renderOscilloscope(){
	g_H10_AudioInfo.normalize(true, -G_H10_SCREEN_HEIGHT / 2, G_H10_SCREEN_HEIGHT / 2); // normalize all values to range provided.
	int triggerIndex = g_H10_AudioInfo.getSampleTriggerIndex();
	H10_clearDisplay();
	int stepSize = (G_H10_SAMPLE_SIZE / 1.5) / G_H10_SCREEN_WIDTH;
	float scale = 0;
	float fadeInOutWidth = G_H10_SCREEN_WIDTH / 8;
	for (int i = triggerIndex + stepSize, x0 = 1; x0 < G_H10_SCREEN_WIDTH && i < G_H10_SAMPLE_SIZE; i += stepSize, x0++)
	{
		int x1 = x0 - 1;
		int y0 = g_H10_AudioInfo.getSample(i);
		int y1 = g_H10_AudioInfo.getSample(i - stepSize);
		if (x1 < fadeInOutWidth)
		{
			scale = (float)x0 / (fadeInOutWidth);
			y0 = (float)y0 * scale;
			scale = (float)x1 / (fadeInOutWidth);
			y1 = (float)y1 * scale;
		}
		else if (x1 > G_H10_SCREEN_WIDTH - fadeInOutWidth)
		{
			scale = (float)(G_H10_SCREEN_WIDTH - x0) / (fadeInOutWidth);
			y0 = (float)y0 * scale;
			scale = (float)(G_H10_SCREEN_WIDTH - x1) / (fadeInOutWidth);
			y1 = (float)y1 * scale;
		}
		g_H10_Canvas.drawLine(x0, y0 + G_H10_SCREEN_HEIGHT / 2, x1, y1 + G_H10_SCREEN_HEIGHT / 2, g_H10_Tft.color565(255, 255, 255));
	}
	g_H10_Tft.pushImage(0, 0, G_H10_SCREEN_WIDTH, G_H10_SCREEN_HEIGHT, g_H10_Canvas.getBuffer());
}

void H10_renderCircleOscilloscope(){
	g_H10_AudioInfo.normalize(true, -G_H10_SCREEN_HEIGHT / 4, G_H10_SCREEN_HEIGHT / 4); // normalize all values to range provided.
	int triggerIndex = g_H10_AudioInfo.getSampleTriggerIndex();
	float radius;
	float step = (360.0 / (G_H10_SAMPLE_SIZE / 2.0)) * PI / 180.0;
	float offsetAngle = (90) * PI / 180.0;
	float x0, x1, x_first;
	float y0, y1, y_first;
	float cx = G_H10_SCREEN_WIDTH / 2;
	float cy = G_H10_SCREEN_HEIGHT / 2;
	float scale = 0;
	float fadeInOutWidth = G_H10_SAMPLE_SIZE / 2 / 8;
	H10_clearDisplay();
	for (float angle = offsetAngle, i = triggerIndex; angle - offsetAngle <= 2 * PI && i < triggerIndex + G_H10_SAMPLE_SIZE / 2.0; angle += step, i++)
	{
		// TODO: blend with beginning more smoothly
		if (i < (triggerIndex + fadeInOutWidth))
		{
			scale = (float)i / (triggerIndex + fadeInOutWidth);
			radius = g_H10_AudioInfo.getSample(i) * scale;
			radius += g_H10_AudioInfo.getSample((triggerIndex + G_H10_SAMPLE_SIZE / 2.0) - (i - triggerIndex) + 1) * (1 - scale);
		}
		else
		{
			radius = g_H10_AudioInfo.getSample(i);
		}
		x0 = cx + (radius + G_H10_SCREEN_HEIGHT / 4) * cos(angle);
		y0 = cy + (radius + G_H10_SCREEN_HEIGHT / 4) * sin(angle);
		if (i > triggerIndex)
		{
			g_H10_Canvas.drawLine(x0, y0, x1, y1, g_H10_Tft.color565(255, 255, 255));
		}
		else
		{
			x_first = x0;
			y_first = y0;
		}

		x1 = x0;
		y1 = y0;
	}
	g_H10_Canvas.drawLine(x0, y0, x_first, y_first, g_H10_Tft.color565(255, 255, 255));

	g_H10_Tft.pushImage(0, 0, G_H10_SCREEN_WIDTH, G_H10_SCREEN_HEIGHT, g_H10_Canvas.getBuffer());
}

/*
  This function just clears the display in fun ways.
  Press button two to cycle thriough all the clearing modes.
*/
void H10_clearDisplay(){
	uint16_t *buffer = g_H10_Canvas.getBuffer();
	uint16_t c565;
	int r = 0, g = 0, b = 0;

	uint32_t rm = sin8(g_H10_frame / 2 + 85) / 3;
	uint32_t gm = sin8(g_H10_frame / 2) / 3;
	uint32_t bm = sin8(g_H10_frame / 2 + 170) / 3;
	switch (g_H10_clearMode)
	{
	case 1:
		for (int i = 0; i < G_H10_SCREEN_HEIGHT * G_H10_SCREEN_WIDTH; i++)
		{
			c565 = buffer[i];
			// ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
			r = ((c565 >> 11) & 0b00011111) << 3;
			g = ((c565 >> 5) & 0b00111110) << 2;
			b = ((c565) & 0b00011111) << 3;

			r -= 60;
			g -= 60;
			b -= 60;

			if (r < 20)
				r = 0;
			if (g < 20)
				g = 0;
			if (b < 20)
				b = 0;
			buffer[i] = g_H10_Tft.color565(r, g, b);
		}
		break;
	case 2:
		for (int i = 0; i < G_H10_SCREEN_HEIGHT * G_H10_SCREEN_WIDTH; i++)
		{
			c565 = buffer[i];
			// ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
			r = ((c565 >> 11) & 0b00011111) << 3;
			g = ((c565 >> 5) & 0b00111110) << 2;
			b = ((c565) & 0b00011111) << 3;

			r -= rm;
			g -= gm;
			b -= bm;

			if (r < 20)
				r = 0;
			if (g < 20)
				g = 0;
			if (b < 20)
				b = 0;
			buffer[i] = g_H10_Tft.color565(r, g, b);
		}
		break;
	case 3:
		for (int y = 1; y < G_H10_SCREEN_HEIGHT / 2 + 1; y++)
		{
			for (int x = 0; x < G_H10_SCREEN_WIDTH; x++)
			{
				c565 = buffer[y * G_H10_SCREEN_WIDTH + x];
				r = ((c565 >> 11) & 0b00011111) << 3;
				g = ((c565 >> 5) & 0b00111110) << 2;
				b = ((c565) & 0b00011111) << 3;

				r -= rm;
				g -= gm;
				b -= bm;

				if (r < 20)
					r = 0;
				if (g < 20)
					g = 0;
				if (b < 20)
					b = 0;
				buffer[y * G_H10_SCREEN_WIDTH + x] = g_H10_Tft.color565(r, g, b);
				buffer[((y - 1) * G_H10_SCREEN_WIDTH) + x] = g_H10_Tft.color565(r, g, b);
			}
		}
		for (int y = G_H10_SCREEN_HEIGHT - 1; y > G_H10_SCREEN_HEIGHT / 2 - 1; y--)
		{
			for (int x = 0; x < G_H10_SCREEN_WIDTH; x++)
			{
				c565 = buffer[y * G_H10_SCREEN_WIDTH + x];
				r = ((c565 >> 11) & 0b00011111) << 3;
				g = ((c565 >> 5) & 0b00111110) << 2;
				b = ((c565) & 0b00011111) << 3;

				r -= rm;
				g -= gm;
				b -= bm;

				if (r < 20)
					r = 0;
				if (g < 20)
					g = 0;
				if (b < 20)
					b = 0;
				buffer[y * G_H10_SCREEN_WIDTH + x] = g_H10_Tft.color565(r, g, b);
				buffer[((y + 1) * G_H10_SCREEN_WIDTH) + x] = g_H10_Tft.color565(r, g, b);
			}
		}
		break;
	case 4:
		for (int y = G_H10_SCREEN_HEIGHT / 2; y >= 0; y--)
		{
			for (int x = 0; x < G_H10_SCREEN_WIDTH; x++)
			{
				c565 = buffer[y * G_H10_SCREEN_WIDTH + x];
				r = ((c565 >> 11) & 0b00011111) << 3;
				g = ((c565 >> 5) & 0b00111110) << 2;
				b = ((c565) & 0b00011111) << 3;

				r -= rm;
				g -= gm;
				b -= bm;

				if (r < 20)
					r = 0;
				if (g < 20)
					g = 0;
				if (b < 20)
					b = 0;
				buffer[y * G_H10_SCREEN_WIDTH + x] = g_H10_Tft.color565(r, g, b);
				buffer[((y + 1) * G_H10_SCREEN_WIDTH) + x] = g_H10_Tft.color565(r, g, b);
			}
		}
		for (int y = G_H10_SCREEN_HEIGHT / 2; y < G_H10_SCREEN_HEIGHT; y++)
		{
			for (int x = 0; x < G_H10_SCREEN_WIDTH; x++)
			{
				c565 = buffer[y * G_H10_SCREEN_WIDTH + x];
				r = ((c565 >> 11) & 0b00011111) << 3;
				g = ((c565 >> 5) & 0b00111110) << 2;
				b = ((c565) & 0b00011111) << 3;

				r -= rm;
				g -= gm;
				b -= bm;

				if (r < 20)
					r = 0;
				if (g < 20)
					g = 0;
				if (b < 20)
					b = 0;
				buffer[y * G_H10_SCREEN_WIDTH + x] = g_H10_Tft.color565(r, g, b);
				buffer[((y - 1) * G_H10_SCREEN_WIDTH) + x] = g_H10_Tft.color565(r, g, b);
			}
		}
		break;
	case 5:
		for (int y = 1; y < G_H10_SCREEN_HEIGHT / 2 + 1; y++)
		{
			for (int x = 1; x < G_H10_SCREEN_WIDTH / 2 + 1; x++)
			{
				c565 = buffer[y * G_H10_SCREEN_WIDTH + x];
				r = ((c565 >> 11) & 0b00011111) << 3;
				g = ((c565 >> 5) & 0b00111110) << 2;
				b = ((c565) & 0b00011111) << 3;

				r -= rm;
				g -= gm;
				b -= bm;

				if (r < 20)
					r = 0;
				if (g < 20)
					g = 0;
				if (b < 20)
					b = 0;
				buffer[y * G_H10_SCREEN_WIDTH + x] = g_H10_Tft.color565(r, g, b);
				buffer[((y - 1) * G_H10_SCREEN_WIDTH) + x - 1] = g_H10_Tft.color565(r, g, b);
			}
		}
		for (int y = G_H10_SCREEN_HEIGHT - 1; y > G_H10_SCREEN_HEIGHT / 2 - 1; y--)
		{
			for (int x = 1; x < G_H10_SCREEN_WIDTH / 2 + 1; x++)
			{
				c565 = buffer[y * G_H10_SCREEN_WIDTH + x];
				r = ((c565 >> 11) & 0b00011111) << 3;
				g = ((c565 >> 5) & 0b00111110) << 2;
				b = ((c565) & 0b00011111) << 3;

				r -= rm;
				g -= gm;
				b -= bm;

				if (r < 20)
					r = 0;
				if (g < 20)
					g = 0;
				if (b < 20)
					b = 0;
				buffer[y * G_H10_SCREEN_WIDTH + x] = g_H10_Tft.color565(r, g, b);
				buffer[((y + 1) * G_H10_SCREEN_WIDTH) + x - 1] = g_H10_Tft.color565(r, g, b);
			}
		}
		for (int y = 1; y < G_H10_SCREEN_HEIGHT / 2 + 1; y++)
		{
			for (int x = G_H10_SCREEN_WIDTH - 2; x > G_H10_SCREEN_WIDTH / 2 - 1; x--)
			{
				c565 = buffer[y * G_H10_SCREEN_WIDTH + x];
				r = ((c565 >> 11) & 0b00011111) << 3;
				g = ((c565 >> 5) & 0b00111110) << 2;
				b = ((c565) & 0b00011111) << 3;

				r -= rm;
				g -= gm;
				b -= bm;

				if (r < 20)
					r = 0;
				if (g < 20)
					g = 0;
				if (b < 20)
					b = 0;
				buffer[y * G_H10_SCREEN_WIDTH + x] = g_H10_Tft.color565(r, g, b);
				buffer[((y - 1) * G_H10_SCREEN_WIDTH) + x + 1] = g_H10_Tft.color565(r, g, b);
			}
		}
		for (int y = G_H10_SCREEN_HEIGHT - 1; y > G_H10_SCREEN_HEIGHT / 2 - 1; y--)
		{
			for (int x = G_H10_SCREEN_WIDTH - 2; x > G_H10_SCREEN_WIDTH / 2 - 1; x--)
			{
				c565 = buffer[y * G_H10_SCREEN_WIDTH + x];
				r = ((c565 >> 11) & 0b00011111) << 3;
				g = ((c565 >> 5) & 0b00111110) << 2;
				b = ((c565) & 0b00011111) << 3;

				r -= rm;
				g -= gm;
				b -= bm;

				if (r < 20)
					r = 0;
				if (g < 20)
					g = 0;
				if (b < 20)
					b = 0;
				buffer[y * G_H10_SCREEN_WIDTH + x] = g_H10_Tft.color565(r, g, b);
				buffer[((y + 1) * G_H10_SCREEN_WIDTH) + x + 1] = g_H10_Tft.color565(r, g, b);
			}
		}
		break;
	case 0:
	default:
		g_H10_Canvas.fillScreen(0x0000);
		break;
	}
}

template <class X, class M, class N, class O, class Q>
X map_Generic(X x, M in_min, N in_max, O out_min, Q out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint16_t H10_getValueColor(float value, float min, float max)
{
	if (value > max)
	{
		value = max;
	}
	if (value < min)
	{
		value = min;
	}
	float valueScaled = map_Generic(value, min, max, 0.0, 1.0);
	float scale = 0;
	int16_t r = 0, g = 0, b = 0;
	switch (g_H10_clearMode)
	{
	case 1:
	{
		r = 255;
		g = (1 - valueScaled) * 255;
		break;
	}
	case 2:
	{
		r = (float)sin8(g_H10_frame / 3.0 + 85.0) * valueScaled;
		g = (float)sin8(g_H10_frame / 3.0) * valueScaled;
		b = (float)sin8(g_H10_frame / 3.0 + 170.0) * valueScaled;
		break;
	}
	case 3:
	{
		r = sin8(255.0 * valueScaled + 85.0);
		g = sin8(255.0 * valueScaled);
		b = sin8(255.0 * valueScaled + 170.0);
		break;
	}
	case 4:
	{
		if (valueScaled < .5)
		{
			// blue
			scale = (valueScaled) / .5;
			b = scale * 255.0;
			g = (valueScaled * scale) * 255.0;
		}
		else if (valueScaled < .75)
		{
			// green
			scale = (valueScaled - .5) / .25;
			b = (valueScaled * (1 - scale)) * 255.0;
			g = (valueScaled * scale) * 255.0;
			r = (valueScaled * scale) * 255.0;
		}
		else if (valueScaled <= 1)
		{
			// red
			scale = (valueScaled - .75) / .125;
			g = (valueScaled * (1 - scale)) * 255.0;
			if (g < 0)
			{
				g = 0;
			}
			r = 255; //(valueScaled * scale) * 255.0;
		}
		break;
	}
	case 0:
	default:
		r = g = b = valueScaled * 255;
	}
	return g_H10_Tft.color565(r, g, b);
}