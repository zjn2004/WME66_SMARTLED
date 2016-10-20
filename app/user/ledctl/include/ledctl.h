#ifndef _LEDCTL_H_
#define _LEDCTL_H_

#include "esp_common.h" 

typedef void (*hnt_wifi_led_func)(unsigned char);

#define WIFI_LED_OFF                  0
#define WIFI_LED_SMARTCONFIG          1
#define WIFI_LED_CONNECTING_AP        2
#define WIFI_LED_CONNECTED_AP         3
#define WIFI_LED_CONNECTED_SERVER     4


void wifi_led_set_status(unsigned char led_status);

unsigned char wifi_led_get_status(void);

void hnt_wifi_led_func_regist(void *func);

#endif /*_LEDCTL_H_*/
