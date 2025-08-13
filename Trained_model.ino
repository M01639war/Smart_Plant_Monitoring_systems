#include <DHT.h>
#include <DHT_U.h>
#include <edge-impulse-sdk/classifier/ei_run_classifier.h> // Edge Impulse model

#define DHTPIN 4
#define DHTTYPE DHT22
#define SOIL_PIN 36
#define RELAY_PIN 18


DHT dht(DHTPIN, DHTTYPE);

extern "C" {
    #include "edge-impulse-sdk/classifier/ei_run_classifier.h"
}

void setup() {
    Serial.begin(115200);
    dht.begin();
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW); // Pump off initially
    delay(2000);
    Serial.println("Smart Plant Monitoring with Pump Control");
}

void loop() {
    // Read sensors
    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();
    int soilRaw = analogRead(SOIL_PIN);
    float soilPercent = map(soilRaw, 4095, 0, 0, 100); // Adjust mapping to match your sensor

    if (isnan(temp) || isnan(hum)) {
        Serial.println("DHT22 read failed!");
        delay(2000);
        return;
    }

    Serial.printf("Temp: %.1f°C, Humidity: %.1f%%, Soil: %.1f%%\n", temp, hum, soilPercent);

    // Prepare TinyML input
    float features[] = { temp, hum, soilPercent };
    signal_t signal;
    numpy::signal_from_buffer(features, sizeof(features) / sizeof(features[0]), &signal);

    // Run inference
    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

    if (res != EI_IMPULSE_OK) {
        Serial.printf("ERR: Failed to run classifier (%d)\n", res);
        delay(2000);
        return;
    }

    // Display predictions
    Serial.println("=== Prediction ===");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        Serial.printf("%s: %.2f%%\n", result.classification[ix].label, result.classification[ix].value * 100);
    }
    Serial.println("==================");

    // Control relay based on prediction
    if (strcmp(result.classification[0].label, "on") == 0 && result.classification[0].value > 0.7) {
        digitalWrite(RELAY_PIN, HIGH); // Turn pump ON
        Serial.println(" Pump ON");
    }
    else {
        digitalWrite(RELAY_PIN, LOW); // Turn pump OFF
        Serial.println(" Pump OFF");
    }

    delay(3000);
}