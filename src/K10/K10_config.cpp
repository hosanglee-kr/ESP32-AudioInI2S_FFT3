


#include "K10_config.h"

// i2s config for using the internal ADC
i2s_config_t g_K10_i2s_adc_config = {
	.mode				  = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
	.sample_rate		  = G_K10_SAMPLE_RATE,
	.bits_per_sample	  = I2S_BITS_PER_SAMPLE_16BIT,
	.channel_format		  = I2S_CHANNEL_FMT_ONLY_LEFT,
	.communication_format = I2S_COMM_FORMAT_STAND_MSB,      //I2S_COMM_FORMAT_I2S_LSB,
	.intr_alloc_flags	  = ESP_INTR_FLAG_LEVEL1,
	.dma_buf_count		  = G_K10_BUFFER_COUNT,
	.dma_buf_len		  = G_K10_BUFFER_LENTH,
	.use_apll			  = false,
	.tx_desc_auto_clear	  = false,
	.fixed_mclk			  = 0
	};

// i2s config for reading from I2S
i2s_config_t g_K10_i2s_mic_config = {
	.mode				  = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
	.sample_rate		  = G_K10_SAMPLE_RATE,
	.bits_per_sample	  = I2S_BITS_PER_SAMPLE_32BIT,
	.channel_format		  = I2S_CHANNEL_FMT_ONLY_LEFT,
	.communication_format = I2S_COMM_FORMAT_STAND_I2S,      //I2S_COMM_FORMAT_I2S,
	.intr_alloc_flags	  = ESP_INTR_FLAG_LEVEL1,
	.dma_buf_count		  = G_K10_BUFFER_COUNT,
	.dma_buf_len		  = G_K10_BUFFER_LENTH,
	.use_apll			  = false,
	.tx_desc_auto_clear	  = false,
	.fixed_mclk			  = 0
	};

// i2s microphone pins
i2s_pin_config_t g_K10_i2s_mic_pins = {
	.bck_io_num	  = G_K10_I2S_MIC_SERIAL_CLOCK,
	.ws_io_num	  = G_K10_I2S_MIC_LEFT_RIGHT_CLOCK,
	.data_out_num = I2S_PIN_NO_CHANGE,
	.data_in_num  = G_K10_I2S_MIC_SERIAL_DATA
	};
