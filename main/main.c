#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF    1100       // Default reference voltage in mV
#define NO_OF_SAMPLES   64         // Number of samples to average

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_0;  // GPIO36 if ADC1
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_11;    // Full voltage range (0-3.6V)
static const adc_unit_t unit = ADC_UNIT_1;

void app_main(void)
{
    // Initialize ADC characteristics
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);

    // Configure ADC
    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);

    while(1)
    {
        uint32_t adc_reading = 0;
        // Take multiple samples for better accuracy
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            adc_reading += adc1_get_raw(channel);
        }
        adc_reading /= NO_OF_SAMPLES;

        // Convert raw reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

        printf("Soil Moisture Sensor Raw: %ld, Voltage: %ld mV\n", adc_reading, voltage);

        vTaskDelay(pdMS_TO_TICKS(2000));  // Wait 2 seconds
    }
}
