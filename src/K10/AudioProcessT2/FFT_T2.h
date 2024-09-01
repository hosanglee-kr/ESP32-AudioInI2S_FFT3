
#pragma once

#include <stdint.h>
#include <arduinoFFT.h> // arduinoFFT 라이브러리 추가

class FFT_T2 {
private:
    int m_fft_size;
    int m_window_size;
    arduinoFFT FFT;  // arduinoFFT 객체 추가
    float *vReal;    // FFT 입력 데이터의 실수 부분
    float *vImag;    // FFT 입력 데이터의 허수 부분

public:
    float *m_energy;

    FFT_T2(int window_size);
    void update(int16_t *samples);
};
