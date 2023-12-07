# ESP32-CAM_PetMonitor
반려동물을 모니터링하는 ESP32-CAM 코드입니다.

Node.js의 Websocket 서버의 client 입장으로 지속적으로 Camera 이미지를 캡쳐받고, 이를 Websocket 서버로 보냅니다.

이 과정에서 10초마다 Websocket 서버와의 연결이 유지되고 있는지 확인합니다.



### 사용되는 라이브러리
```
ArduinoWebsockets
```



### 새롭게 Define 해야하는 변수
```
// Wifi
const char* ssid = "";
const char* password = "";

// Websocket server
const char* websocket_server_host = "";
const uint16_t websocket_server_port = 0000;
```

