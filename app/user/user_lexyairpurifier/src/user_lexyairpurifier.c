#include "esp_common.h"
#include "alink_export.h"

#include "user_uart.h"
#include "hnt_interface.h"
#include "user_lexyairpurifier.h"
#include "user_config.h"
#include "ledctl.h"
#include "hnt_config.h"

#if USER_LEXY_AIR_PURIFIER
static uart_get_AirPurifier_status_t gLexyAirPurifierStatus;

LOCAL void ICACHE_FLASH_ATTR
system_factory_reset(void)
{
    os_printf("[%s][%d]\n\r",__FUNCTION__,__LINE__);

    setSmartConfigFlag(1);   // set smartconfig
    vTaskDelay(100);

    system_restart();
}

static int eGetWifiStatusRsp(u8 msg_seq)
{
    int ret;
    msg_get_wifi_status_rsp_t msg;

    memset(&msg, 0, sizeof(msg));
    msg.hdr.op = OP_SLAVE_ACK;
    msg.hdr.seq = msg_seq;
    msg.hdr.type = TYPE_WIFI_GET_STATUS;
    msg.hdr.len = DATA_LEN(msg);
    msg.wifiStatus = 0x00;
    
    ret = uart_tx_without_rsp((char*)&msg, 
                      sizeof(msg));
    
    if(ret != OP_OK) return -1;
    
    return 0;
}

static void uart_event_process(u8 *buf, u32 len)
{
    u8 checksum = 0;
    int i;
    
    uart_msg_hdr_t *hdr = (uart_msg_hdr_t *)buf;

    if(hdr->op == OP_SLAVE)
    {
        printf("OP_SLAVE.\n");
        if(hdr->type == TYPE_WIFI_GET_STATUS)
        {
            eGetWifiStatusRsp(hdr->seq);
        }        
        return;
    }
    else if(hdr->op != OP_EVT)
        return;

    msg_event_t *eventMsg = (msg_event_t *)buf;

    for (i = 2; i < len - 2; i++) {
        checksum += buf[i];
    }
    
    if (buf[i] != checksum) {
        printf("recv checksum %02x,expect checksum:%02x error\n",
            buf[i],checksum);
        return;
    }
    
    switch (eventMsg->hdr.type){
#if 0    
        case TYPE_EVENT_ALARM:            
            sprintf(para_value, "0x%02x", eventMsg->data[0]);
            hnt_xmpp_notif_config_changed(CWMP_FV_LEXY_AIR_PURIFIER_ERRCODE, para_value);
            hnt_xmpp_notif_update_data(CWMP_FV_LEXY_AIR_PURIFIER_ERRCODE, para_value);            
            break;  
#endif            
        case TYPE_EVENT_POWER:   
            gLexyAirPurifierStatus.power = eventMsg->data[0];             
            gLexyAirPurifierStatus.lastGetTime = system_get_time();            
            hnt_device_status_change();            
            break;
#if 0            
        case TYPE_EVENT_WORKING_STATUS:     
            if(eventMsg->data[0] == 0x00)
                sprintf(para_value, "%s", "default");
            else if(eventMsg->data[0] == 0x01)
                sprintf(para_value, "%s", "auto");
            else if(eventMsg->data[0] == 0x02)
                sprintf(para_value, "%s", "quiet");
            else if(eventMsg->data[0] == 0x03)
                sprintf(para_value, "%s", "super");   
            else if(eventMsg->data[0] == 0x04)
                sprintf(para_value, "%s", "manual");             
            hnt_xmpp_notif_config_changed(CWMP_FV_LEXY_AIR_PURIFIER_WORKING_MODE, para_value);
            
            if(eventMsg->data[1] == 0x00)
                sprintf(para_value, "%s", "default");
            else if(eventMsg->data[1] == 0x01)
                sprintf(para_value, "%s", "lowest");
            else if(eventMsg->data[1] == 0x02)
                sprintf(para_value, "%s", "low");
            else if(eventMsg->data[1] == 0x03)
                sprintf(para_value, "%s", "medium");   
            else if(eventMsg->data[1] == 0x04)
                sprintf(para_value, "%s", "high");       
            else if(eventMsg->data[1] == 0x05)
                sprintf(para_value, "%s", "highest");          
            hnt_xmpp_notif_config_changed(CWMP_FV_LEXY_AIR_PURIFIER_WIND_SPEED, para_value);
            break;       
        case TYPE_EVENT_TIMER:            
            sprintf(para_value, "%d", eventMsg->data[0]);
            hnt_xmpp_notif_config_changed(CWMP_FV_LEXY_AIR_PURIFIER_WORKING_TIMER, para_value);
            break;
#endif            
        case TYPE_EVENT_ANION:            
            gLexyAirPurifierStatus.anion = eventMsg->data[0];             
            gLexyAirPurifierStatus.lastGetTime = system_get_time();            
            hnt_device_status_change();            
            break;  
#if 0            
        case TYPE_EVENT_UV:            
            sprintf(para_value, "%s", eventMsg->data[0]?"on":"off");
            hnt_xmpp_notif_config_changed(CWMP_FV_LEXY_AIR_PURIFIER_UV_SWITCH, para_value);
            break;  
        case TYPE_EVENT_SWING:            
            sprintf(para_value, "%s", eventMsg->data[0]?"on":"off");
            hnt_xmpp_notif_config_changed(CWMP_FV_LEXY_AIR_PURIFIER_SWING_SWITCH, para_value);
            break;  
        case TYPE_EVENT_HUMIDIFY:            
            sprintf(para_value, "%s", eventMsg->data[0]?"on":"off");
            hnt_xmpp_notif_config_changed(CWMP_FV_LEXY_AIR_PURIFIER_HUMIDIFY_SWITCH, para_value);
            break; 
#endif            
        case TYPE_EVENT_WIFI_RESET:
            printf("wifi reset,data:%02x!\n", eventMsg->data[0]);
            system_factory_reset();
            break;          
        default:
            break;            
    }

    return;    
}

