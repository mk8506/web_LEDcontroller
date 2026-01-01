
// web_module.c
#include "web_module.h"
#include "led_module.h"
#include "sensor_module.h"
#include "esp_http_server.h"
#include "esp_log.h"

static const char *TAG = "WEB";

esp_err_t toggle_handler(httpd_req_t *req) {
    led_toggle();
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_sendstr(req, led_state.led_on ? "LED ON" : "LED OFF");
    return ESP_OK;
}

void start_webserver_module(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    httpd_start(&server, &config);

    httpd_uri_t toggle_uri = {.uri="/toggle",.method=HTTP_GET,.handler=toggle_handler,.user_ctx=NULL};
    httpd_register_uri_handler(server,&toggle_uri);
}

void websocket_broadcast_task(void *pvParameters) {
    while(1) {
        // WebSocket 연결된 클라이언트에 sensor_data.distance 전송
        // 예: {"distance":35.2}
        vTaskDelay(pdMS_TO_TICKS(SENSOR_SAMPLE_PERIOD));
    }
}
