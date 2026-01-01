#ifndef CONFIG_H
#define CONFIG_H

#define LED_STRIP_GPIO    19
#define LED_STRIP_LENGTH  60

#define WIFI_SSID "wifi name"
#define WIFI_PASSWORD "password"

#define SENSOR_SAMPLE_PERIOD 500 // ms

typedef struct {
    bool led_on;
    int red;
    int green;
    int blue;
} led_state_t;

typedef struct {
    float distance; // cm
} sensor_data_t;

extern led_state_t led_state;
extern sensor_data_t sensor_data;

#endif
