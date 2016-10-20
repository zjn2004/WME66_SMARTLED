#ifndef _USER_SMARTLED_H
#define _USER_SMARTLED_H

#define GPIO_VALUE_0 0
#define GPIO_VALUE_1 1

/* LED */
#define SMARTLED_POWER_LED_GPIO     12

#define SMARTLED_WIFI_LED_GPIO      13


/* KEY */
#define SMARTLED_KEY_NUM          2

#define SMARTLED_KEY1_IO_MUX       PERIPHS_IO_MUX_MTMS_U
#define SMARTLED_KEY1_IO_GPIO      14
#define SMARTLED_KEY1_IO_FUNC      FUNC_GPIO14

#define SMARTLED_KEY2_IO_MUX       PERIPHS_IO_MUX_GPIO4_U
#define SMARTLED_KEY2_IO_GPIO      4
#define SMARTLED_KEY2_IO_FUNC      FUNC_GPIO4

/* CONTROL */
#define SMARTLED_CTRL_IO_MUX     PERIPHS_IO_MUX_MTDO_U
#define SMARTLED_CTRL_IO_GPIO    15
#define SMARTLED_CTRL_IO_FUNC    FUNC_GPIO15

/* PATH */
#define SMARTLED_POWER                               "OnOff_Power"
#define SMARTLED_COLOR_TEMPERATURE                   "Color_Temperature"
#define SMARTLED_LIGHT_BRIGHTNESS                    "Light_Brightness"
#define SMARTLED_TIMEDELAY_POWEROFF                  "TimeDelay_PowerOff"
#define SMARTLED_WORKMODE_MASTERLIGHT                "WorkMode_MasterLight"
#define SMARTLED_WORKMODE                            "Work_Mode"


typedef void (*hnt_power_led_func)(unsigned char);
#define POWER_LED_ON             1
#define POWER_LED_OFF            0

int ICACHE_FLASH_ATTR
eSmartLedGetPower(void);

int ICACHE_FLASH_ATTR
eSmartLedSetPower(uint8 value);

#endif /*_USER_SMARTLED_H */
