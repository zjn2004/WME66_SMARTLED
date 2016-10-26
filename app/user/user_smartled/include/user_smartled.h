#ifndef _USER_SMARTLED_H
#define _USER_SMARTLED_H

/* device info */
#define DEV_NAME "ALINKTEST"
#define DEV_CATEGORY "LIVING"
#define DEV_TYPE "LIGHT"
#define DEV_BRAND "SEAING"

#ifdef PASS_THROUGH
#define DEV_MODEL "ALINKTEST_LIVING_LIGHT_SMARTLED_LUA"
#define ALINK_KEY "bIjq3G1NcgjSfF9uSeK2"
#define ALINK_SECRET "W6tXrtzgQHGZqksvJLMdCPArmkecBAdcr2F5tjuF"
#else
#define DEV_MODEL "ALINKTEST_LIVING_LIGHT_SMARTLED"
#define ALINK_KEY "ljB6vqoLzmP8fGkE6pon"
#define ALINK_SECRET "YJJZjytOCXDhtQqip4EjWbhR95zTgI92RVjzjyZF"
#endif
#define DEV_MANUFACTURE "ALINKTEST"
/*sandbox key/secret*/
#define ALINK_KEY_SANDBOX "dpZZEpm9eBfqzK7yVeLq"
#define ALINK_SECRET_SANDBOX "THnfRRsU5vu6g6m9X6uFyAjUWflgZ0iyGjdEneKm"

#define DEV_SN "1234567890"
#define DEV_VERSION "1.0.3"
#define DEV_MAC "19:FE:34:A2:C7:1A"	//"AA:CC:CC:CA:CA:01" // need get from device
#define DEV_CHIPID "3D0044000F47333139373030"	// need get from device


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
eSmartLedGetPower(char *paramName, char *value);

int ICACHE_FLASH_ATTR
eSmartLedSetPower(char *paramName, char *value);

#endif /*_USER_SMARTLED_H */
