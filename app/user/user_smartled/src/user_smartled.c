#include "esp_common.h"
#include "alink_export.h"

#include "user_smartled.h"

#include "user_config.h"
#include "hnt_interface.h"

#include "driver/key.h"
#include "driver/gpio.h"
#include "ledctl.h"

#if USER_SMARTLED

LOCAL uint8 smartled_switch_status = 0;

void ICACHE_FLASH_ATTR
smartled_power_led_status(unsigned char power_led_status)
{
    if(power_led_status)//on
        GPIO_OUTPUT_SET(GPIO_ID_PIN(SMARTLED_POWER_LED_GPIO), GPIO_VALUE_0);
    else//off
        GPIO_OUTPUT_SET(GPIO_ID_PIN(SMARTLED_POWER_LED_GPIO), GPIO_VALUE_1);
}

int ICACHE_FLASH_ATTR
eSmartLedGetPower(char *paramName, char *value)
{        
    sprintf(value, "%d", smartled_switch_status);

    return 0;
}

int ICACHE_FLASH_ATTR
eSmartLedSetPower(char *paramName, char *value)
{
    smartled_switch_status = atoi(value);   
    smartled_switch_status = (smartled_switch_status? GPIO_VALUE_1 : GPIO_VALUE_0);
    
    GPIO_OUTPUT_SET(SMARTLED_CTRL_IO_GPIO, smartled_switch_status);
    if(smartled_switch_status)
        smartled_power_led_status(POWER_LED_ON);
    else
        smartled_power_led_status(POWER_LED_OFF);

    return 0;    
}

void ICACHE_FLASH_ATTR
smartled_gpio_status_init(void)
{   
	GPIO_ConfigTypeDef pGPIOConfig;
	pGPIOConfig.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;
	pGPIOConfig.GPIO_Pullup = GPIO_PullUp_EN;
	pGPIOConfig.GPIO_Mode = GPIO_Mode_Output;
    
	pGPIOConfig.GPIO_Pin = GPIO_Pin_12;	//power led
	gpio_config(&pGPIOConfig);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);	//close power led
	
	pGPIOConfig.GPIO_Pin = GPIO_Pin_13;	//wifi led
	gpio_config(&pGPIOConfig);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 1);	//close wifi led   
	
	pGPIOConfig.GPIO_Pin = GPIO_Pin_15;	//relay control
	gpio_config(&pGPIOConfig);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0);	//close relay control	
}

LOCAL os_timer_t smartled_status_led_timer;
LOCAL uint8_t wifi_led_status = 0;
LOCAL uint8_t wifi_led_blink_index = 0;
LOCAL uint8_t g_led_setting = 0;

void ICACHE_FLASH_ATTR
smartled_wifi_status_led_cb(void *arg)
{
    uint8 led_setting = *(uint8 *)arg;

    if(led_setting == WIFI_LED_SMARTCONFIG)
    {
        wifi_led_status = (wifi_led_status ? GPIO_VALUE_0 : GPIO_VALUE_1);
        GPIO_OUTPUT_SET(GPIO_ID_PIN(SMARTLED_WIFI_LED_GPIO), wifi_led_status);
    }
    else if(led_setting == WIFI_LED_CONNECTING_AP)
    {
        if(wifi_led_blink_index%4 == 2)
            GPIO_OUTPUT_SET(GPIO_ID_PIN(SMARTLED_WIFI_LED_GPIO), GPIO_VALUE_0);
        else
            GPIO_OUTPUT_SET(GPIO_ID_PIN(SMARTLED_WIFI_LED_GPIO), GPIO_VALUE_1);
            
        wifi_led_blink_index++;        
    }
    else if(led_setting == WIFI_LED_CONNECTED_AP)
    {
        if((wifi_led_blink_index%8 == 2)
            ||(wifi_led_blink_index%8 == 4))
            GPIO_OUTPUT_SET(GPIO_ID_PIN(SMARTLED_WIFI_LED_GPIO), GPIO_VALUE_0);
        else
            GPIO_OUTPUT_SET(GPIO_ID_PIN(SMARTLED_WIFI_LED_GPIO), GPIO_VALUE_1);
            
        wifi_led_blink_index++;    
    }
}

void ICACHE_FLASH_ATTR
smartled_wifi_status_led(uint8 led_setting)
{
	os_printf("[%s][%d] wifi led %d \n\r",__FUNCTION__,__LINE__,led_setting);
    g_led_setting = led_setting;

    if(led_setting == WIFI_LED_OFF)
    {
        os_timer_disarm(&smartled_status_led_timer);          
        wifi_led_status = 0;
        GPIO_OUTPUT_SET(GPIO_ID_PIN(SMARTLED_WIFI_LED_GPIO), GPIO_VALUE_1);
    } 
    else if(led_setting == WIFI_LED_SMARTCONFIG)
    {
        os_timer_disarm(&smartled_status_led_timer);          
        os_timer_setfn(&smartled_status_led_timer, (os_timer_func_t *)smartled_wifi_status_led_cb, &g_led_setting);
        os_timer_arm(&smartled_status_led_timer, 250, 1);
    }
    else if(led_setting == WIFI_LED_CONNECTING_AP)
    {
        os_timer_disarm(&smartled_status_led_timer);          
        os_timer_setfn(&smartled_status_led_timer, (os_timer_func_t *)smartled_wifi_status_led_cb, &g_led_setting);
        os_timer_arm(&smartled_status_led_timer, 500, 1);
    }
    else if(led_setting == WIFI_LED_CONNECTED_AP)
    {
        os_timer_disarm(&smartled_status_led_timer);          
        os_timer_setfn(&smartled_status_led_timer, (os_timer_func_t *)smartled_wifi_status_led_cb, &g_led_setting);
        os_timer_arm(&smartled_status_led_timer, 500, 1);        
    }
    else if(led_setting == WIFI_LED_CONNECTED_SERVER)
    {
        os_timer_disarm(&smartled_status_led_timer);          
        wifi_led_status = 1;
        GPIO_OUTPUT_SET(GPIO_ID_PIN(SMARTLED_WIFI_LED_GPIO), GPIO_VALUE_0);
    }    
}


