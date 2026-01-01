// sensor_module.c
#include "sensor_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "SENSOR";
sensor_data_t sensor_data = {0};

void sensor_init(void) {
    // 거리 센서 초기화 코드 들어가야함(예: HC-SR04, VL53L0X 등)
    ESP_LOGI(TAG, "Distance sensor initialized");
}

void sensor_task(void *pvParameters) {
    while(1) {
        // 실제 센서 읽기 코드 들어가야함(여기서는 모킹)
        sensor_data.distance = 5 + (esp_random() % 50); // 5~55cm 랜덤

        vTaskDelay(pdMS_TO_TICKS(SENSOR_SAMPLE_PERIOD));
    }
}
