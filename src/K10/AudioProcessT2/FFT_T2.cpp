
#include "FFT_T2.h"
#include <math.h>
#include <stdlib.h>

FFT_T2::FFT_T2(int window_size) : FFT(), m_fft_size(1), m_window_size(window_size) {
    while (m_fft_size < window_size) {
        m_fft_size *= 2;
    }

    vReal = static_cast<float *>(malloc(sizeof(float) * m_fft_size));
    vImag = static_cast<float *>(malloc(sizeof(float) * m_fft_size));
    for (int i = 0; i < m_fft_size; i++) {
        vReal[i] = 0;
        vImag[i] = 0; // 허수 부분을 0으로 초기화
    }

    m_energy = static_cast<float *>(malloc(sizeof(float) * (m_window_size / 4)));
}

void FFT_T2::update(int16_t *samples) {
    int offset = (m_fft_size - m_window_size) / 2;
    for (int i = 0; i < m_window_size; i++) {
        vReal[offset + i] = samples[i] / 30.0f;
        vImag[offset + i] = 0; // 허수 부분은 여전히 0
    }

    // arduinoFFT의 Windowing 함수로 Hamming 윈도우 적용
    FFT.windowing(vReal, m_fft_size, FFT_WIN_TYP_HAMMING, FFT_FORWARD);

    // FFT 계산 수행
    FFT.compute(vReal, vImag, m_fft_size, FFT_FORWARD);

    // 복소수 결과를 크기로 변환
    FFT.complexToMagnitude(vReal, vImag, m_fft_size);

    // 에너지 계산
    for (int i = 0; i < m_window_size / 4; i++) {
        m_energy[i] = vReal[i];
    }
}

