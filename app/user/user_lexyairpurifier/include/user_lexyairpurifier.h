#ifndef _USER_LEXYROBOT101_H
#define _USER_LEXYROBOT101_H

#define MAX_UART_PKT_LEN   64
#define uint8_t uint8
#define uint32_t uint32

enum {
    RESERV_ON_TIMER = 0,
    RESERV_OFF_TIMER = 1,
};

enum {
/* GET */
    TYPE_GET_WORKING_STATUS = 0x00,
    TYPE_GET_WORKING_DATA = 0x01,
    
/* SET */
    TYPE_SET_POWER = 0x10,
    TYPE_SET_WORKING_MODE = 0x11,
    TYPE_SET_WIND_SPEED = 0x12,  
    TYPE_SET_TIMER = 0x13,
    TYPE_SET_RESERVATION = 0x14,
    TYPE_SET_ANION_SWITCH = 0x15,
    TYPE_SET_UV_SWITCH = 0x16,    
    TYPE_SET_SWING = 0x17,
    TYPE_SET_FILTER_RESET = 0x18,  
    TYPE_SET_CHILD_LOCK = 0x19,
    TYPE_SET_HUMIDIFY = 0x1A,      
    TYPE_SET_WIFI_LED = 0x1B,  
        
/* EVENT */
    TYPE_EVENT_ALARM = 0x20,
    TYPE_EVENT_POWER = 0x21,
    TYPE_EVENT_WORKING_STATUS = 0x22,
    TYPE_EVENT_TIMER = 0x24,
    TYPE_EVENT_ANION = 0x25,
    TYPE_EVENT_UV = 0x26,
    TYPE_EVENT_SWING = 0x27,
    TYPE_EVENT_HUMIDIFY = 0x28,
    TYPE_EVENT_WIFI_RESET = 0x29,
/* SLAVE MSG */
    TYPE_SLP_GET_STATUS = 0x30,
    TYPE_SLP_SET_STATUS = 0x31,        
    TYPE_WIFI_GET_STATUS = 0x32,        
    
};

/* get lexy air purifier information */
typedef struct{
    uart_msg_hdr_t hdr;
    uint8_t checksum;   
    uint8_t dataEnd;
}msg_get_air_purifier_status_t;

typedef struct{
    uart_msg_hdr_t hdr;
    uint8_t modelNum;
    uint8_t power;
    uint8_t workingMode;    
    uint8_t windSpeed;    
    uint8_t timer;    
    uint8_t reservation;    
    uint8_t anion;    
    uint8_t uv;    
    uint8_t humidify;    
    uint8_t swing;    
    uint8_t err;    
    uint8_t checksum;        
    uint8_t dataEnd;    
}msg_get_air_purifier_status_rsp_t;

typedef struct{
    uart_msg_hdr_t hdr;
    uint8_t checksum;   
    uint8_t dataEnd;
}msg_get_air_purifier_data_t;

typedef struct{
    uart_msg_hdr_t hdr;
    uint8_t pm25[2];
    uint8_t hcho[2];
    uint8_t temperature;    
    uint8_t humidity;   
    uint8_t smell;       
    uint8_t filter1time[2];
    uint8_t filter2time[2];
    uint8_t filter3time[2];    
    uint8_t filterReplace;       
    uint8_t checksum;    
    uint8_t dataEnd;    
}msg_get_air_purifier_data_rsp_t;


/* set power  */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t power;
    uint8_t checksum;    
    uint8_t dataEnd;       
}msg_set_power_t;


/* set working mode  */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t workingMode;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_set_working_mode_t;


/* set speed  */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t speed;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_set_speed_t;


/* set timer  */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t timer;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_set_timer_t;


/* set reservation  */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t reservation;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_set_reservation_t;


/* set anion  */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t anion;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_set_anion_t;


/* set uv  */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t uv;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_set_uv_t;


/* set swing  */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t swing;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_set_swing_t;


/* set filter reset  */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t reset;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_set_filter_reset_t;


/* set child lock  */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t childLock;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_set_child_lock_t;


/* set  humidify  */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t humidify;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_set_humidify_t;


/* set wifi LED  */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t wifiLED;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_set_wifi_led_t;


/* slp mode  */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t enable;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_get_wifi_slp_mode_rsp_t;

typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t enable;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_set_wifi_slp_mode_t;

typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t wifiStatus;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_get_wifi_status_rsp_t;


typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t ret;
    uint8_t checksum;
    uint8_t dataEnd;
}msg_set_common_rsp_t;

/* event */
typedef struct {
    uart_msg_hdr_t hdr;
    uint8_t data[0];
}msg_event_t;


#define LEXY_AIR_PURIFIER_PM25                    "Device.Lexy.AirPurifier.PM25"
#define LEXY_AIR_PURIFIER_HCHO                    "Device.Lexy.AirPurifier.HCHO"

