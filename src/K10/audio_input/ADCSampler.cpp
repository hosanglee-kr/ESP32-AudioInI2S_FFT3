#include "ADCSampler.h"

ADCSampler::ADCSampler(adc_unit_t adcUnit, adc1_channel_t adcChannel, const i2s_config_t &i2s_config)
	: I2SSampler(I2S_NUM_0, i2s_config) {
	m_adcUnit	 = adcUnit;
	m_adcChannel = adcChannel;
}

void ADCSampler::configureI2S() {
	// init ADC pad
	i2s_set_adc_mode(m_adcUnit, m_adcChannel);
	// enable the adc
	i2s_adc_enable(m_i2sPort);
}

int ADCSampler::read(int16_t *samples, int count) {
	

	#if G_K10_I2S_MIC_BITS_PER_SAMPLE == I2S_BITS_PER_SAMPLE_16BIT
		// I2S에서 데이터 읽기
		size_t bytes_read = 0;
		if (i2s_read(m_i2sPort, samples, sizeof(int16_t) * count, &bytes_read, portMAX_DELAY) != ESP_OK) {
			// 읽기 오류 처리
			return -1;
		}

		// 읽은 바이트 수를 샘플 수로 변환
		int samples_read = bytes_read / sizeof(int16_t);
		// 처리할 수 있는 샘플 수를 초과하지 않도록 보장
		samples_read = (samples_read > count) ? count : samples_read;

		// // 샘플 변환 및 처리
		// for (int i = 0; i < samples_read; i++) {
		// 	// 예를 들어, 12비트 샘플을 읽어온 후 16비트로 변환 및 스케일링
		// 	samples[i] = (2048 - (samples[i] & 0xFFF)) * 15;
		// }


	#elif G_K10_I2S_MIC_BITS_PER_SAMPLE == I2S_BITS_PER_SAMPLE_32BIT
		/ int32_t 배열을 위한 메모리 할당
		int32_t *raw_samples = (int32_t *)malloc(sizeof(int32_t) * count);
		if (raw_samples == NULL) {
			// 메모리 할당 실패 처리
			return -1;
		}

		size_t bytes_read = 0;
		// I2S에서 int32_t 데이터 읽기
		if (i2s_read(m_i2sPort, raw_samples, sizeof(int32_t) * count, &bytes_read, portMAX_DELAY) != ESP_OK) {
			// 읽기 오류 처리
			free(raw_samples);
			return -1;
		}

		int samples_read = bytes_read / sizeof(int32_t);
		// 처리할 수 있는 샘플 수를 초과하지 않도록 보장
		samples_read = (samples_read > count) ? count : samples_read;

		/// 
		// int32_t 샘플을 int16_t로 변환 및 처리
		for (int i = 0; i < samples_read; i++) {
			// int32_t에서 16비트 데이터 추출
			int16_t raw_sample = (int16_t)(raw_samples[i] & 0xFFFF);
			// 변환 및 처리
			samples[i] = (2048 - (uint16_t(raw_sample) & 0xFFF)) * 15;
		}
		// 메모리 해제
		free(raw_samples);
	#endif 

	return samples_read;


	// int samples_read = bytes_read / sizeof(int32_t);
	// // 처리할 수 있는 샘플 수를 초과하지 않도록 보장
	// samples_read = (samples_read > count) ? count : samples_read;

	// // int32_t 샘플을 int16_t로 변환 및 처리
	// for (int i = 0; i < samples_read; i++) {
	// 	// int32_t에서 16비트 데이터 추출
	// 	int16_t raw_sample = (int16_t)(raw_samples[i] & 0xFFFF);
	// 	// 변환 및 처리
	// 	samples[i] = (2048 - (uint16_t(raw_sample) & 0xFFF)) * 15;
	// }
}
