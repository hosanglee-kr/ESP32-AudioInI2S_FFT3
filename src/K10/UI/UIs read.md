

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


---


`UI` 클래스에 FreeRTOS를 적용하여, 데이터를 비동기적으로 처리하고 WebSocket을 통해 실시간으로 전송하는 방법을 살펴보겠습니다. FreeRTOS는 ESP32에서 멀티태스킹을 지원하는 실시간 운영체제입니다. 이를 통해 ESP32는 여러 개의 작업(Task)을 동시에 수행할 수 있습니다. 

### FreeRTOS 적용 예제

여기에서는 FreeRTOS를 사용하여 데이터를 처리하는 작업을 별도의 태스크(Task)로 분리하고, WebSocket을 통해 주기적으로 데이터를 전송하는 예제를 구현해보겠습니다.

#### 1. `UI` 클래스 정의

```cpp
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class UI {
private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    TaskHandle_t taskHandle;

public:
    UI(int port) : server(port), ws("/ws"), taskHandle(NULL) {}

    void begin() {
        // LittleFS 파일 시스템 초기화
        if (!LittleFS.begin()) {
            Serial.println("Failed to mount file system");
            return;
        }

        // 웹 서버에서 HTML 페이지 서빙
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(LittleFS, "/index.html", String(), false);
        });

        // WebSocket 핸들러 설정
        ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, 
                          AwsEventType type, void *arg, uint8_t *data, size_t len) {
            onWebSocketEvent(server, client, type, arg, data, len);
        });

        server.addHandler(&ws);
        server.begin();

        // 데이터 처리 태스크 생성
        xTaskCreatePinnedToCore(taskFunc, "DataUpdateTask", 4096, this, 1, &taskHandle, 1);
    }

    void update(float *samples, float *fft) {
        // 데이터를 WebSocket으로 전송하는 코드
        String samplesData = prepareData(samples, 256);
        String fftData = prepareData(fft, 256);
        
        ws.textAll("samples:" + samplesData);  // WebSocket을 통해 samples 데이터 전송
        ws.textAll("fft:" + fftData);          // WebSocket을 통해 fft 데이터 전송
    }

private:
    static void taskFunc(void *pvParameters) {
        UI *ui = static_cast<UI *>(pvParameters);
        float samples[256];
        float fft[256];

        while (true) {
            // 여기서 실제 데이터 처리를 하고 update() 메서드를 호출
            for (int i = 0; i < 256; i++) {
                samples[i] = random(0, 1024) / 1024.0;
                fft[i] = random(0, 1024) / 1024.0;
            }

            ui->update(samples, fft);
            vTaskDelay(pdMS_TO_TICKS(100));  // 100ms 대기
        }
    }

    // WebSocket 이벤트 처리 메서드
    void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                          AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.println("WebSocket client connected");
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.println("WebSocket client disconnected");
        }
    }

    // 데이터를 JSON 형식의 문자열로 변환하는 메서드
    String prepareData(float *data, size_t len) {
        String json = "[";
        for (size_t i = 0; i < len; i++) {
            json += String(data[i]);
            if (i < len - 1) {
                json += ",";
            }
        }
        json += "]";
        return json;
    }
};
```

#### 2. 주요 변경 사항 설명

1. **FreeRTOS Task 추가**: 
    - `taskFunc`라는 이름의 새로운 FreeRTOS 태스크를 생성했습니다. 이 태스크는 `begin()` 메서드가 호출될 때 생성됩니다. `taskFunc`는 주기적으로 데이터를 처리하고 `update()` 메서드를 호출하여 WebSocket을 통해 데이터를 전송합니다.

2. **xTaskCreatePinnedToCore()**:
    - `xTaskCreatePinnedToCore()` 함수를 사용하여 태스크를 특정 코어에 고정시켜 생성합니다. 이 함수는 태스크를 생성하고 실행할 코어를 지정할 수 있도록 합니다. ESP32는 듀얼 코어이기 때문에 이 기능을 활용하면 성능을 최적화할 수 있습니다. 여기서는 코어 1에 태스크를 할당했습니다.