#define LEXY_AIR_PURIFIER_TEMPERATURE             "Device.Lexy.AirPurifier.Temperature"
#define LEXY_AIR_PURIFIER_HUMIDITY                "Device.Lexy.AirPurifier.Humidity"
#define LEXY_AIR_PURIFIER_SMELL                   "Device.Lexy.AirPurifier.Smell"
#define LEXY_AIR_PURIFIER_FILTER1_USEDTIME        "Device.Lexy.AirPurifier.Filter.1.UsedTime"
#define LEXY_AIR_PURIFIER_FILTER2_USEDTIME        "Device.Lexy.AirPurifier.Filter.2.UsedTime"
#define LEXY_AIR_PURIFIER_FILTER3_USEDTIME        "Device.Lexy.AirPurifier.Filter.3.UsedTime"
#define LEXY_AIR_PURIFIER_FILTER1_REPLACE         "Device.Lexy.AirPurifier.Filter.1.Replace"
#define FV_LEXY_AIR_PURIFIER_FILTER2_REPLACE         "Device.Lexy.AirPurifier.Filter.2.Replace"
#define LEXY_AIR_PURIFIER_FILTER3_REPLACE         "Device.Lexy.AirPurifier.Filter.3.Replace"

#define LEXY_AIR_PURIFIER_PRODUCT_MODEL           "Device.Lexy.AirPurifier.ProductModel"
#define LEXY_AIR_PURIFIER_POWER                   "OnOff_Power"
#define LEXY_AIR_PURIFIER_WORKING_MODE            "Device.Lexy.AirPurifier.WorkingMode"
#define LEXY_AIR_PURIFIER_WIND_SPEED              "Device.Lexy.AirPurifier.WindSpeed"
#define LEXY_AIR_PURIFIER_WORKING_TIMER           "Device.Lexy.AirPurifier.WorkingTimer"
#define LEXY_AIR_PURIFIER_RESERVATION_ON          "Device.Lexy.AirPurifier.ReservationTime"
#define LEXY_AIR_PURIFIER_RESERVATION_OFF         "Device.Lexy.AirPurifier.ReservationOffTime"

#define LEXY_AIR_PURIFIER_ANION                   "OnOff_Ions"
#define LEXY_AIR_PURIFIER_UV_SWITCH               "Device.Lexy.AirPurifier.UV"
#define LEXY_AIR_PURIFIER_HUMIDIFY_SWITCH         "Device.Lexy.AirPurifier.Humidify"
#define LEXY_AIR_PURIFIER_SWING_SWITCH            "Device.Lexy.AirPurifier.Swing"
#define LEXY_AIR_PURIFIER_FILTER_RESET            "Device.Lexy.AirPurifier.FilterReset"
#define LEXY_AIR_PURIFIER_CHILDREN_LOCK           "Device.Lexy.AirPurifier.ChildrenLock"

#define LEXY_AIR_PURIFIER_ERRCODE                 "Device.Lexy.AirPurifier.ErrCode"

typedef struct{
    uint8_t modelNum;
    uint8_t power;
    uint8_t workingMode;    
    uint8_t windSpeed;    
    uint8_t timer;    
    uint8_t reservation;    
    uint8_t anion;    
    uint8_t uv;    
    uint8_t humidify;    
    uint8_t swing;    
    uint8_t err;       
    uint32_t lastGetTime;      
}uart_get_AirPurifier_status_t;

typedef struct{
    uint16_t pm25;
    uint16_t hcho;
    uint8_t temperature;    
    uint8_t humidity;   
    uint16_t filter1time;
    uint16_t filter2time;
    uint16_t filter3time;    
    uint8_t filterReplace;       
    uint32_t lastGetTime;      
}uart_get_AirPurifier_data_t;

/* CUSTOM INFO */
/* device info */
#define DEV_NAME "AIRPURIFIER"
#define DEV_CATEGORY "LIVING"
#define DEV_TYPE "KJ801P"
#define DEV_BRAND "LEXY"
#define DEV_MANUFACTURE "LEXY"

#define DEV_MODEL "SEAING_LIVING_AIRPURIFIER_KJ801"

#define ALINK_KEY "gWwRIyMVpQzCbStHIrmE"
#define ALINK_SECRET "Nb3vM8ACmS2vSKp8i3I6wK82XtyPPy04lkOfjKm2"
#define ALINK_TPSK "3Uy72LlTBfZY1bJnoWVqRg3j2aFvycCmzUykDIWDEk0="

/*sandbox key/secret*/
#define ALINK_KEY_SANDBOX "2tcHGVrhB96xMrUzN5oc"
#define ALINK_SECRET_SANDBOX "M44yDCSQQvZ7v98Fuismj7t1Cf0zRTPlAZm8k0Zb"
#define ALINK_TPSK_SANDBOX "/GnwTbaVLZsJWdPLcRIMCmN+g639O6DkxY2Rj07+5yc="


/*设备硬件信息:系统上电后读取的硬件/固件信息,此处为演示需要,直接定义为宏.产品对接时,需要调用自身接口获取*/

#define DEV_SN "1234567890"
#define DEV_VERSION "1.0.0"

#endif  /* _USER_LEXYAIRPURIFIER_H */
