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

#include "c_types.h"
#include <stdio.h>
#include <string.h>
#include "esp_common.h"
#include "user_uart.h"

#if USER_UART_CTRL_DEV_EN

#include "../../../include/driver/uart.h"


xQueueHandle xQueueCusUart;
xQueueHandle uart_rsp_q;

static u8 uart_msg_seq = 0;
#define UART_SEQ (uart_msg_seq)

hnt_uart_event_handler_t uart_event_handle = NULL;

void hnt_uart_event_handle(char *buf, u16 len)
{    
    if(uart_event_handle)
    {        
        uart_event_handle(buf, len);
    }
}

void hnt_uart_event_func_regist(void *func)
{
    uart_event_handle = (hnt_uart_event_handler_t)func;
}

static void hnt_uart_rx_process(CusUartIntrPtr uartptrData)
{ 
    uart_msg_hdr_t *hdr = (uart_msg_hdr_t *)uartptrData.rx_buf;

#if 1
    printf("recv %d:\n",uartptrData.rx_len);
    u_int j = 0;

    for (j = 0; j < uartptrData.rx_len; j++) 
    {
        printf("%02x ", uartptrData.rx_buf[j]);
    }
    printf("\n");
#endif

    if((hdr->op == OP_GET_ACK)||(hdr->op == OP_SET_ACK)||(hdr->op == OP_ERR))
    {
        if(hdr->seq == uart_msg_seq)
        {
			xQueueSend(uart_rsp_q, (void *)&uartptrData, 0);            
        }
        else 
            printf("seq %02x not correct, should be %02x.\n", hdr->seq,uart_msg_seq);
       
        return;
    }
    else if((hdr->op != OP_EVT)&&(hdr->op != OP_SLAVE))
    {
        printf("op %d err.\n", hdr->op);    
        return;
    }

    hnt_uart_event_handle(uartptrData.rx_buf,uartptrData.rx_len);
    
    return;
}

void ICACHE_FLASH_ATTR 
hnt_at_cmdProcess(char *dat_in, int in_len)
{
    int i;
    for(i = 2; i < in_len; i++)
    {
        if(dat_in[i] == '\n')
        {
            dat_in[i+1] = '\0';
            break;
        }
    }
    
    printf("\n%s\n",dat_in);
    if((memcmp(&dat_in[2], "+Z", 2) == 0) 
        ||(memcmp(&dat_in[2], "+z", 2) == 0))
        system_restart();
}

void debug_print_hex_data(char*buf, int len)
{
	int i = 0;
	printf("\n_____________[%d]__________\n",len);
	for(i=0;i<len;i++)
	{
		printf("%02X ",*(buf+i));
	}
	printf("\n____________________________\n");
	return;
}	

int uart0_write_data(u8 *data, int len)
{
	int re_len = 0;
	int i = 0;
	for(i  = 0; i <len; i++)
	{
		uart0_write_char(*(data+i));
	}
	return i;
}

static u8 ICACHE_FLASH_ATTR cus_uart_data_handle(char *dat_in, int in_len, char *dat_out)
{
	ESP_DBG(("uart data handler.."));
	debug_print_hex_data(dat_in, in_len);
	return 0x00;
}

void ICACHE_FLASH_ATTR user_uart_task(void *pvParameters)
{
    CusUartIntrPtr uartptrData;
	
    while(1)
    {
    	if (xQueueReceive(xQueueCusUart, (void *)&uartptrData, (portTickType)100/*portMAX_DELAY*/)) // wait about 5sec 
    	{           
            if((memcmp(uartptrData.rx_buf, "AT", 2) == 0) 
                ||(memcmp(uartptrData.rx_buf, "at", 2) == 0))
            {
                hnt_at_cmdProcess(uartptrData.rx_buf,uartptrData.rx_len);               
            }
            else if((uartptrData.rx_buf[0] == SYN0)
                &&(uartptrData.rx_buf[1] == SYN1))
            {
//                ESP_DBG(("recv uart data,len %d",uartptrData.rx_len));
//                debug_print_hex_data(uartptrData.rx_buf,uartptrData.rx_len);
                hnt_uart_rx_process(uartptrData);
            }
            else
            {
                ESP_DBG(("recv err uart data,len %d",uartptrData.rx_len));
                debug_print_hex_data(uartptrData.rx_buf,uartptrData.rx_len);            
            }
    	}
    }

    vTaskDelete(NULL);
	
}