3. **vTaskDelay()**:
    - `vTaskDelay(pdMS_TO_TICKS(100))`는 FreeRTOS에서 태스크를 일정 시간 동안 대기 상태로 만드는 함수입니다. 여기서는 100ms 동안 대기한 후 다시 태스크가 실행됩니다.

#### 3. 사용 예시

```cpp
UI ui(80);

void setup() {
    Serial.begin(115200);
    ui.begin();
}

void loop() {
    // 메인 루프에서는 아무 작업도 수행하지 않습니다.
    // 모든 작업은 FreeRTOS 태스크로 수행됩니다.
}
```

### 요약

- `UI` 클래스는 웹 기반 데이터 시각화 애플리케이션을 ESP32에서 쉽게 구현할 수 있도록 설계되었습니다.
- FreeRTOS를 사용하여 데이터를 처리하는 태스크를 별도로 생성함으로써 ESP32의 멀티태스킹 기능을 활용할 수 있습니다.
- 이 구조는 데이터를 비동기적으로 처리하고, WebSocket을 통해 실시간으로 전송하는 데 적합합니다.

이를 통해 ESP32가 복잡한 작업을 동시에 수행할 수 있게 되며, 데이터 처리 및 전송 작업을 효율적으로 관리할 수 있습니다.

---


`UI` 클래스명을 `WebUI`로 변경하려면 클래스 선언과 정의, 그리고 클래스 이름이 사용된 모든 곳에서 `UI`를 `WebUI`로 수정해야 합니다. 다음은 이를 반영한 코드입니다.

### 1. `WebUI.h`

```cpp
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class WebUI {
private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    TaskHandle_t taskHandle;

public:
    WebUI(int port) : server(port), ws("/ws"), taskHandle(NULL) {}

    void begin() {
        // LittleFS 파일 시스템 초기화
        if (!LittleFS.begin()) {
            Serial.println("Failed to mount file system");
            return;
        }

        // 웹 서버에서 HTML 페이지 서빙
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(LittleFS, "/index.html", String(), false);
        });

        // WebSocket 핸들러 설정
        ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, 
                          AwsEventType type, void *arg, uint8_t *data, size_t len) {
            onWebSocketEvent(server, client, type, arg, data, len);
        });

        server.addHandler(&ws);
        server.begin();

        // 데이터 처리 태스크 생성
        xTaskCreatePinnedToCore(taskFunc, "DataUpdateTask", 4096, this, 1, &taskHandle, 1);
    }

    void update(float *samples, float *fft) {
        // 데이터를 WebSocket으로 전송하는 코드
        String samplesData = prepareData(samples, 256);
        String fftData = prepareData(fft, 256);
        
        ws.textAll("samples:" + samplesData);  // WebSocket을 통해 samples 데이터 전송
        ws.textAll("fft:" + fftData);          // WebSocket을 통해 fft 데이터 전송
    }

private:
    static void taskFunc(void *pvParameters) {
        WebUI *webUI = static_cast<WebUI *>(pvParameters);
        float samples[256];
        float fft[256];

        while (true) {
            // 여기서 실제 데이터 처리를 하고 update() 메서드를 호출
            for (int i = 0; i < 256; i++) {
                samples[i] = random(0, 1024) / 1024.0;
                fft[i] = random(0, 1024) / 1024.0;
            }

            webUI->update(samples, fft);
            vTaskDelay(pdMS_TO_TICKS(100));  // 100ms 대기
        }
    }

    // WebSocket 이벤트 처리 메서드
    void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                          AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.println("WebSocket client connected");
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.println("WebSocket client disconnected");
        }
    }

    // 데이터를 JSON 형식의 문자열로 변환하는 메서드
    String prepareData(float *data, size_t len) {
        String json = "[";
        for (size_t i = 0; i < len; i++) {
            json += String(data[i]);
            if (i < len - 1) {
                json += ",";
            }
        }
        json += "]";
        return json;
    }
};
```