LOCAL struct single_key_param *single_key[SMARTLED_KEY_NUM];
LOCAL struct keys_param keys;
LOCAL uint8 power_long_press_flag = 0;
LOCAL uint8 wifi_long_press_flag = 0;

LOCAL void ICACHE_FLASH_ATTR
smartled_power_short_press(void)
{
	os_printf("[%s][%d]\n\r",__FUNCTION__,__LINE__);

    if(power_long_press_flag) 
    {           
        power_long_press_flag = 0;           
        return ;       
    }

    if(smartled_switch_status)
    {
        eSmartLedSetPower(NULL,"0");        
    }
    else
    {
        eSmartLedSetPower(NULL,"1");                
    }
    hnt_device_status_change();
}

LOCAL void ICACHE_FLASH_ATTR
smartled_power_long_press(void)
{
    os_printf("[%s][%d]\n\r",__FUNCTION__,__LINE__);
    
    power_long_press_flag = 1;           

    vTaskDelay(100);

    system_restart();
}

LOCAL void ICACHE_FLASH_ATTR
smartled_wifi_short_press(void)
{
    if(wifi_long_press_flag) 
    {           
        wifi_long_press_flag = 0;           
        return ;       
    }

    os_printf("[%s][%d]\n\r",__FUNCTION__,__LINE__);

    setSmartConfigFlag(0x1);   // wifi key short press enter smartconfig
    vTaskDelay(100);
    system_restart();

}

LOCAL void ICACHE_FLASH_ATTR
smartled_wifi_long_press(void)
{
    os_printf("[%s][%d]\n\r",__FUNCTION__,__LINE__);
    wifi_long_press_flag = 1;
    
	need_factory_reset = 1;
}

void ICACHE_FLASH_ATTR
smartled_key_button_init(void)
{
	single_key[0] = key_init_single(SMARTLED_KEY1_IO_GPIO, SMARTLED_KEY1_IO_MUX, SMARTLED_KEY1_IO_FUNC,
                                	smartled_power_long_press, smartled_power_short_press);
    single_key[1] =	key_init_single(SMARTLED_KEY2_IO_GPIO, SMARTLED_KEY2_IO_MUX, SMARTLED_KEY2_IO_FUNC, 
                                    smartled_wifi_long_press, smartled_wifi_short_press);   

	keys.key_num = SMARTLED_KEY_NUM;
	keys.single_key = single_key;
	key_init(&keys);
}

deviceParameter_t DeviceParamList[] = {
{SMARTLED_POWER, eSmartLedGetPower, eSmartLedSetPower}
//{SMARTLED_COLOR_TEMPERATURE, eSmartLedGetVirtualDevice, eSmartLedSetVirtualDevice},
//{SMARTLED_LIGHT_BRIGHTNESS, eSmartLedGetVirtualDevice, eSmartLedSetVirtualDevice},
//{SMARTLED_TIMEDELAY_POWEROFF, eSmartLedGetVirtualDevice, eSmartLedSetVirtualDevice},
//{SMARTLED_WORKMODE_MASTERLIGHT, eSmartLedGetVirtualDevice, eSmartLedSetVirtualDevice},
//{SMARTLED_WORKMODE, NULL, eSmartLedSetVirtualDevice}
};


customInfo_t customInfo;

void ICACHE_FLASH_ATTR
hnt_custom_info_init(void)
{
	strcpy(customInfo.name, DEV_NAME);
	strcpy(customInfo.sn, DEV_SN);
	strcpy(customInfo.model, DEV_MODEL);
	strcpy(customInfo.brand, DEV_BRAND);

	strcpy(customInfo.type, DEV_TYPE);
	strcpy(customInfo.version, DEV_VERSION);
	strcpy(customInfo.category, DEV_CATEGORY);
	strcpy(customInfo.manufacturer, DEV_MANUFACTURE);

	strcpy(customInfo.key, ALINK_KEY);
	strcpy(customInfo.secret, ALINK_SECRET);
    
	strcpy(customInfo.key_sandbox, ALINK_KEY_SANDBOX);
	strcpy(customInfo.secret_sandbox, ALINK_SECRET_SANDBOX);

    hnt_custom_info_regist(&customInfo);
}


void
user_custom_init(void)
{        
	os_printf("%s,%d\n", __FUNCTION__,__LINE__);
    hnt_custom_info_init();
    
    hnt_param_array_regist(&DeviceParamList[0], sizeof(DeviceParamList)/sizeof(DeviceParamList[0]));

    hnt_wifi_led_func_regist(smartled_wifi_status_led);

    smartled_gpio_status_init();
    smartled_key_button_init();
}
#endif
