#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "driver/i2c.h"
#include "driver/adc.h"

static const char *I2CUTILS_MODULE_TAG = "i2c_utils";

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SCL_IO   CONFIG_I2C_MASTER_SCL_IO
#define I2C_MASTER_SDA_IO   CONFIG_I2C_MASTER_SDA_IO
#define I2C_MASTER_FREQ_HZ  CONFIG_I2C_MASTER_FREQ_HZ


void i2c_master_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void i2c_scanner()
{
    for (int addr = 1; addr < 127; addr++) {

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);

        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);

        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 50 / portTICK_PERIOD_MS);

        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            ESP_LOGI(I2CUTILS_MODULE_TAG, "Device found at 0x%02X", addr);
        }
    }
}

esp_err_t sht3x_read(float *temperature, float *humidity)
{
    uint8_t cmd[2] = {0x24, 0x00};
    uint8_t data[6];

    i2c_cmd_handle_t handle = i2c_cmd_link_create();

    i2c_master_start(handle);
    i2c_master_write_byte(handle, (0x44 << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(handle, cmd, 2, true);
    i2c_master_stop(handle);

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, handle, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(handle);

    if (ret != ESP_OK) return ret;

    vTaskDelay(pdMS_TO_TICKS(20));

    handle = i2c_cmd_link_create();

    i2c_master_start(handle);
    i2c_master_write_byte(handle, (0x44 << 1) | I2C_MASTER_READ, true);
    i2c_master_read(handle, data, 6, I2C_MASTER_LAST_NACK);
    i2c_master_stop(handle);

    ret = i2c_master_cmd_begin(I2C_NUM_0, handle, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(handle);

    if (ret != ESP_OK) return ret;

    uint16_t rawTemp = (data[0] << 8) | data[1];
    uint16_t rawHum  = (data[3] << 8) | data[4];

    *temperature = -45 + 175 * ((float)rawTemp / 65535.0);
    *humidity = 100 * ((float)rawHum / 65535.0);

    return ESP_OK;
}