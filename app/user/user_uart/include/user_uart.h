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

#ifndef __USER_UART_H__
#define __USER_UART_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "../../../include/user_config.h"


#define CUS_UART0_QUEUE_LENGTH (10)
#define CUS_UART_TX_MAX     (128)  // uart tx buf max len

#if USER_UART_CTRL_DEV_EN

extern xQueueHandle xQueueCusUart;

typedef struct
{
	int rx_len;
	char rx_buf[CUS_UART_TX_MAX];
}CusUartIntrPtr;

void debug_print_hex_data(char*buf, int len);
void ICACHE_FLASH_ATTR user_uart_task(void *pvParameters);
void ICACHE_FLASH_ATTR user_uart_dev_start(void);

#define DATA_LEN(msg) (sizeof(msg) - sizeof(msg.hdr) - sizeof(msg.checksum)- sizeof(msg.dataEnd))

#define MSG_RESEND_TIMES 2

#define UART_RX_BUF_SIZE    64
typedef struct uart_rx_s {
    u8 rx_buf[UART_RX_BUF_SIZE];
    u8 rx_len;
}uartRx_t;

enum {
    OP_OK = 0x00,
    OP_FAIL,
};

enum {
    SYN0 = 0xA5,
    SYN1 = 0x5A,
};

enum {
    OP_GET = 0x00,
    OP_GET_ACK,
    OP_SET,
    OP_SET_ACK,
    OP_EVT,
    OP_EVT_ACK,
    OP_SLAVE,
    OP_SLAVE_ACK,    
    OP_ERR = 0Xff,    
};

/* general message header */
typedef struct {
    u8 syn[2];
    u8 op;
    u8 seq;
    u8 type;
    u8 len;
}uart_msg_hdr_t;

typedef void (*hnt_uart_event_handler_t)(char *buf, u16 len);
void hnt_uart_event_func_regist(void *func);

int uart_tx_with_rsp(char* msg,int msgLen,char* rspMsg,int rspMsgLen);
int uart_tx_without_rsp(char* msg,int msgLen);

#endif

#endif