### 2. 사용 예시 (`main.ino`)

```cpp
WebUI webUI(80);

void setup() {
    Serial.begin(115200);
    webUI.begin();
}

void loop() {
    // 메인 루프에서는 아무 작업도 수행하지 않습니다.
    // 모든 작업은 FreeRTOS 태스크로 수행됩니다.
}
```

### 변경 사항 설명

1. **클래스 이름 변경**:
   - `UI` 클래스명을 `WebUI`로 변경했습니다. 모든 멤버 함수, 생성자, 클래스 포인터 등에서 `UI`를 `WebUI`로 수정했습니다.

2. **`taskFunc` 함수 내에서의 변경**:
   - `taskFunc` 함수 내에서 `UI` 대신 `WebUI` 포인터를 사용하도록 수정했습니다.

3. **사용 예시**:
   - `WebUI` 클래스를 사용하는 부분에서도 이름을 변경했습니다.

이제 `WebUI` 클래스를 사용하여 웹 인터페이스에서 실시간 데이터를 처리하고 전송할 수 있습니다. 이 변경 사항은 클래스 이름만 바뀐 것이며, 기존의 기능은 그대로 유지됩니다.



--- 


각각의 데이터(`samples`와 `fft`)를 처리하기 위해 별도의 큐를 사용하는 방법을 구현할 수 있습니다. 이렇게 하면 `samples`와 `fft` 데이터를 각각 독립적으로 관리하고 처리할 수 있습니다. 아래는 두 개의 큐를 사용하도록 수정된 `WebUI` 클래스의 예시입니다.

### 1. `WebUI.h`

```cpp
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

class WebUI {
private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    TaskHandle_t taskHandle;
    QueueHandle_t samplesQueue;  // 샘플 데이터를 위한 큐
    QueueHandle_t fftQueue;      // FFT 데이터를 위한 큐

    static const int queueLength = 10;  // 큐 길이
    static const int dataLength = 256;  // 샘플 및 FFT 데이터의 길이

public:
    WebUI(int port) 
        : server(port), ws("/ws"), taskHandle(NULL) {
        // 큐 생성: 각 항목은 float 배열(샘플 또는 FFT 데이터)을 담을 수 있는 크기
        samplesQueue = xQueueCreate(queueLength, sizeof(float[dataLength]));
        fftQueue = xQueueCreate(queueLength, sizeof(float[dataLength]));
    }

    void begin() {
        // LittleFS 파일 시스템 초기화
        if (!LittleFS.begin()) {
            Serial.println("Failed to mount file system");
            return;
        }

        // 웹 서버에서 HTML 페이지 서빙
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(LittleFS, "/index.html", String(), false);
        });

        // WebSocket 핸들러 설정
        ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, 
                          AwsEventType type, void *arg, uint8_t *data, size_t len) {
            onWebSocketEvent(server, client, type, arg, data, len);
        });

        server.addHandler(&ws);
        server.begin();

        // 데이터 처리 태스크 생성
        xTaskCreatePinnedToCore(taskFunc, "DataUpdateTask", 4096, this, 1, &taskHandle, 1);
    }

    void updateSamples(float *samples) {
        // 큐에 샘플 데이터를 삽입
        if (xQueueSend(samplesQueue, samples, pdMS_TO_TICKS(10)) != pdPASS) {
            Serial.println("Samples queue is full. Data update failed.");
        }
    }

    void updateFFT(float *fft) {
        // 큐에 FFT 데이터를 삽입
        if (xQueueSend(fftQueue, fft, pdMS_TO_TICKS(10)) != pdPASS) {
            Serial.println("FFT queue is full. Data update failed.");
        }
    }

private:
    static void taskFunc(void *pvParameters) {
        WebUI *webUI = static_cast<WebUI *>(pvParameters);
        float samples[dataLength];
        float fft[dataLength];

        while (true) {
            // 큐에서 샘플 데이터 수신
            if (xQueueReceive(webUI->samplesQueue, &samples, portMAX_DELAY) == pdPASS) {
                webUI->sendSamplesToClient(samples);
            }

            // 큐에서 FFT 데이터 수신
            if (xQueueReceive(webUI->fftQueue, &fft, portMAX_DELAY) == pdPASS) {
                webUI->sendFFTToClient(fft);
            }
        }
    }

    void sendSamplesToClient(float *samples) {
        // 샘플 데이터를 WebSocket으로 전송하는 코드
        String samplesData = prepareData(samples, dataLength);
        ws.textAll("samples:" + samplesData);  // WebSocket을 통해 samples 데이터 전송
    }

    void sendFFTToClient(float *fft) {
        // FFT 데이터를 WebSocket으로 전송하는 코드
        String fftData = prepareData(fft, dataLength);
        ws.textAll("fft:" + fftData);  // WebSocket을 통해 fft 데이터 전송
    }

    // WebSocket 이벤트 처리 메서드
    void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                          AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.println("WebSocket client connected");
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.println("WebSocket client disconnected");
        }
    }

    // 데이터를 JSON 형식의 문자열로 변환하는 메서드
    String prepareData(float *data, size_t len) {
        String json = "[";
        for (size_t i = 0; i < len; i++) {
            json += String(data[i]);
            if (i < len - 1) {
                json += ",";
            }
        }
        json += "]";
        return json;
    }
};
```

