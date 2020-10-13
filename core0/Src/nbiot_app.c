#include "main.h"

TaskHandle_t NB_TaskHandle = NULL;
usart_transfer_t NbXfer;

uint8_t g_NbRxBuffer[1024] = {0};
uint8_t g_NbTxBuffer[128] = {0};

uint16_t g_NbRxCnt = 0;
uint8_t  g_NbStartRx = false;
uint32_t g_NbRxTimeCnt = 0;

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

uint32_t NB73_get_id(char *data, uint8_t index)
{
    int i = 0;
    char *temp = NULL;

    for(i = 0; i < index; i++)
    {
        temp = strstr(data, ",");
        if(temp == NULL)
        {
            temp = strstr(data, ":");
        }
        data = data + (temp - data) + 1;
    }

    return atoi(data);
}

/* ������ͨ��NBģ���ϴ���OneNet*/
void NB_UploadData(void)
{
	//�ȴ�ģ�鸽������
	NB_SendCmd("AT+CGATT?\r\n" ,"+CGATT:1", 2000);
	
	//
	NB_SendCmd("AT+MIPLCLOSE=0\r\n","OK",200);
	
	NB_SendCmd("AT+MIPLDELETE=0\r\n","OK",200);
	
	//����ͨ���׼�
	NB_SendCmd("AT+MIPLCREATE\r\n" ,"+MIPL", 2000);
	
	//����¶ȶ���
	NB_SendCmd("AT+MIPLADDOBJ=0,3303,1,\"1\",6,1\r\n" ,"OK", 2000);
	
	//���ʱ����ֵ����
	NB_SendCmd("AT+MIPLADDOBJ=0,3304,1,\"1\",6,1\r\n","OK", 2000);
	
	//���XXX����
	NB_SendCmd("AT+MIPLADDOBJ=0,3201,1,\"1\",3,0\r\n","OK", 2000);
	
	//����ע������
	NB_SendCmd("AT+MIPLOPEN=0,3600,30\r\n","OK", 2000);
	
	//��Դ����
	NB_SendCmd("AT+MIPLOPEN\r\n" ,"+MIPLEVENT:6", 2000);
}


/* NB-IOTģ���ʼ������ */
void NB_AppTask(void)
{
	uint8_t xReturn = pdFALSE;
	uint32_t mid;
	uint32_t oid;
	uint32_t eid;
	uint8_t  value;
	
	NbXfer.data = g_NbRxBuffer;
	NbXfer.dataSize = sizeof(g_NbRxBuffer);
	NB_Init();
	while(1)
	{
		/*wait task notify*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &nb_event, portMAX_DELAY);
		if ( pdTRUE == xReturn && EVT_TIMTOUT == EVT_OK) 
		{
			if(strstr((char *)g_NbRxBuffer, "+MIPLDISCOVER"))
            {
                mid = NB73_get_id((char *)g_NbRxBuffer, 1);
                oid = NB73_get_id((char *)g_NbRxBuffer, 2);
            }
            else if(strstr((char *)g_NbRxBuffer, "+MIPLOBSERVE"))
            {
                mid = NB73_get_id((char *)g_NbRxBuffer, 1);
                oid = NB73_get_id((char *)g_NbRxBuffer, 3);
            }
            else if(strstr((char *)g_NbRxBuffer, "+MIPLREAD"))
            {
                mid = NB73_get_id((char *)g_NbRxBuffer, 1);
                oid = NB73_get_id((char *)g_NbRxBuffer, 2);
            }
            else if(strstr((char *)g_NbRxBuffer,"+MIPLWRITE"))
            {

            }
            else if(strstr((char *)g_NbRxBuffer,"+MIPLEVENT"))
            {
                eid = NB73_get_id((char *)g_NbRxBuffer, 1);
            }
            else
            {
                return;
            }
			FLEXCOMM2_SendStr((char *)g_NbTxBuffer);
		}
		//��ս��ܵ�������
        memset(g_NbRxBuffer, 0, sizeof(g_NbRxBuffer));
        g_flexcomm3RxCnt = 0;
	}
}

/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void FLEXCOMM2_IRQHandler(void)
{
	uint8_t ucTemp;
	/*���ڽ��յ�����*/
    if( USART_GetStatusFlags(FLEXCOMM2_PERIPHERAL) & (kUSART_RxFifoNotEmptyFlag | kUSART_RxError) )
    {
		/*��ȡ����*/
        ucTemp = USART_ReadByte(FLEXCOMM2_PERIPHERAL);
		g_NbStartRx = true;
		if(g_NbRxCnt < sizeof(g_NbRxBuffer)) {
			/* �����ܵ������ݱ��浽����*/
			g_NbRxBuffer[g_NbRxCnt++] = ucTemp;
			
		}
	}
}

void FLEXCOMM2_TimeTick(void)
{
    if(g_NbStartRx )
    {
        g_NbRxTimeCnt++;
		if(g_NbRxTimeCnt >= 10) { //10msδ���ܵ�����,��ʾ�������ݳ�ʱ
			g_NbRxTimeCnt = 0;
			g_NbStartRx = false;
			xTaskNotify(NB_TaskHandle, EVT_TIMTOUT, eSetBits);
        }
    }
}