static int lexy_air_purifier_wifi_status_led(uint8_t led_setting)
{
    printf("lexy_air_purifier_wifi_status_led %d\n",led_setting);

    int ret;
    msg_set_wifi_led_t msg;
    msg_set_common_rsp_t rspMsg, rspMsgCheck;   

    memset(&msg, 0, sizeof(msg));
    memset(&rspMsg, 0, sizeof(rspMsg));    
    msg.hdr.op = OP_SET;
    msg.hdr.type = TYPE_SET_WIFI_LED;
    msg.hdr.len = DATA_LEN(msg);
    
    if(led_setting == WIFI_LED_OFF)
        msg.wifiLED = 0;//OFF
    else if(led_setting == WIFI_LED_SMARTCONFIG)
        msg.wifiLED = 3;//FAST BLINK
    else if(led_setting == WIFI_LED_CONNECTING_AP)
        msg.wifiLED = 2;//SLOW BLINK
    else if(led_setting == WIFI_LED_CONNECTED_AP)
        msg.wifiLED = 1;//ON
    else if(led_setting == WIFI_LED_CONNECTED_SERVER)
        msg.wifiLED = 1;//ON

    ret = uart_tx_with_rsp((char*)&msg, 
                      sizeof(msg),
                      (char*)&rspMsg,
                      sizeof(rspMsg));
    
    if(ret != OP_OK) return -1;
    
    memset(&rspMsgCheck, 0, sizeof(rspMsgCheck));
    rspMsgCheck.hdr.syn[0] = SYN0;        
    rspMsgCheck.hdr.syn[1] = SYN1;
    rspMsgCheck.hdr.op = msg.hdr.op + 1;
    rspMsgCheck.hdr.seq = msg.hdr.seq;
    rspMsgCheck.hdr.type = msg.hdr.type;
    rspMsgCheck.hdr.len = DATA_LEN(rspMsgCheck);
    if (memcmp(&rspMsgCheck.hdr, &rspMsg.hdr, sizeof(uart_msg_hdr_t)) != 0) 
    {
        printf("msg check error\n");
        return -1;
    }    
    
    if(ret != OP_OK) return -1;
    
    return 0;
}

