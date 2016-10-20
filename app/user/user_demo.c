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

#include "esp_common.h" 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "alink_export.h"
#include "user_config.h"
#include "esp_ota.h"
#include "esp_system.h"
#if USER_UART_CTRL_DEV_EN
#include "user_uart.h" // user uart handler head
#endif
#if USER_PWM_LIGHT_EN
#include "user_light.h"  // user pwm light head
#endif
#include "ledctl.h" 

#define ENABLE_GPIO_KEY

#ifdef ENABLE_GPIO_KEY
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/key.h"
#include "espressif/esp8266/eagle_soc.h"
struct single_key_param *single_key[2];
struct keys_param keys;
static int is_long_press = 0;
#endif	

#define SMARTCONFIG_SUCCESS    0
#define SMARTCONFIG_FLAG       1
#define SOFTAP_FLAG            SMARTCONFIG_FLAG + 1
#define SMARTCONFIG_OFF        SOFTAP_FLAG + 1


static char vendor_ssid[32];
static char vendor_passwd[64];
static char *vendor_tpsk = "PLnBaCPHF7icf65a5nJmcL2GZC+w3vwCnH36k8O91og=";
int ICACHE_FLASH_ATTR vendor_callback(char *ssid, char *passwd, char *bssid, unsigned int security, char channel) 
{
	if (!ssid) {
        int ret = 0;
        ret = readSmartConfigFlag();    // -1 read flash fail!
        
		os_printf("zconfig timeout! setSmartConfigFlag %d\n",ret+1);
        
        setSmartConfigFlag(ret+1);
        
        vTaskDelay(100 / portTICK_RATE_MS);	 // 100 ms
		system_restart();
	} else {
    	setSmartConfigFlag(0);
        wifi_led_set_status(WIFI_LED_CONNECTING_AP);
		os_printf("ssid:%s ,key:%s\n", ssid,passwd);
		struct station_config *config = (struct station_config *)zalloc(sizeof(struct station_config));
		strncpy(config->ssid, ssid, 32);
		strncpy(config->password, passwd, 64);
		wifi_station_set_config(config);
		free(config);
		wifi_station_connect();
	} 

} 

unsigned portBASE_TYPE ICACHE_FLASH_ATTR stack_free_size() 
{
	xTaskHandle taskhandle;
	unsigned portBASE_TYPE stack_freesize = 0;
	taskhandle = xTaskGetCurrentTaskHandle();
	stack_freesize = uxTaskGetStackHighWaterMark(taskhandle);
	os_printf("stack free size =%u\n", stack_freesize);
	return stack_freesize;
}

void ICACHE_FLASH_ATTR startdemo_task(void *pvParameters) 
{
	os_printf("heap_size %d\n", system_get_free_heap_size());
	stack_free_size();
	while (1) {
		int ret = wifi_station_get_connect_status();   // wait for sys wifi connected OK.
		if (ret == STATION_GOT_IP)
			break;
		vTaskDelay(100 / portTICK_RATE_MS);	 // 100 ms
	}
	alink_demo();
	vTaskDelete(NULL);
}

int need_notify_app = 0;
#if 1
void ICACHE_FLASH_ATTR smartconfig_task(void *pvParameters) 
{
    wifi_led_set_status(WIFI_LED_SMARTCONFIG);

    os_printf("\nsmartconfig_task,tpsk %s\n",vendor_tpsk);
	zconfig_start(vendor_callback, vendor_tpsk);   // alink smart config start
	
	os_printf("waiting network ready...\r\n");
	while (1) {
		// device could show smart config status here, such as light Flashing.
		int ret = wifi_station_get_connect_status();
		if (ret == STATION_GOT_IP)
			break;
		vTaskDelay(100 / portTICK_RATE_MS);
	}
	need_notify_app = 1;
	vTaskDelete(NULL);
}
#else
void ICACHE_FLASH_ATTR wificonnect_task(void *pvParameters)  // it is a test router cfg info for wifi conncted
{
    signed char ret;
    wifi_set_opmode(STATION_MODE);
    struct ip_info sta_ipconfig;
    struct station_config config;
    bzero(&config, sizeof(struct station_config));
    sprintf(config.ssid, "IOT_DEMO_TEST");
    sprintf(config.password, "123456789");
    wifi_station_set_config(&config);
    wifi_station_connect();
    wifi_get_ip_info(STATION_IF, &sta_ipconfig);
    while(sta_ipconfig.ip.addr == 0 ) {
        vTaskDelay(1000 / portTICK_RATE_MS);
        wifi_get_ip_info(STATION_IF, &sta_ipconfig);
    }
    vTaskDelete(NULL);
}

