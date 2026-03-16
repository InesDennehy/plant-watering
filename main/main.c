/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "driver/i2c.h"
#include "driver/adc.h"

#include "i2cutils.c"

static const char *TAG = "plant_monitor";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/

#define CONFIG_BLINK_PERIOD 1000

void init_moisture_analog_sensor()
{ 
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_12);
}

void app_main(void)
{
    i2c_master_init();
    init_moisture_analog_sensor();
    i2c_scanner();
    int soil_moisture;
    float air_humidity;
    float air_temperature;
    float voltage;
    while (1) {
        soil_moisture = adc1_get_raw(ADC1_CHANNEL_7);
        voltage = soil_moisture * (3.3 / 4095);
        ESP_LOGI(TAG, "Voltage: %.2f V, measured: %d", voltage, soil_moisture);
        sht3x_read(&air_temperature, &air_humidity);
        ESP_LOGI(TAG, "Air temperature: %.2f °C, Air humidity: %.2f %%", air_temperature, air_humidity);
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}
