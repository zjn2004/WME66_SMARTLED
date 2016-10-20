/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__
#define RESTORE_KEEP_TIMER 0

#define USE_OPTIMIZE_PRINTF
extern  int need_factory_reset ;
#define LFILE_START_ADDR  0xfe000// flash size is 0x200000
#define LFILE_SIZE 4096

#define USER_UART_CTRL_DEV_EN 	 (0)   // support allink uart smart device
#define USER_PWM_LIGHT_EN        (0)   // support alink pwm light  
#define USER_VIRTUAL_DEV_TEST	 (1)

#define CUS_GLOBAL_VER "AlinkV1.0.02-20160920"


#define ESP_DBG(a) os_printf("[dbg@%s,%d]",__FUNCTION__,__LINE__);os_printf a;os_printf("\r\n")   // for dbg print

typedef struct virtual_dev {   // real device update this Structure parameters
//	u32 power;
	u32 temp_value;
	u32 light_value;
	u32 time_delay;
	u32 work_mode;
}VIRTUAL_DEV;// virtual_device = {
//0x01, 0x30, 0x50, 0, 0x01};

extern VIRTUAL_DEV virtual_device;

/* device info */
#define DEV_NAME "ALINKTEST"
#define DEV_CATEGORY "LIVING"
#define DEV_TYPE "LIGHT"
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
/*设备硬件信息:系统上电后读取的硬件/固件信息,此处为演示需要,直接定义为宏.产品对接时,需要调用自身接口获取*/
#define DEV_SN "1234567890"
#define DEV_VERSION "1.0.3"
#define DEV_MAC "19:FE:34:A2:C7:1A"	//"AA:CC:CC:CA:CA:01" // need get from device
#define DEV_CHIPID "3D0044000F47333139373030"	// need get from device

void
user_custom_init(void);

void ICACHE_FLASH_ATTR
hnt_device_status_change(void);

#define USER_SMARTLED 1

#endif