#endif


void alink_softap_setup(void)
{
	/*
	 * wilress params: 11BGN
	 * channel: auto, or 1, 6, 11
	 * authentication: OPEN
	 * encryption: NONE
	 * gatewayip: 172.31.254.250, netmask: 255.255.255.0
	 * DNS server: 172.31.254.250. 	IMPORTANT!!!  ios depend on it!
	 * DHCP: enable
	 * SSID: 32 ascii char at most
	 * softap timeout: 5min
	 */
	//ssid: max 32Bytes(excluding '\0')
    
    wifi_set_opmode(SOFTAP_MODE);

    wifi_softap_dhcps_stop(); 

    struct ip_info ap_info;
    IP4_ADDR(&ap_info.ip, 172, 31, 254, 250);
    IP4_ADDR(&ap_info.netmask, 255, 255, 255, 0);
    IP4_ADDR(&ap_info.gw, 172, 31, 254, 250);

    if ( true != wifi_set_ip_info(SOFTAP_IF, &ap_info)) {
            os_printf("set default ip wrong\n");
    }
    
    wifi_softap_dhcps_start();

    struct softap_config config;
    wifi_softap_get_config(&config);
    memset(config.ssid, 0, sizeof(config.ssid));
	snprintf(config.ssid, 32, "alink_%s", DEV_MODEL);
    
    config.ssid_len = strlen(config.ssid);
    config.authmode = AUTH_OPEN;
    
    wifi_softap_set_config(&config);    
    
    os_printf("create softap ssid:%s\n",config.ssid);
    
}


/*
	以下为softap配网时，设备起的softap tcp server sample
*/
///////////////softap tcp server sample/////////////////////
#define	STR_SSID_LEN		(32 + 1)
#define STR_PASSWD_LEN		(64 + 1)
char alink_ssid[STR_SSID_LEN];
char alink_passwd[STR_PASSWD_LEN];
unsigned char alink_bssid[6];

#ifndef	info
#define info(format, ...)	printf(format, ##__VA_ARGS__)
#endif

#define SOFTAP_TCP_SERVER_PORT		(65125)

/* json info parser */
int get_ssid_and_passwd(char *msg)
{
	char *ptr, *end, *name;
	int len;

	//ssid
	name = "\"ssid\":";
	ptr = strstr(msg, name);
	if (!ptr) {
		info("%s not found!\n", name);
		goto exit;
	}
	ptr += strlen(name);
	while (*ptr++ == ' ');/* eating the beginning " */
	end = strchr(ptr, '"');
	len = end - ptr;

//	assert(len < sizeof(aws_ssid));
	strncpy(alink_ssid, ptr, len);
	alink_ssid[len] = '\0';

	//passwd
	name = "\"passwd\":";
	ptr = strstr(msg, name);
	if (!ptr) {
		info("%s not found!\n", name);
		goto exit;
	}

	ptr += strlen(name);
	while (*ptr++ == ' ');/* eating the beginning " */
	end = strchr(ptr, '"');
	len = end - ptr;

//	assert(len < sizeof(aws_passwd));
	strncpy(alink_passwd, ptr, len);
	alink_passwd[len] = '\0';

	//bssid-mac
	name = "\"bssid\":";
	ptr = strstr(msg, name);
	if (!ptr) {
		info("%s not found!\n", name);
		goto exit;
	}

	ptr += strlen(name);
	while (*ptr++ == ' ');/* eating the beginning " */
	end = strchr(ptr, '"');
	len = end - ptr;

#if 0
	memset(aws_bssid, 0, sizeof(aws_bssid));

	sscanf(ptr, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
			&aws_bssid[0], &aws_bssid[1], &aws_bssid[2],
			&aws_bssid[3], &aws_bssid[4], &aws_bssid[5]);
#endif

	return 0;
exit:
	return -1;
}

