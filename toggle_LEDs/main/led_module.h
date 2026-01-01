#ifndef LED_MODULE_H
#define LED_MODULE_H

#include "config.h"
#include "esp_err.h"

void led_init(void);
void led_task(void *pvParameters);
void led_set_color(int red, int green, int blue);
void led_toggle(void);

#endif
