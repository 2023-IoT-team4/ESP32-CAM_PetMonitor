#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>

#define CAMERA_MODEL_AI_THINKER

/************************************************************
// Wifi
const char* ssid = "";
const char* password = "";

// Websocket server
const char* websocket_server_host = "";
const uint16_t websocket_server_port = 0000;
************************************************************/

#include "camera_pins.h"
#include "config.h"

using namespace websockets;
WebsocketsClient client;

// 10초마다 Websocket 서버와 연결이 되었는지 확인한다.
unsigned long currentTime;
unsigned long lastAvailableChecked = 0;
const unsigned long clickTimeout = 10 * 1000;

bool connectToServer() {
  return client.connect(websocket_server_host, websocket_server_port, "/");
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    Serial.println("hello");
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 20;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  // Websocket connection
  while (!connectToServer()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Websocket Connected!");
  lastAvailableChecked = millis();
}

void loop() {
  if ((millis() - lastAvailableChecked) > clickTimeout) {
    Serial.println("연결 확인..");
    if (client.available()) {
      lastAvailableChecked = millis();
    } else {
      // 서버와 연결이 끊어졌을 시, 다시 연결 시도
      while (!connectToServer()) {
        Serial.print(".");
        delay(500);
      }
      Serial.println("\nWebsocket Reconnected!");
    }
  }

  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Camera capture failed");
    esp_camera_fb_return(fb);
    return;
  }

  if (fb->format != PIXFORMAT_JPEG) {
    Serial.println("Non-JPEG data not implemented");
    return;
  }

  client.sendBinary((const char *)fb->buf, fb->len);

  esp_camera_fb_return(fb);
}