//setup softap server
bool alink_softap_tcp_server(void)
{
	struct sockaddr_in server, client;
	socklen_t socklen = sizeof(client);
	int fd = -1, connfd, len, ret;
	char *buf, *msg;
	int opt = 1, buf_size = 512, msg_size = 512;
    bool config_success = 0;

	info("setup softap & tcp-server\n");

	buf = malloc(buf_size);
	msg = malloc(msg_size);
//	assert(fd && msg);

	fd = socket(AF_INET, SOCK_STREAM, 0);
//	assert(fd >= 0);

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);//SOFTAP_GATEWAY_IP, 0xFAFE1FAC;
	server.sin_port = htons(SOFTAP_TCP_SERVER_PORT);

	ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
//	assert(!ret);

	ret = bind(fd, (struct sockaddr *)&server, sizeof(server));
//	assert(!ret);

	ret = listen(fd, 10);
//	assert(!ret);

    in_addr_t ip_addr;
    ip_addr = ntohl(server.sin_addr.s_addr);
	info("server %d.%d.%d.%d port %d created\n", 
        (ip_addr>>24)&0x000000ff,(ip_addr>>16)&0x000000ff,
        (ip_addr>>8)&0x000000ff,(ip_addr>>0)&0x000000ff,
        ntohs(server.sin_port));

    fd_set fds;
    int maxsock;    
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    
    tv.tv_sec = 180;
    tv.tv_usec = 0;
    
    maxsock = fd;
    
    ret = select(maxsock + 1, &fds, NULL, NULL, &tv);//wait
    //log_printf("select ret=%d\n", ret);
    if(ret > 0) 
    {
        if (FD_ISSET(fd, &fds))
        {
                connfd = accept(fd, (struct sockaddr *)&client, &socklen);
            //  assert(connfd > 0);
            
                ip_addr = ntohl(client.sin_addr.s_addr);
            
                info("client %d.%d.%d.%d port %d connected!\n",         
                    (ip_addr>>24)&0x000000ff,(ip_addr>>16)&0x000000ff,
                    (ip_addr>>8)&0x000000ff,(ip_addr>>0)&0x000000ff,
                    ntohs(client.sin_port));
            
            
                len = recvfrom(connfd, buf, buf_size, 0,
                        (struct sockaddr *)&client, &socklen);
            //  assert(len >= 0);
            
                buf[len] = 0;
                info("softap tcp server recv: %s\n", buf);
                
                uint8 macaddr[6];
                char mac[STR_MAC_LEN];
            
                if (wifi_get_macaddr(0, macaddr)) {
                    os_printf("macaddr=%02x:%02x:%02x:%02x:%02x:%02x\n", MAC2STR(macaddr));
                    snprintf(mac, sizeof(mac), MACSTR, MAC2STR(macaddr));
                }
            
                ret = get_ssid_and_passwd(buf);
                if (!ret) {
                    snprintf(msg, buf_size,
                        "{\"code\":1000, \"msg\":\"format ok\", \"model\":\"%s\", \"mac\":\"%s\"}",
                        DEV_MODEL, mac);
                } else
                    snprintf(msg, buf_size,
                        "{\"code\":2000, \"msg\":\"format error\", \"model\":\"%s\", \"mac\":\"%s\"}",
                        DEV_MODEL, mac);
            
                len = sendto(connfd, msg, strlen(msg), 0,
                        (struct sockaddr *)&client, socklen);
            //  assert(len >= 0);
                info("ack %s\n", msg);
            
                close(connfd);
                config_success = true;
        }
    }
    else if(ret == 0)
    {
        info("select timeout!!\n");    
        config_success = false;
    }
    else if(ret < 0) 
    {
        info("select error! ret=%d\n", ret);
        system_restart();        
    }

	close(fd);

	free(buf);
	free(msg);
   
	return config_success;
}