static int eLexyGetAirPurifierStatus(void)
{    
    int ret;
    msg_get_air_purifier_status_t msg;
    msg_get_air_purifier_status_rsp_t rspMsg, rspMsgCheck;   
    
    memset(&msg, 0, sizeof(msg));
    memset(&rspMsg, 0, sizeof(rspMsg));
    msg.hdr.op = OP_GET;
    msg.hdr.type = TYPE_GET_WORKING_STATUS;
    msg.hdr.len = DATA_LEN(msg);
    
    ret = uart_tx_with_rsp((char*)&msg, 
                      sizeof(msg),
                      (char*)&rspMsg,
                      sizeof(rspMsg));

    if(ret != OP_OK) return -1;
    
    memset(&rspMsgCheck, 0, sizeof(rspMsgCheck));
    rspMsgCheck.hdr.syn[0] = SYN0;
    rspMsgCheck.hdr.syn[1] = SYN1;    
    rspMsgCheck.hdr.op = msg.hdr.op + 1;
    rspMsgCheck.hdr.seq = msg.hdr.seq;
    rspMsgCheck.hdr.type = msg.hdr.type;
    rspMsgCheck.hdr.len = DATA_LEN(rspMsgCheck);
    if (memcmp(&rspMsgCheck.hdr, &rspMsg.hdr, sizeof(uart_msg_hdr_t)) != 0) {
        printf("msg check error\n");
        return -1;
    } 

    printf("modelNum      = %d\n", rspMsg.modelNum);
    printf("power         = %d\n", rspMsg.power);
    printf("workingMode   = %d\n", rspMsg.workingMode);
    printf("windSpeed     = %d\n", rspMsg.windSpeed);
    printf("timer         = %d\n", rspMsg.timer);
    printf("reservation   = %d\n", rspMsg.reservation);
    printf("anion         = %d\n", rspMsg.anion);
    printf("uv            = %d\n", rspMsg.uv);
    printf("humidify      = %d\n", rspMsg.humidify);
    printf("swing         = %d\n", rspMsg.swing);
    printf("err           = %d\n", rspMsg.err);


    gLexyAirPurifierStatus.modelNum = rspMsg.modelNum;            
    gLexyAirPurifierStatus.power = rspMsg.power;            
    gLexyAirPurifierStatus.workingMode = rspMsg.workingMode;     
    gLexyAirPurifierStatus.windSpeed = rspMsg.windSpeed;            
    gLexyAirPurifierStatus.timer = rspMsg.timer;            
    gLexyAirPurifierStatus.reservation = rspMsg.reservation;  
    gLexyAirPurifierStatus.anion = rspMsg.anion;            
    gLexyAirPurifierStatus.uv = rspMsg.uv;            
    gLexyAirPurifierStatus.humidify = rspMsg.humidify;  
    gLexyAirPurifierStatus.swing = rspMsg.swing;            
    gLexyAirPurifierStatus.err = rspMsg.err;      
    
    gLexyAirPurifierStatus.lastGetTime = system_get_time();
    
    return 0;
}


static int eGetLexyAirPurifierWorkingStatus(char *paramName, char *value)
{    
    int ret;
    
    printf("paramName:%s\n", paramName);

    if((gLexyAirPurifierStatus.lastGetTime == 0)
        ||(system_get_time()-gLexyAirPurifierStatus.lastGetTime)>=(1*1000*1000))
    {       
        eLexyGetAirPurifierStatus();
    }
    
    if(strcmp(paramName,LEXY_AIR_PURIFIER_POWER) == 0)
    {
        sprintf(value, "%d", gLexyAirPurifierStatus.power);
    }
    else if(strcmp(paramName,LEXY_AIR_PURIFIER_ANION) == 0)
    {
        sprintf(value, "%d", gLexyAirPurifierStatus.anion);
    }


    return 0;
}

