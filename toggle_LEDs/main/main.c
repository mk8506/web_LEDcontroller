//C:\Users\SienMinj\esp\v5.5.1\esp-idf $export.bat
//D:\My_Programming\ESP\toggle_LEDs $idf.py build

#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "led_strip.h"
#include "led_strip_rmt_encoder.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

// --- LED strip variables ---
#define RMT_CHANNEL       RMT_CHANNEL_0
#define LED_TYPE          LED_STIP_WS2812B
#define LED_STRIP_GPIO    19
#define LED_STRIP_LENGTH  60
led_strip_handle_t strip = NULL;

// --- wifi variables ---
const char* ssid = "wifi name"; //esp32 only works on 24Ghz
const char* password = "password"; //connect both laptop and esp to the same wifi
// turn on Maximize Capability on hotspot

// --- web control ---
bool led_on = false;

// --- LOG --- 
static const char *TAG = "ex";

// --- declare functions ---
void wifi_init(void);
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data);
void start_webserver(void);
esp_err_t toggle_handler(httpd_req_t *req);
void led_init(void);
void led_loop(void *pvParameters);

// --- app_main : use functions ---
void app_main(void) {
    wifi_init();       // connect Wi-Fi
    led_init();        // initialize LED (RMT settings, etc)
    start_webserver(); // start webserver
    xTaskCreate(led_loop, "led_loop", 4096, NULL, 5, NULL); // LED control while loop //here
}

esp_err_t toggle_handler(httpd_req_t *req) {
    led_on = !led_on;

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "*");

    httpd_resp_sendstr(req, led_on ? "LED ON" : "LED OFF");
    return ESP_OK;
}

void start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    httpd_start(&server, &config);

    httpd_uri_t toggle_uri = {
        .uri = "/toggle",
        .method = HTTP_GET,
        .handler = toggle_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &toggle_uri);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect(); // retry automatically
    }
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI("IP", "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void wifi_init(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // register handlers BEFORE wifi_start()
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {0};
    strcpy((char*)wifi_config.sta.ssid, ssid);
    strcpy((char*)wifi_config.sta.password, password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // start wifi AFTER registering handlers
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI("WIFI", "wifi_init finished.");
}

void led_init() {
    ESP_LOGI(TAG, "init");

    // Step 1: Create RMT TX channel
    rmt_channel_handle_t led_chan = NULL;
    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = LED_STRIP_GPIO,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz = 0.1us tick
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    // Step 2: Create LED strip encoder
    led_strip_encoder_config_t encoder_config = {
        .resolution = 10 * 1000 * 1000, // 10MHz
    };
    rmt_encoder_handle_t led_encoder = NULL;
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    // Step 3: Enable RMT channel
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    // Step 4: Create LED strip object
    led_strip_config_t strip_config = {
        .max_leds = LED_STRIP_LENGTH,
    };
    strip_config.strip_gpio_num = LED_STRIP_GPIO;
    strip_config.led_model = LED_MODEL_WS2812; // or LED_MODEL_WS2812B depending on ESP-IDF version

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10 MHz
        .flags.with_dma = false
        //.rmt_channel = led_chan,
    };
    //esp_err_t led_strip_new_rmt_device(const led_strip_config_t *led_config, const led_strip_rmt_config_t *rmt_config, led_strip_handle_t *ret_strip)

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &strip));

    //log
    ESP_LOGI(TAG, "LED strip initialized on GPIO %d with %d LEDs", LED_STRIP_GPIO, LED_STRIP_LENGTH);
}

void led_loop(void *pvParameters) {
    uint8_t r = 0, g = 0, b = 0;

    while (1) {

        if (led_on) {
            // LED ON behavior
            for (int i = 0; i < LED_STRIP_LENGTH / 3; i++) {
                r = (i * 30) % 255;
                g = (255 - i * 30) % 255;
                b = (i * 15) % 255;
                led_strip_set_pixel(strip, i, r, g, b);
            }
            led_strip_refresh(strip);
        } else {
            // LED OFF behavior
            led_strip_clear(strip);
            led_strip_refresh(strip);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

  