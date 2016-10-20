#include "ledctl.h"

hnt_wifi_led_func wifi_led_status_func = NULL;
static unsigned char wifi_status = WIFI_LED_OFF;

void wifi_led_set_status(unsigned char led_status)
{    
    wifi_status = led_status;

    if(wifi_led_status_func)
        wifi_led_status_func(led_status);
}

unsigned char wifi_led_get_status(void)
{    
    return wifi_status;
}


void
hnt_wifi_led_func_regist(void *func)
{
    wifi_led_status_func = (hnt_wifi_led_func)func;
}