void alink_softap_stop(bool config_success)
{   
    if(config_success){
        wifi_set_opmode(STATION_MODE);

        setSmartConfigFlag(0);
        wifi_led_set_status(WIFI_LED_CONNECTING_AP);
        os_printf("ssid:%s,key:%s\n", alink_ssid,alink_passwd);
        struct station_config *config = (struct station_config *)zalloc(sizeof(struct station_config));
        strncpy(config->ssid, alink_ssid, 32);
        strncpy(config->password, alink_passwd, 64);
        wifi_station_set_config(config);
        free(config);
        wifi_station_connect();
    }
    else{
        wifi_set_opmode(STATION_MODE);        
        setSmartConfigFlag(SMARTCONFIG_OFF);
        wifi_led_set_status(WIFI_LED_OFF);

        struct station_config *config = (struct station_config *)zalloc(sizeof(struct station_config));
        strncpy(config->ssid, alink_ssid, 32);
        strncpy(config->password, alink_passwd, 64);
        wifi_station_set_config(config);
        free(config);
        
        vTaskDelay(100 / portTICK_RATE_MS);	 // 100 ms
        system_restart();
    }
}

void ICACHE_FLASH_ATTR alink_softap_task(void *pvParameters) 
{
	bool config_success;

    os_printf("alink_softap_task\n");   

    wifi_led_set_status(WIFI_LED_SMARTCONFIG);

	/* prepare and setup softap */
	alink_softap_setup();

	/* tcp server to get ssid & passwd */
	config_success = alink_softap_tcp_server();

	/* notification */
	alink_softap_stop(config_success);

	os_printf("heap_size %d\n", system_get_free_heap_size());
	stack_free_size();
	while (1) {
        int ret = wifi_station_get_connect_status();   // wait for sys wifi connected OK.
        if (ret == STATION_GOT_IP)
        {
            break;
        }
        vTaskDelay(100 / portTICK_RATE_MS);	 // 100 ms
	}
    
	need_notify_app = 1;
	vTaskDelete(NULL);

	return;
}



#if  1				// wifi
    //wifi_set_event_handler_cb(wifi_event_hand_function);
static void ICACHE_FLASH_ATTR wifi_event_hand_function(System_Event_t * event) 
{  
    u32_t addr;

//    os_printf("\n****wifi_event_hand_function %d******\n", event->event_id);
    switch (event->event_id) 
    {  
        case EVENT_STAMODE_CONNECTED:     
            os_printf("EVENT_STAMODE_CONNECTED ssid:%s,channel %d\n",
                event->event_info.connected.ssid,event->event_info.connected.channel);     
            break;
        case EVENT_STAMODE_DISCONNECTED:        
            os_printf("EVENT_STAMODE_DISCONNECTED ssid:%s,reason %d\n",
                event->event_info.disconnected.ssid,event->event_info.disconnected.reason);     
            wifi_led_set_status(WIFI_LED_CONNECTING_AP);
            /*work around: sometimes FSM can't goto reconnect state*/
            wifi_station_disconnect();
            wifi_station_connect();            
            break;  
        case EVENT_STAMODE_AUTHMODE_CHANGE:     
            break;  
        case EVENT_STAMODE_GOT_IP:     
            addr = ntohl(event->event_info.got_ip.ip.addr);
            os_printf("EVENT_STAMODE_GOT_IP %d.%d.%d.%d\n", 
                (addr>>24)&0x000000ff,
                (addr>>16)&0x000000ff,
                (addr>>8)&0x000000ff,
                (addr>>0)&0x000000ff);        
            wifi_led_set_status(WIFI_LED_CONNECTED_AP);
            break;  
        case EVENT_STAMODE_DHCP_TIMEOUT:    
            os_printf("EVENT_STAMODE_DHCP_TIMEOUT\n");     
            break;  
        default:        
            break;  
    }
}


 
#endif	

