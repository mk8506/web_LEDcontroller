#include "wifi_module.h"
#include "led_module.h"
#include "web_module.h"
#include "sensor_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    wifi_init_module();
    led_init();
    sensor_init();
    start_webserver_module();

    xTaskCreate(led_task, "led_task", 4096, NULL, 5, NULL);
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
    xTaskCreate(websocket_broadcast_task, "ws_task", 4096, NULL, 5, NULL);
}
