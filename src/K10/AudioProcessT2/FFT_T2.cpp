
#include "FFT_T2.h"
#include <math.h>
#include <stdlib.h>

FFT_T2::FFT_T2(int window_size) : FFT(), m_fft_size(1), m_window_size(window_size) {
    while (m_fft_size < window_size) {
        m_fft_size *= 2;
    }

    m_fft_input = static_cast<float *>(malloc(sizeof(float) * m_fft_size));
    vImag = static_cast<float *>(malloc(sizeof(float) * m_fft_size));
    m_fft_output = static_cast<float *>(malloc(sizeof(float) * m_fft_size / 2)); // FFT 출력 버퍼 할당

    for (int i = 0; i < m_fft_size; i++) {
        m_fft_input[i] = 0;
        vImag[i] = 0; // 허수 부분을 0으로 초기화
    }

    m_energy = static_cast<float *>(malloc(sizeof(float) * (m_window_size / 4)));
}

void FFT_T2::update(int16_t *samples) {
    int offset = (m_fft_size - m_window_size) / 2;
    for (int i = 0; i < m_window_size; i++) {
        m_fft_input[offset + i] = samples[i] / 30.0f;
        vImag[offset + i] = 0; // 허수 부분은 여전히 0
    }

    // arduinoFFT의 windowing 함수로 Hamming 윈도우 적용
    FFT.windowing(m_fft_input, m_fft_size, FFTWindow::Hamming, FFTDirection::Forward);	
    //compute(T *vReal, T *vImag, uint_fast16_t samples, FFTDirection dir) const;
    //FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);	    // Weigh data 
    //FFT.windowing(m_fft_input, m_fft_size, FFT_WIN_TYP_HAMMING);

    // FFT 계산 수행
    FFT.compute(m_fft_input, vImag, m_fft_size, FFTDirection::Forward);
    // void compute(T *vReal, T *vImag, uint_fast16_t samples, FFTDirection dir) const;
    // 복소수 결과를 크기로 변환
    FFT.complexToMagnitude(m_fft_input, vImag, m_fft_size);

    // m_fft_output에 FFT 결과 저장
    for (int i = 0; i < m_fft_size / 2; i++) {
        m_fft_output[i] = m_fft_input[i];
    }

    // 에너지 계산
    for (int i = 0; i < m_window_size / 4; i++) {
        m_energy[i] = m_fft_output[i];
    }
}