#ifdef ENABLE_GPIO_KEY
void ICACHE_FLASH_ATTR setLed1State(int flag) 
{
	if (0 == flag) {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);	// led 0ff
	} else {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);	// led on
	}
}

void ICACHE_FLASH_ATTR setLed2State(int flag) 
{
	if (0 == flag) {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0);	// led off
	} else {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);	// led on
	}
}


int need_factory_reset = 0;
static int sw1_long_key_flag = 0;
static int sw2_long_key_flag = 0;
void ICACHE_FLASH_ATTR user_key_long_press(void) 
{
	os_printf("[%s][%d] user key long press \n\r",__FUNCTION__,__LINE__);
	sw1_long_key_flag = 1;
	setLed1State(1);
	setLed2State(0);
	need_factory_reset = 1;
	os_printf("need_factory_reset =%d \n", need_factory_reset);

	return;
} 

void ICACHE_FLASH_ATTR user_key_short_press(void) 
{
	int ret = 0;
       if(sw1_long_key_flag) {
           sw1_long_key_flag = 0;
           return ;
       }
	os_printf("[%s][%d] user key press \n\r",__FUNCTION__,__LINE__);
	setLed1State(0);
	setLed2State(1);
	setSmartConfigFlag(0x1);   // short press enter smartconfig
	vTaskDelay(100);
	system_restart();  // restart then enter to smartconfig mode
} 

#define MD5_STR_LEN 32
#define MAX_URL_LEN 256
typedef struct
{
    char fwName[256];
    char fwVersion[256];
    unsigned int fwSize;
    char fwUrl[MAX_URL_LEN];
    char fwMd5[MD5_STR_LEN+1];
    int zip;
}FwFileInfo_t;
typedef xTaskHandle pthread_mutex_t;      /* identify a mutex */
typedef xTaskHandle pthread_t;            /* identify a thread */

typedef struct
{
    int status;
    pthread_mutex_t mutex;
    pthread_t id;
}FwOtaStatus_t;

extern FwOtaStatus_t fwOtaStatus;
extern FwFileInfo_t fwFileInfo;
extern void dumpFwInfo();
extern void alink_ota_main_thread(FwFileInfo_t * pFwFileInfo);
extern void alink_ota_init();
#define pthread_mutex_init(a, b)sys_mutex_new(a)

void ota_test()
{
    pthread_mutex_init(&fwOtaStatus.mutex,NULL);
    alink_ota_init();
    strcpy(fwFileInfo.fwMd5,"B3822931C345C88C9BA4AFB03C0C7295");
    fwFileInfo.fwSize = 388388;
    //strcpy(fwFileInfo.fwUrl,"http://otalink.alicdn.com/ALINKTEST_LIVING_LIGHT_SMARTLED_LUA/1.0.1/user1.2048.new.5.bin");
    strcpy(fwFileInfo.fwUrl,"http://otalink.alicdn.com/ALINKTEST_LIVING_LIGHT_SMARTLED/1.0.1/user1.2048.new.5.bin");
    strcpy(fwFileInfo.fwVersion,"1.0.1");
    fwFileInfo.zip = 0;
    dumpFwInfo();
    create_thread(&fwOtaStatus.id, "firmware upgrade pthread", (void *)alink_ota_main_thread, &fwFileInfo, 0xc00);

}
void ICACHE_FLASH_ATTR sw2_key_long_press() 
{
	os_printf("sw2_key_long_press\n");
    sw2_long_key_flag = 1;
    setLed1State(0);
    setLed2State(0);
}

