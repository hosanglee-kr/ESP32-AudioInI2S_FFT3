#include "I2SMEMSSampler.h"

#include "soc/i2s_reg.h"

I2SMEMSSampler::I2SMEMSSampler(
	i2s_port_t		  i2s_port,
	i2s_pin_config_t &i2s_pins,
	i2s_config_t	  i2s_config,
	bool			  fixSPH0645)
	: I2SSampler(i2s_port, i2s_config) {
	m_i2sPins	 = i2s_pins;
	m_fixSPH0645 = fixSPH0645;
}

void I2SMEMSSampler::configureI2S() {
	if (m_fixSPH0645) {
		// FIXES for SPH0645
		REG_SET_BIT(I2S_TIMING_REG(m_i2sPort), BIT(9));
		REG_SET_BIT(I2S_CONF_REG(m_i2sPort), I2S_RX_MSB_SHIFT);
	}

	i2s_set_pin(m_i2sPort, &m_i2sPins);
}

int I2SMEMSSampler::read(int16_t *samples, int count) {

	#if G_K10_I2S_MIC_BITS_PER_SAMPLE == I2S_BITS_PER_SAMPLE_16BIT
		// 원시 샘플을 위한 메모리 할당
		int16_t *raw_samples = (int16_t *)malloc(sizeof(int16_t) * count);
		if (raw_samples == NULL) {
			// 메모리 할당 실패 처리
			return -1;
		}

		size_t bytes_read = 0;
		// I2S에서 읽기
		if (i2s_read(m_i2sPort, raw_samples, sizeof(int16_t) * count, &bytes_read, portMAX_DELAY) != ESP_OK) {
			// 읽기 오류 처리
			free(raw_samples);
			return -1;
		}

		// 읽은 바이트 수를 샘플 수로 변환
		int samples_read = bytes_read / sizeof(int16_t);
		// 처리할 수 있는 샘플 수를 초과하지 않도록 보장
		samples_read = (samples_read > count) ? count : samples_read;

		// 원시 샘플을 직접 복사
		for (int i = 0; i < samples_read; i++) {
			samples[i] = raw_samples[i];
		}

	#elif G_K10_I2S_MIC_BITS_PER_SAMPLE == I2S_BITS_PER_SAMPLE_32BIT
		// 원시 샘플을 위한 메모리 할당
		int32_t *raw_samples = (int32_t *)malloc(sizeof(int32_t) * count);
		if (raw_samples == NULL) {
			// 메모리 할당 실패 처리
			return -1;
		}

		size_t bytes_read = 0;
		// I2S에서 읽기
		if (i2s_read(m_i2sPort, raw_samples, sizeof(int32_t) * count, &bytes_read, portMAX_DELAY) != ESP_OK) {
			// 읽기 오류 처리
			free(raw_samples);
			return -1;
		}

		int samples_read = bytes_read / sizeof(int32_t);
		// 처리할 수 있는 샘플 수를 초과하지 않도록 보장
		samples_read = (samples_read > count) ? count : samples_read;

		// 원시 샘플 처리
		for (int i = 0; i < samples_read; i++) {
			samples[i] = (raw_samples[i] & 0xFFFFFFF0) >> 14;
		}
	#endif



	free(raw_samples);
	return samples_read;
}
