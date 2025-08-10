#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// ---------- CONFIGURE ----------
#define WIFI_SSID       "vivo Y28 5G"
#define WIFI_PASSWORD   "Mahes2345#"
#define THINGSBOARD_SERVER "thingsboard.cloud"
#define TOKEN           "ahuhdpor2hz4i4m96czk"

#define DHTPIN 4
#define DHTTYPE DHT22
#define SOIL_PIN 34
#define PUMP_PIN 18
#define SOIL_THRESHOLD 30 // Below this %, pump turns ON
// -------------------------------

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

SemaphoreHandle_t dataMutex;
float temperature = 0.0;
float humidity = 0.0;
int soilPercent = 0;
bool pumpStatus = false;

void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println(" Connected!");
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to ThingsBoard...");
    if (client.connect("ESP32_Device", TOKEN, NULL)) {
      Serial.println(" connected!");
    } else {
      Serial.print(" failed, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

// ---- Tasks ----
void TaskDHT(void *pvParameters) {
  for (;;) {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      xSemaphoreTake(dataMutex, portMAX_DELAY);
      temperature = t;
      humidity = h;
      xSemaphoreGive(dataMutex);
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void TaskSoil(void *pvParameters) {
  for (;;) {
    int soilValue = analogRead(SOIL_PIN);
    int soilP = map(soilValue, 4095, 0, 0, 100);
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    soilPercent = soilP;
    xSemaphoreGive(dataMutex);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void TaskPump(void *pvParameters) {
  for (;;) {
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    int soil = soilPercent;
    xSemaphoreGive(dataMutex);

    if (soil < SOIL_THRESHOLD) {
      digitalWrite(PUMP_PIN, HIGH);
      pumpStatus = true;
    } else {
      digitalWrite(PUMP_PIN, LOW);
      pumpStatus = false;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void TaskMQTT(void *pvParameters) {
  for (;;) {
    if (!client.connected()) connectMQTT();
    client.loop();

    float t, h;
    int s;
    bool p;
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    t = temperature;
    h = humidity;
    s = soilPercent;
    p = pumpStatus;
    xSemaphoreGive(dataMutex);

    String payload = "{\"temperature\":";
    payload += t;
    payload += ",\"humidity\":";
    payload += h;
    payload += ",\"soil_moisture\":";
    payload += s;
    payload += ",\"pump_status\":\"";
    payload += (p ? "ON" : "OFF");
    payload += "\"}"; //JSON data to publish to thingsboard.cloud

    Serial.println("Publishing: " + payload);
    client.publish("v1/devices/me/telemetry", payload.c_str());

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(SOIL_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  dht.begin();
  connectWiFi();
  client.setServer(THINGSBOARD_SERVER, 1883);

  dataMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(TaskDHT, "TaskDHT", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskSoil, "TaskSoil", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskPump, "TaskPump", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(TaskMQTT, "TaskMQTT", 8192, NULL, 1, NULL, 1);
}

void loop() {
  // Everything is handled in FreeRTOS tasks
}