void ICACHE_FLASH_ATTR sw2_key_short_press() 
{
	if (sw2_long_key_flag) {
		sw2_long_key_flag = 0;
		return ;
    }
    os_printf("sw2_key short \n");
	//ota_test();
}



int ICACHE_FLASH_ATTR readSmartConfigFlag() {
	int res = 0;
	uint32  value = 0;

	res = spi_flash_read(LFILE_START_ADDR+LFILE_SIZE, &value, 4);
	if (res != SPI_FLASH_RESULT_OK) {
		os_printf("read flash data error\n");
		return ALINK_ERR;
	}
    if(value == 0xFFFFFFFF){
        setSmartConfigFlag(1);
    	res = spi_flash_read(LFILE_START_ADDR+LFILE_SIZE, &value, 4);
    	if (res != SPI_FLASH_RESULT_OK) {
    		os_printf("read flash data error\n");
    		return ALINK_ERR;
    	}        
    }
	os_printf("read SmartConfig flag:0x%02x\n",value);    
	return (int)value;
}
// flash写接口需要先擦后写,这里擦了4KB,会导致整个4KB flash存储空间的浪费,可以将这个变量合并保存到用户数据中.
int ICACHE_FLASH_ATTR setSmartConfigFlag(int value) 
{
		int res = 0;	
		uint32 data = (uint32) value;
		spi_flash_erase_sector((LFILE_START_ADDR+LFILE_SIZE)/4096);
		res = spi_flash_write((LFILE_START_ADDR+LFILE_SIZE), &data, 4);
		if (res != SPI_FLASH_RESULT_OK) {
			os_printf("write data error\n");
			return ALINK_ERR;
		}
		os_printf("write flag(%d) success.", value);
		return ALINK_OK;
}


#endif
void ICACHE_FLASH_ATTR wificonnect_test_conn_ap(void) 
{
    signed char ret;
	ESP_DBG(("set a test conn router."));
    wifi_set_opmode(STATION_MODE);
    struct ip_info sta_ipconfig;
    struct station_config config;
    bzero(&config, sizeof(struct station_config));
    sprintf(config.ssid, "IOT_DEMO_TEST");
    sprintf(config.password, "123456789");
    wifi_station_set_config(&config);
    wifi_station_connect();
    wifi_get_ip_info(STATION_IF, &sta_ipconfig);
	return;
}