### 2. `main.ino`

```cpp
WebUI webUI(80);

void setup() {
    Serial.begin(115200);
    webUI.begin();
}

void loop() {
    // 필요시 webUI.updateSamples(samplesArray); 또는 webUI.updateFFT(fftArray); 호출
}
```

### 주요 변경 사항 설명

1. **두 개의 큐 생성**:
   - `samplesQueue`는 샘플 데이터를 저장하는 큐입니다.
   - `fftQueue`는 FFT 데이터를 저장하는 큐입니다.
   - 이 두 큐는 각각 독립적으로 데이터를 처리하고 전송합니다.

2. **`updateSamples()`와 `updateFFT()` 메서드**:
   - 각각의 메서드는 데이터를 해당 큐에 삽입합니다. 만약 큐가 꽉 차면 실패 메시지를 출력합니다.

3. **`taskFunc()`**:
   - `taskFunc`에서 `samplesQueue`와 `fftQueue`를 각각 모니터링하며, 데이터가 도착하면 `sendSamplesToClient()` 또는 `sendFFTToClient()` 메서드를 호출하여 데이터를 WebSocket으로 전송합니다.

4. **`sendSamplesToClient()`와 `sendFFTToClient()` 메서드**:
   - 각 메서드는 해당 데이터를 WebSocket을 통해 클라이언트에 전송합니다.

### 큐 사용의 이점
- **데이터 분리 관리**: `samples`와 `fft` 데이터를 각각의 큐에서 관리하여, 서로 독립적으로 처리할 수 있습니다.
- **동기화 관리**: 각 큐가 독립적으로 데이터를 처리하므로, 동기화와 데이터 충돌 문제가 줄어듭니다.

이 구조는 `samples`와 `fft` 데이터를 각각 큐를 통해 안정적으로 관리하고, 웹 클라이언트에 전송할 수 있도록 합니다.


---

해당 기능을
asyncwebserver를 이용하여
web page에 
Waveform, Spectrogram, Graphic Equaliser 
3가지를 체크박스로 디스플레이되게 

samples, fft 데이터를 각각 websocket으로 전송
웹페이지 체크박스로 3가지 차트 디스플리이 여부 제어
html 파일을 littlefs를 사용하게 변경


