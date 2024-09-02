#pragma once

#include <driver/i2s.h>
#include <freertos/FreeRTOS.h>

// display toggle button
#define     G_K10_GPIO_BUTTON                 0

// sample rate for the system
#define     G_K10_SAMPLE_RATE                 16000

#define     G_K10_BUFFER_LENTH                1024
#define     G_K10_BUFFER_COUNT                4



// approx 30ms of audio @ 16KHz
#define     G_K10_WINDOW_SIZE                 512


// are you using an I2S microphone - comment this out if you want to use an analog mic and ADC input
#define     G_K10_USE_I2S_MIC_INPUT

// I2S Microphone Settings
// Which channel is the I2S microphone on? I2S_CHANNEL_FMT_ONLY_LEFT or I2S_CHANNEL_FMT_ONLY_RIGHT
// Generally they will default to LEFT - but you may need to attach the L/R pin to GND
#define     G_K10_I2S_MIC_CHANNEL			    I2S_CHANNEL_FMT_ONLY_LEFT
// #define  G_K10_I2S_MIC_CHANNEL               I2S_CHANNEL_FMT_ONLY_RIGHT

#define     G_K10_I2S_MIC_SERIAL_CLOCK	        GPIO_NUM_26
#define     G_K10_I2S_MIC_LEFT_RIGHT_CLOCK      GPIO_NUM_22
#define     G_K10_I2S_MIC_SERIAL_DATA		    GPIO_NUM_21

// Analog Microphone Settings - ADC1_CHANNEL_7 is GPIO35
#define     G_K10_ADC_MIC_CHANNEL			    ADC1_CHANNEL_7



#define     G_K10_AUDIOPROCESS_T1
//#define   G_K10_AUDIOPROCESS_T2


// i2s config for using the internal ADC
extern  i2s_config_t    g_K10_i2s_adc_config;
// i2s config for reading from of I2S
extern  i2s_config_t    g_K10_i2s_mic_config;
// i2s microphone pins
extern  i2s_pin_config_t g_K10_i2s_mic_pins;

