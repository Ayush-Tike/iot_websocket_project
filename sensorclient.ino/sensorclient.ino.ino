
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <DHT.h>

// Wi-Fi credentials
const char* WIFI_SSID = "Sid";
const char* WIFI_PASSWORD = "Sid@12345";

// Replace with your computer's LAN IPv4 address
const char* SERVER_IP = "10.220.171.29";
const uint16_t SERVER_PORT = 8765;

#define DHTPIN 5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WebSocketsClient webSocket;

unsigned long lastRead = 0;
const unsigned long READ_INTERVAL = 2500;


void webSocketEvent(WStype_t type, uint8_t* payload,
                    size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("WebSocket CONNECTED");
      Serial.print("Server: ");
      Serial.print(SERVER_IP);
      Serial.print(":");
      Serial.println(SERVER_PORT);
      break;

    case WStype_DISCONNECTED:
      Serial.println("WebSocket DISCONNECTED");
      break;

    case WStype_ERROR:
      Serial.println("WebSocket ERROR");
      if (payload != nullptr && length > 0) {
        Serial.print("Details: ");
        Serial.write(payload, length);
        Serial.println();
      }
      break;

    default:
      break;
  }
}


void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  webSocket.begin(SERVER_IP, SERVER_PORT, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(3000);
}

void loop() {
  webSocket.loop();

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  unsigned long now = millis();

  if (now - lastRead >= READ_INTERVAL) {
    lastRead = now;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("DHT11 read failed. Check wiring.");
      return;
    }

    // Send a JSON message through WebSocket
    String message = "{";
    message += "\"device\":\"ESP32-DHT11\",";
    message += "\"temperature\":";
    message += String(temperature, 1);
    message += ",\"humidity\":";
    message += String(humidity, 1);
    message += ",\"uptime_ms\":";
    message += String(now);
    message += "}";

    if (webSocket.isConnected()) {
      webSocket.sendTXT(message);
      Serial.println("Sent: " + message);
    } else {
      Serial.println("Waiting for WebSocket connection...");
    }
  }
}