void ICACHE_FLASH_ATTR user_uart_dev_start(void)
{
    uart_init_new();   // cfg uart0 connection device MCU, cfg uart1 TX debug output
    xQueueCusUart = xQueueCreate((unsigned portBASE_TYPE)CUS_UART0_QUEUE_LENGTH, sizeof(CusUartIntrPtr));
    uart_rsp_q = xQueueCreate((unsigned portBASE_TYPE)4, sizeof(CusUartIntrPtr));

    xTaskCreate(user_uart_task, (uint8 const *)"uart", 256, NULL, tskIDLE_PRIORITY + 1, NULL);

	return;
}

int uart_tx_without_rsp(
    char* msg,
    int msgLen)
{ 
    int i = 0;
    unsigned char checksum = 0;

    msg[0]=0xA5;
    msg[1]=0x5A;

    for (i = 2; i < msgLen - 2; i++) {
        checksum += msg[i];
    }
    msg[msgLen - 2] = checksum;
    msg[msgLen - 1] = 0xAA;

#if 1
    printf("send:\n");
    u_int j = 0;

    for (j = 0; j < msgLen; j++) {
        printf("%02x ", msg[j]);
    }
    printf("\n");
#endif
    
    uart0_write_data(msg, msgLen);
    return 0;
}


int uart_tx_with_rsp(
    char* msg,
    int msgLen,
    char* rspMsg,
    int rspMsgLen)
{ 
    int i = 0;
    unsigned char checksum = 0;
    unsigned char send_times = 0;        
    int ret = OP_OK;
    CusUartIntrPtr uartptrData;
        
    msg[0] = 0xA5;
    msg[1] = 0x5A;
    
    msg[3] = UART_SEQ;

    for (i = 2; i < msgLen - 2; i++) {
        checksum += msg[i];
    }
    msg[msgLen - 2] = checksum;
    msg[msgLen - 1] = 0xAA;

    memset(&uartptrData,0,sizeof(CusUartIntrPtr));
    
    do
    {
        printf("send:\n");
        u_int j = 0;
        for (j = 0; j < msgLen; j++) {
            printf("%02x ", msg[j]);
        }
        printf("\n");
        
        uart0_write_data(msg, msgLen);
        send_times++;
        xQueueReceive(uart_rsp_q, (void *)&uartptrData, (portTickType)500/*portMAX_DELAY*/);        
        printf("receive msg from uart_rsp_q,rx_len %d bytes,rspMsgLen %d.\n",
            uartptrData.rx_len, rspMsgLen);

        if (uartptrData.rx_len > 0)
        {
            if (uartptrData.rx_len <= rspMsgLen) 
            {
                memcpy(rspMsg, uartptrData.rx_buf, uartptrData.rx_len);
                ret = OP_OK;
            }
            else 
            {
                printf("Err: UART1 received data more than %d bytes\n", rspMsgLen);
                ret = OP_FAIL;
            }
            checksum = 0;
            for (i = 2; i < uartptrData.rx_len - 2; i++) {
                checksum += rspMsg[i];
            }

            if (rspMsg[i] != checksum) {
                printf("rsp checksum:%02x,expect checksum:%02x\n",rspMsg[i], checksum);
                ret = OP_FAIL;
            }
        }
        else
        {
            printf("Err: no data received on UART1\n");
            ret = OP_FAIL;     
        }
    }while((ret == OP_FAIL)&&(send_times < MSG_RESEND_TIMES));

    return ret;
}

#endif