static int eSetLexyAirPurifierPower(char *paramName, char *value)
{
    int ret;
    msg_set_power_t msg;
    msg_set_common_rsp_t rspMsg, rspMsgCheck;   
    uint8_t powerStatus ;

    if (strcmp(value, "on") == 0 || strcmp(value, "ON") == 0 || strcmp(value, "1") == 0)
        powerStatus = 0x01;
    else if (strcmp(value, "off") == 0 || strcmp(value, "OFF") == 0 || strcmp(value, "0") == 0)
        powerStatus = 0x00;
    else
    {
        printf("invalid input value[%s]\n",value);
        return -1;
    }

    memset(&msg, 0, sizeof(msg));
    memset(&rspMsg, 0, sizeof(rspMsg));
    msg.hdr.op = OP_SET;
    msg.hdr.type = TYPE_SET_POWER;
    msg.hdr.len = DATA_LEN(msg);
    msg.power = powerStatus;
    
    ret = uart_tx_with_rsp((char*)&msg, 
                      sizeof(msg),
                      (char*)&rspMsg,
                      sizeof(rspMsg));
    
    if(ret != OP_OK) return -1;
    
    memset(&rspMsgCheck, 0, sizeof(rspMsgCheck));
    rspMsgCheck.hdr.syn[0] = SYN0;        
    rspMsgCheck.hdr.syn[1] = SYN1;
    rspMsgCheck.hdr.op = msg.hdr.op + 1;
    rspMsgCheck.hdr.seq = msg.hdr.seq;
    rspMsgCheck.hdr.type = msg.hdr.type;
    rspMsgCheck.hdr.len = DATA_LEN(rspMsgCheck);
    if (memcmp(&rspMsgCheck.hdr, &rspMsg.hdr, sizeof(uart_msg_hdr_t)) != 0 || rspMsg.ret != OP_OK) {
        printf("msg check error\n");
        return -1;
    }
    gLexyAirPurifierStatus.power = powerStatus;             
    gLexyAirPurifierStatus.lastGetTime = system_get_time();  
    
    return 0;
}

static int eSetLexyAirPurifierAnion(char *paramName, char *value)
{
    int ret;
    msg_set_anion_t msg;
    msg_set_common_rsp_t rspMsg, rspMsgCheck;   
    uint8_t anion ;

    if (strcmp(value, "on") == 0 || strcmp(value, "ON") == 0 || strcmp(value, "1") == 0)
        anion = 0x01;
    else if (strcmp(value, "off") == 0 || strcmp(value, "OFF") == 0 || strcmp(value, "0") == 0)
        anion = 0x00;
    else
    {
        printf("invalid input value[%s]\n",value);
        return -1;
    }

    memset(&msg, 0, sizeof(msg));
    memset(&rspMsg, 0, sizeof(rspMsg));
    msg.hdr.op = OP_SET;
    msg.hdr.type = TYPE_SET_ANION_SWITCH;
    msg.hdr.len = DATA_LEN(msg);
    msg.anion = anion;
    
    ret = uart_tx_with_rsp((char*)&msg, 
                      sizeof(msg),
                      (char*)&rspMsg,
                      sizeof(rspMsg));
    
    if(ret != OP_OK) return -1;
    
    memset(&rspMsgCheck, 0, sizeof(rspMsgCheck));
    rspMsgCheck.hdr.syn[0] = SYN0;        
    rspMsgCheck.hdr.syn[1] = SYN1;
    rspMsgCheck.hdr.op = msg.hdr.op + 1;
    rspMsgCheck.hdr.seq = msg.hdr.seq;
    rspMsgCheck.hdr.type = msg.hdr.type;
    rspMsgCheck.hdr.len = DATA_LEN(rspMsgCheck);
    if (memcmp(&rspMsgCheck.hdr, &rspMsg.hdr, sizeof(uart_msg_hdr_t)) != 0 || rspMsg.ret != OP_OK) {
        printf("msg check error\n");
        return -1;
    }
    
    gLexyAirPurifierStatus.anion = anion;             
    gLexyAirPurifierStatus.lastGetTime = system_get_time();      
    return 0;
}


deviceParameter_t DeviceParamList[] = {
{LEXY_AIR_PURIFIER_POWER, eGetLexyAirPurifierWorkingStatus, eSetLexyAirPurifierPower},
{LEXY_AIR_PURIFIER_ANION, eGetLexyAirPurifierWorkingStatus, eSetLexyAirPurifierAnion}
};

customInfo_t customInfo;
char alink_tpsk[64+1]={0};

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
    
	strcpy(alink_tpsk, ALINK_TPSK);
    hnt_vendor_tpsk_regist(alink_tpsk);    
}

void
user_custom_init(void)
{       
	os_printf("%s,%d\n", __FUNCTION__,__LINE__);
    
    hnt_custom_info_init();
    
    hnt_uart_event_func_regist(uart_event_process);

    hnt_param_array_regist(&DeviceParamList[0], sizeof(DeviceParamList)/sizeof(DeviceParamList[0]));

/*wifi led install */
    hnt_wifi_led_func_regist(lexy_air_purifier_wifi_status_led);
}
#endif
