#include "main.h"

TaskHandle_t NB_TaskHandle = NULL;
usart_transfer_t NbXfer;

uint8_t g_NbRxBuffer[1024] = {0};
uint8_t g_NbTxBuffer[64] = {0};

uint32_t nb_event = 0;

/***************************************************************************************
  * @brief   ����һ���ַ���
  * @input   base:ѡ��˿�; data:��Ҫ���͵�����
  * @return
***************************************************************************************/
void FLEXCOMM2_SendStr(const char *str)
{
	USART_WriteBlocking(FLEXCOMM2_PERIPHERAL, (uint8_t *)str, strlen(str));
}

/* UART user callback */
void UART_UserCallback(USART_Type *base,usart_dma_handle_t *handle,status_t status,void *userData)
{
	if (kStatus_USART_RxIdle == status)
    {
		USART_TransferReceiveDMA(FLEXCOMM2_PERIPHERAL, &g_uartDmaHandle, &NbXfer);
		xTaskNotify(NB_TaskHandle, NB_OK, eSetBits);
	}
}

/*****************************************************************
* ���ܣ�����ATָ��
* ����: send_buf:���͵��ַ���
		recv_str���ڴ������а��������ַ���
        p_at_cfg��AT����
* �����ִ�н������
******************************************************************/
uint8_t NB_SendCmd(const char *cmd, const char *recv_str, uint16_t time_out)
{
    uint8_t try_cnt = 0;
	memset(g_NbRxBuffer, 0, sizeof(g_NbRxBuffer));
nb_retry:
    FLEXCOMM2_SendStr(cmd);//����ATָ��
	
    /*wait resp_time*/
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &nb_event, time_out);
	
    //���յ��������а�����Ӧ������
    if(strstr((char *)g_NbRxBuffer, recv_str) != NULL) {
        return true;
    } else {
        if(try_cnt++ > 3) {
            return false;
        }
        goto nb_retry;//����
    }
}




/* NB-IoT ģ���ʼ�� */
void NB_Init()
{
	NB_SendCmd("AT\r\n","OK",1000);//����ATָ��ģʽ
	
	NB_SendCmd("AT+WKMOD=ONENET\r\n", "OK", 200);//�������
	
	NB_SendCmd("AT+CGATT?\r\n" ,"OK", 200);
	
	NB_SendCmd("AT+S\r\n", "OK", 200);
	
	NB_SendCmd("AT+MIPLCLOSE=0\r\n", "OK", 200);
	
	NB_SendCmd("AT+MIPLDELETE=0\r\n","OK", 200);
	
	NB_SendCmd("AT+MIPLCREATE\r\n" ,"OK", 200);
	
	NB_SendCmd("AT+MIPLADDOBJ=0,3303,1,\"1\",6,1\r\n" ,"OK", 200);
	
	NB_SendCmd("AT+MIPLADDOBJ=0,3304,1,\"1\",6,1\r\n" ,"OK", 200);
	
	NB_SendCmd("AT+MIPLADDOBJ=0,3201,1,\"1\",3,0\r\n" ,"OK", 200);
	
	NB_SendCmd("AT+MIPLOPEN=0,3600,30\r\n" ,"OK", 200);
	
	
	
	NB_SendCmd("AT+MIPLNOTIFY=0,%u,3304,0,5700,4,%d,%s,0,0\r\n" ,"OK", 200);
	
	FLEXCOMM2_SendStr("AT+ENTM\r\n");//�˳�ATָ��ģʽ
}

/* NB-IOTģ���ʼ������ */
void NB_AppTask(void)
{
	NbXfer.data = g_NbRxBuffer;
	NbXfer.dataSize = sizeof(g_NbRxBuffer);
	
	while(1)
	{
		vTaskDelay(1);
	}
}





