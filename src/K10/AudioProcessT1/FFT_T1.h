#pragma once


#include <inttypes.h>

class HammingWindow {
   private:
	float *m_coefficients;
	int	   m_window_size;

   public:
	HammingWindow(int window_size);
	~HammingWindow();
	void applyWindow(float *input);
};

#include <stdint.h>

#include "tools/kiss_fftr.h"
// #include "tools/kiss_fftr.h"


class FFT_T1{
	private:
		HammingWindow *m_hamming_window;
		int m_fft_size;
		int m_window_size;
		kiss_fftr_cfg m_cfg;
		kiss_fft_cpx *m_fft_output;

	public:
		float *m_energy;
		float *m_fft_input;

		FFT_T1(int window_size);
		void update(int16_t *samples);
};