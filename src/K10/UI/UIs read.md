

위 코드에서, 클래스 `UI`는 다양한 그래픽 요소들을 관리하고 표시하는 역할을 합니다. 이 클래스는 주로 오디오 시각화를 위해 사용되며, `Waveform`, `GraphicEqualiser`, `Spectrogram` 등의 클래스들과 상호작용합니다. 주요 기능은 다음과 같습니다:

### 주요 클래스 및 메서드 설명
1. **UI 클래스** (`UI.h` 및 `UI.cpp`):
   - **멤버 변수:**
     - `m_palette`: 색상 팔레트.
     - `m_waveform`: 파형을 그리는 객체.
     - `m_graphic_equaliser`: 그래픽 이퀄라이저 객체.
     - `m_spectrogram`: 스펙트로그램 객체.
     - `m_display`: 디스플레이 객체 (`TFT_eSPI`).
     - `m_draw_task_handle`: FreeRTOS에서 사용하는 태스크 핸들.
   - **생성자 `UI(TFT_eSPI &display, int window_size)`**: 디스플레이와 창 크기를 받아서 UI 요소들을 초기화하고, FreeRTOS 태스크를 생성합니다.
   - **`toggle_display()`**: 화면에 표시되는 그래픽 요소를 전환합니다.
   - **`update(float *samples, float *fft)`**: 샘플 및 FFT 데이터를 업데이트하고, 그리기 태스크를 알림으로써 화면을 갱신합니다.
   - **`draw()`**: 화면에 그래픽 요소들을 그립니다.

2. **Waveform 클래스** (`Waveform.h` 및 `Waveform.cpp`):
   - **멤버 변수:**
     - `m_samples`: 오디오 샘플 데이터.
     - `m_num_samples`: 샘플 수.
   - **생성자 `Waveform(TFT_eSPI &display, int x, int y, int width, int height, int num_samples)`**: 디스플레이, 위치, 크기, 샘플 수를 받아서 초기화합니다.
   - **`update(const float *samples)`**: 새 샘플 데이터를 받아 업데이트합니다.
   - **`_draw(TFT_eSPI &display)`**: 디스플레이에 파형을 그립니다.

3. **Spectrogram 클래스** (`Spectrogram.h` 및 `Spectrogram.cpp`):
   - **멤버 변수:**
     - `m_palette`: 팔레트 객체.
     - `bitmap`: 비트맵 객체, 스펙트로그램 이미지를 저장.
   - **생성자 `Spectrogram(Palette *palette, int x, int y, int width, int height)`**: 팔레트와 위치, 크기를 받아 초기화합니다.
   - **`update(float *magnitudes)`**: FFT 크기 데이터를 업데이트하여 스펙트로그램을 갱신합니다.
   - **`_draw(TFT_eSPI &display)`**: 디스플레이에 스펙트로그램을 그립니다.

4. **GraphicEqualiser 클래스** (`GraphicEqualiser.h` 및 `GraphicEqualiser.cpp`):
   - **멤버 변수:**
     - `m_palette`: 팔레트 객체.
     - `m_num_bins`: 이퀄라이저 막대의 개수.
     - `bar_chart`, `bar_chart_peaks`: 막대 그래프와 그 피크 값들을 저장하는 배열.
   - **생성자 `GraphicEqualiser(Palette *palette, int x, int y, int width, int height, int num_bins)`**: 팔레트와 위치, 크기, 이퀄라이저 막대 개수를 받아 초기화합니다.
   - **`update(float *mag)`**: FFT 데이터를 받아 막대 그래프와 피크 값을 업데이트합니다.
   - **`_draw(TFT_eSPI &display)`**: 디스플레이에 그래픽 이퀄라이저를 그립니다.

5. **Palette 클래스** (`Palette.h` 및 `Palette.cpp`):
   - **멤버 변수:**
     - `colors`: 색상 배열.
   - **`get_color(int index)`**: 주어진 인덱스에 해당하는 색상을 반환합니다.

6. **Bitmap 클래스** (`Bitmap.h` 및 `Bitmap.cpp`):
   - **멤버 변수:**
     - `pixels`: 비트맵의 픽셀 데이터.
     - `rows`: 비트맵의 각 행을 가리키는 포인터 배열.
     - `width`, `height`: 비트맵의 너비와 높이.
   - **`scroll_left()`**: 비트맵을 왼쪽으로 스크롤합니다.

### 동작 흐름
- `UI` 객체는 생성 시 디스플레이 객체를 받아 다양한 그래픽 컴포넌트를 초기화합니다.
- `toggle_display()` 메서드로 그래픽 요소를 전환할 수 있습니다.
- `update()` 메서드가 호출되면, 오디오 데이터와 FFT 데이터가 각 그래픽 요소에 전달되고, 화면을 갱신하도록 그리기 태스크를 트리거합니다.
- `draw()` 메서드는 모든 그래픽 요소를 디스플레이에 렌더링합니다.

이 코드는 주로 임베디드 시스템에서 실시간 오디오 시각화를 위해 설계되었습니다. FreeRTOS 태스크와 연동되어 비동기적으로 화면을 갱신하는 방식이 특징입니다.