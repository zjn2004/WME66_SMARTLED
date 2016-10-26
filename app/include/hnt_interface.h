/*
 * Copyright (c) 2014 Seaing, Ltd.
 * All Rights Reserved. 
 * Seaing Confidential and Proprietary.
 */
#ifndef __HNT_INTERFACE_H__
#define __HNT_INTERFACE_H__

#define log_printf printf

enum { 
    Success = 0,
    Method_Not_Supported = 9000,
    Request_Denied = 9001,
    Internal_Error = 9002,
    Invalid_Arguments = 9003,    
    Resources_Exceeded = 9004,
    Invalid_Parameter_Name = 9005,
    Invalid_Parameter_Type = 9006,
    Invalid_Parameter_Value = 9007,    
    Upgrade_Failure = 9800,
    Downloading_Firmware = 9801,
    Upgrading_Firmware = 9802,
};


typedef struct deviceParameter_s {
    char * paramName;
    int (*getParameterFunc)(char *, char *);    
    int (*setParameterFunc)(char *, char *);
}deviceParameter_t;

typedef struct {
    deviceParameter_t *table;
    int tableSize;
}deviceParameterTable_t;

typedef struct customInfo_s {
        /* optional */
        char sn[STR_SN_LEN];
        char name[STR_NAME_LEN];
        char brand[STR_NAME_LEN];
        char type[STR_NAME_LEN];
        char category[STR_NAME_LEN];
        char manufacturer[STR_NAME_LEN];
        char version[STR_NAME_LEN];
    
        /* manufacturer_category_type_name */
        char model[STR_MODEL_LEN];
    
        /* mandatory for gateway, optional for child dev */
        char key[STR_KEY_LEN];
        char secret[STR_SEC_LEN];
        
        char key_sandbox[STR_KEY_LEN];
        char secret_sandbox[STR_SEC_LEN];
}customInfo_t;

void ICACHE_FLASH_ATTR
hnt_param_array_regist(deviceParameter_t *custom_param_array, uint32 array_size);

void ICACHE_FLASH_ATTR
hnt_custom_info_regist(customInfo_t *customInfo);

void ICACHE_FLASH_ATTR
hnt_device_status_change(void);

char* vendor_get_model(void);
#endif
