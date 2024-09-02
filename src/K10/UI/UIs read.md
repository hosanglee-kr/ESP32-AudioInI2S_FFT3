

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

---

ESP32에서 LittleFS를 사용하여 HTML 파일을 서빙하려면, 먼저 LittleFS 파일 시스템을 초기화하고, HTML 파일을 LittleFS에 업로드한 후, 웹 서버에서 이 파일을 제공해야 합니다.

### 1. LittleFS 파일 시스템 초기화

ESP32에서 LittleFS 파일 시스템을 초기화하고 HTML 파일을 제공하는 예제 코드를 아래에 작성하겠습니다. 이 코드는 HTML 파일을 LittleFS에 저장하고 이를 웹 서버에서 서빙합니다.

#### a. LittleFS 파일 시스템 설정

1. **LittleFS 라이브러리 설치**:
   - Arduino IDE에서 LittleFS 라이브러리를 설치합니다. `Tools` -> `LittleFS Data Upload`을 통해 LittleFS 데이터를 업로드할 수 있습니다.

2. **HTML 파일 업로드**:
   - HTML 파일을 `data` 폴더에 저장합니다. 예를 들어, `data/index.html` 파일을 만들어 해당 HTML 코드를 저장합니다.

#### b. ESP32 코드

```cpp
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

// WiFi 정보
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// 웹 서버 및 WebSocket 설정
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void setup() {
  Serial.begin(115200);

  // LittleFS 초기화
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
    return;
  }

  // WiFi 연결
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // 웹 페이지 제공
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", String(), false);
  });

  // WebSocket 이벤트 핸들러
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  server.begin();
}

void loop() {
  float samples[256];
  float fft[256];
  
  // 예시: 데이터를 생성 (실제 데이터로 교체)
  for (int i = 0; i < 256; i++) {
    samples[i] = random(0, 1024) / 1024.0;
    fft[i] = random(0, 1024) / 1024.0;
  }
  
  // UI 업데이트 (여기에 맞는 코드 추가)
  // ui->update(samples, fft);

  // samples 및 fft 데이터 WebSocket으로 전송
  String samplesData = prepareSamplesData(samples);
  String fftData = prepareFftData(fft);
  
  ws.textAll("samples:" + samplesData); // "samples:" 접두사로 samples 데이터 전송
  ws.textAll("fft:" + fftData); // "fft:" 접두사로 fft 데이터 전송

  delay(100);  // 적절한 딜레이 추가
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                      void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket client disconnected");
  }
}

String prepareSamplesData(float *samples) {
  String data = "[";
  for (int i = 0; i < 256; i++) {
    data += String(samples[i]);
    if (i < 255) data += ",";
  }
  data += "]";
  return data;
}

String prepareFftData(float *fft) {
  String data = "[";
  for (int i = 0; i < 256; i++) {
    data += String(fft[i]);
    if (i < 255) data += ",";
  }
  data += "]";
  return data;
}
```

### 2. HTML 파일 예제

`data/index.html` 파일의 내용은 다음과 같습니다. 이 파일은 체크박스와 차트를 포함하고 있습니다.

```html
<!DOCTYPE html>
<html>
<head>
  <title>Audio Visualization</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    .chart-container {
      display: flex;
      flex-direction: column;
      align-items: center;
      margin-top: 20px;
    }
    .chart-container canvas {
      margin: 10px;
    }
  </style>
</head>
<body>
  <h1>Audio Visualization</h1>

  <!-- 체크박스들 -->
  <div>
    <label>
      <input type="checkbox" id="waveformToggle" checked> Waveform
    </label>
    <label>
      <input type="checkbox" id="spectrogramToggle" checked> Spectrogram
    </label>
    <label>
      <input type="checkbox" id="fftToggle" checked> Graphic Equaliser
    </label>
  </div>

  <!-- 차트 컨테이너들 -->
  <div class="chart-container">
    <canvas id="waveformChart"></canvas>
    <canvas id="spectrogramChart"></canvas>
    <canvas id="fftChart"></canvas>
  </div>

  <script>
    const waveformCtx = document.getElementById('waveformChart').getContext('2d');
    const spectrogramCtx = document.getElementById('spectrogramChart').getContext('2d');
    const fftCtx = document.getElementById('fftChart').getContext('2d');

    const waveformChart = new Chart(waveformCtx, {
      type: 'line',
      data: {
        labels: Array.from({ length: 256 }, (_, i) => i),
        datasets: [{ label: 'Waveform', data: [], borderColor: 'rgb(75, 192, 192)', tension: 0.1 }]
      },
      options: { responsive: true, animation: false }
    });

    const spectrogramChart = new Chart(spectrogramCtx, {
      type: 'line',
      data: {
        labels: Array.from({ length: 256 }, (_, i) => i),
        datasets: [{ label: 'Spectrogram', data: [], borderColor: 'rgb(255, 205, 86)', tension: 0.1 }]
      },
      options: { responsive: true, animation: false }
    });

    const fftChart = new Chart(fftCtx, {
      type: 'line',
      data: {
        labels: Array.from({ length: 256 }, (_, i) => i),
        datasets: [{ label: 'FFT', data: [], borderColor: 'rgb(255, 99, 132)', tension: 0.1 }]
      },
      options: { responsive: true, animation: false }
    });

    const ws = new WebSocket(`ws://${window.location.host}/ws`);

    ws.onmessage = (event) => {
      const message = event.data;
      
      if (message.startsWith("samples:")) {
        const samplesData = JSON.parse(message.substring("samples:".length));
        waveformChart.data.datasets[0].data = samplesData;
        if (document.getElementById('waveformToggle').checked) {
          waveformChart.update();
        }
      } else if (message.startsWith("fft:")) {
        const fftData = JSON.parse(message.substring("fft:".length));
        fftChart.data.datasets[0].data = fftData;
        spectrogramChart.data.datasets[0].data = fftData; // 스펙트로그램에 FFT 데이터 사용
        if (document.getElementById('fftToggle').checked) {
          fftChart.update();
        }
        if (document.getElementById('spectrogramToggle').checked) {
          spectrogramChart.update();
        }
      }
    };

    // 체크박스 상태에 따라 차트 표시/숨김 처리
    document.getElementById('waveformToggle').addEventListener('change', (event) => {
      if (event.target.checked) {
        waveformChart.canvas.style.display = 'block';
      } else {
        waveformChart.canvas.style.display = 'none';
      }
    });

    document.getElementById('spectrogramToggle').addEventListener('change', (event) => {
      if (event.target.checked) {
        spectrogramChart.canvas.style.display = 'block';
      } else {
        spectrogramChart.canvas.style.display = 'none';
      }
    });

    document.getElementById('fftToggle').addEventListener('change', (event) => {
      if (event.target.checked) {
        fftChart.canvas.style.display = 'block';
      } else {
        fftChart.canvas.style.display = 'none';
      }
    });
  </script>
</body>
</html>
```

### 3. 코드 설명

- **LittleFS 초기화**: `LittleFS.begin()`을 호출하여 LittleFS 파일 시스템을 초기화합니다.
- **HTML 파일 서빙**: `server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { ... });`을 사용하여 루트 URL에 대한 요청을 처리하고 HTML 파일을 제공하는 핸들러를 설정합니다.
- **WebSocket 데이터 전송**: WebSocket을 통해 `samples`와 `fft` 데이터를 전송하고, 클라이언트에서 이 데이터를 받아 차트를 업데이트합니다.
- **체크박스 처리**: 체크박스