static void ICACHE_FLASH_ATTR sys_show_rst_info(void)
{
	struct rst_info *rtc_info = system_get_rst_info();	
	os_printf("reset reason: %x\n", rtc_info->reason);	
	if (rtc_info->reason == REASON_WDT_RST ||		
		rtc_info->reason == REASON_EXCEPTION_RST ||		
		rtc_info->reason == REASON_SOFT_WDT_RST) 
	{		
		if (rtc_info->reason == REASON_EXCEPTION_RST) 
		{			
			os_printf("Fatal exception (%d):\n", rtc_info->exccause);		
		}		
		os_printf("epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x\n",rtc_info->epc1, rtc_info->epc2, rtc_info->epc3, rtc_info->excvaddr, rtc_info->depc);	
	}

	return;
}
void ICACHE_FLASH_ATTR user_check_rst_info()
{
    struct rst_info *rtc_info = system_get_rst_info();

    os_printf("reset reason: %x\n", rtc_info->reason);

    if (rtc_info->reason == REASON_WDT_RST ||
        rtc_info->reason == REASON_EXCEPTION_RST ||
        rtc_info->reason == REASON_SOFT_WDT_RST) {
        if (rtc_info->reason == REASON_EXCEPTION_RST) {
            os_printf("Fatal exception (%d):\n", rtc_info->exccause);
        }
        os_printf("epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x\n",
                rtc_info->epc1, rtc_info->epc2, rtc_info->epc3, rtc_info->excvaddr, rtc_info->depc);
    }
    else{
      os_printf("epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x\n",
                rtc_info->epc1, rtc_info->epc2, rtc_info->epc3, rtc_info->excvaddr, rtc_info->depc);  
    }

}
void ICACHE_FLASH_ATTR user_demo(void) 
{
	unsigned int ret = 0;

    wifi_led_set_status(WIFI_LED_OFF);
    
#if USER_VIRTUAL_DEV_TEST
	uart_init_new();
    user_custom_init();
#endif    

	os_printf("SDK version:%s,alink version:%s\n", system_get_sdk_version(), CUS_GLOBAL_VER);
	os_printf("heap_size %d\n", system_get_free_heap_size());
    user_check_rst_info();
#ifdef ENABLE_GPIO_KEY   // demo for smartplug class products
//	init_key();
#endif	
	wifi_set_event_handler_cb(wifi_event_hand_function);

	wifi_set_opmode(STATION_MODE);
	os_printf("##[%s][%s|%d]Malloc %u.Available memory:%d.\n", __FILE__, __FUNCTION__, __LINE__, \
		sizeof(struct station_config), system_get_free_heap_size());
	ret = readSmartConfigFlag();	// -1 read flash fail!
	os_printf(" read flag:%d \n", ret);
    if(ret == SMARTCONFIG_SUCCESS){
        wifi_led_set_status(WIFI_LED_CONNECTING_AP);	
    }
	else if ((ret > SMARTCONFIG_SUCCESS)&&(ret <= SMARTCONFIG_FLAG)) {		
		xTaskCreate(smartconfig_task, "smartconfig_task", 256, NULL, 2, NULL);
		need_notify_app = 1;
	} 
    else if(ret == SOFTAP_FLAG)
    {
		xTaskCreate(alink_softap_task, "alink_softap_task", 256*2, NULL, 2, NULL);    
    }
    else if(ret >= SMARTCONFIG_OFF)
        wifi_led_set_status(WIFI_LED_OFF);	
    
	xTaskCreate(startdemo_task, "startdemo_task",(256*4), NULL, 2, NULL);
	
#if USER_PWM_LIGHT_EN
	user_esp_test_pwm();  // this a test ctrl pwm data, real device not need this
#endif
#if USER_UART_CTRL_DEV_EN
	user_uart_dev_start();  // create a task to handle uart data
#endif

	return;
}

LOCAL uint32 totallength = 0;
LOCAL uint32 sumlength = 0;
extern FwFileInfo_t fwFileInfo;

int upgrade_download(char *pusrdata, unsigned short length)
{
    char *ptr = NULL;
    char *ptmp2 = NULL;
    char lengthbuffer[32];
    bool ret = false;
    os_printf("%s %d totallength =%d length = %d \n",__FUNCTION__,__LINE__,totallength,length);
    if( (pusrdata == NULL )||length < 1) {
	 system_upgrade_flag_set(UPGRADE_FLAG_START);
	 sumlength = fwFileInfo.fwSize;
	 return -1;
    }
    if(sumlength < 1)
        return -1;
    if(totallength == 0){  // the first data packet
      
        ptr = pusrdata;
        if ((char)*(ptr) != 0xEA)
        {
            totallength = 0;
            sumlength = 0;
            return -1;
        }
        ret = ota_write_flash(pusrdata, length,true);
	 if(ret != false) {
            totallength += length;
	 }else {
            return -1;
	 }
    } else {
        totallength += length;
        if (totallength  > sumlength ){
            length = length - (totallength - sumlength);
            totallength = sumlength;
        }
        ret = ota_write_flash(pusrdata, length,true);
	 if (ret != true) {
	 	return -1; 
	 }
    }
    if (totallength == sumlength) {
        if( system_get_fw_start_sec() != 0 ) {
            if (-4 == upgrade_crc_check(system_get_fw_start_sec(),sumlength)) {
                totallength = 0;
                sumlength = 0;
	         return -1;
            } else {
                system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
                totallength = 0;
                sumlength = 0;
            }
        } else {
            totallength = 0;
            sumlength = 0;
        }    
    }
    return 0;
}
