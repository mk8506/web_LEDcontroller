#include "led_module.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt.h"
#include "led_strip.h"
#include "sensor_module.h"

static const char *TAG = "LED";

led_strip_handle_t strip = NULL;
led_state_t led_state = {false, 0, 0, 0};

void led_init(void) {
    ESP_LOGI(TAG, "LED init");

    // RMT 채널 설정
    rmt_channel_handle_t led_chan = NULL;
    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = LED_STRIP_GPIO,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    // LED 인코더 생성
    led_strip_encoder_config_t encoder_config = {.resolution = 10 * 1000 * 1000};
    rmt_encoder_handle_t led_encoder = NULL;
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    led_strip_config_t strip_config = {.max_leds = LED_STRIP_LENGTH};
    strip_config.strip_gpio_num = LED_STRIP_GPIO;
    strip_config.led_model = LED_MODEL_WS2812;
    led_strip_rmt_config_t rmt_config = {.resolution_hz = 10 * 1000 * 1000, .flags.with_dma=false};

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &strip));
    ESP_LOGI(TAG, "LED strip initialized on GPIO %d", LED_STRIP_GPIO);
}

void led_task(void *pvParameters) {
    while(1) {
        if (led_state.led_on) {
            int r = 0, g = 0, b = 0;
            if(sensor_data.distance < 10) { r=255; g=0; b=0; }
            else if(sensor_data.distance < 30) { r=255; g=255; b=0; }
            else { r=0; g=255; b=0; }

            for(int i=0;i<LED_STRIP_LENGTH;i++)
                led_strip_set_pixel(strip,i,r,g,b);
            led_strip_refresh(strip);
        } else {
            led_strip_clear(strip);
            led_strip_refresh(strip);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void led_set_color(int red, int green, int blue) {
    led_state.red = red;
    led_state.green = green;
    led_state.blue = blue;
}

void led_toggle(void) {
    led_state.led_on = !led_state.led_on;
}
