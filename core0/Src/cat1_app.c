#include "main.h"

TaskHandle_t CAT1_TaskHandle = NULL;

uint8_t g_Cat1RxBuffer[1024] = {0};
uint8_t g_Cat1TxBuffer[1024] = {0};

uint16_t g_Cat1RxCnt = 0;
uint8_t  g_Cat1StartRx = false;
uint32_t g_Cat1RxTimeCnt = 0;

uint32_t cat1_event = 0;

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
uint8_t CAT1_SendCmd(const char *cmd, const char *recv_str, uint16_t time_out)
{
    uint8_t try_cnt = 0;
	g_Cat1RxCnt = 0;
	memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
nb_retry:
    FLEXCOMM2_SendStr(cmd);//����ATָ��
	
    /*wait resp_time*/
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, time_out);
	
    //���յ��������а�����Ӧ������
    if(strstr((char *)g_Cat1RxBuffer, recv_str) != NULL) {
        return true;
    } else {
        if(try_cnt++ > 3) {
            return false;
        }
        goto nb_retry;//����
    }
}

//��CAT1���صĽ���н�ȡ��Ҫ���ַ���
char* substr(char *src, char* head)
{
    uint8_t len = strlen(head);
    char *p = NULL;
    p = strchr(src+len, '\r');
    if(p != NULL){
        *p = 0x00;
    }else{
        return NULL;
    }
    return src+len;
}


/* CAT1-IoT ģ���ʼ�� */
void CAT1_Init()
{
	//wait "WH-GM5"
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
	
	while(CAT1_SendCmd("+++", "a", 1000) == false){
		vTaskDelay(10);
	}
	
	if(CAT1_SendCmd("a","+ok", 1000)==false)
	{
		DEBUG_PRINTF("********** WIFI Init error \r\n");
		g_sys_para.sampLedStatus = WORK_FATAL_ERR;
		return;
	}

	if(g_sys_flash_para.Cat1InitFlag != 0xAA)
	{
		CAT1_SendCmd("AT+E=OFF\r\n" ,"OK", 200);
        
        FLEXCOMM2_SendStr("AT+SN?\r\n");
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, 300);
        if(strstr((char *)g_Cat1RxBuffer,"OK") != NULL){
            char *s = substr((char *)g_Cat1RxBuffer,"+SN:");
            if(s){
                strncpy(g_sys_flash_para.SN, s, sizeof(g_sys_flash_para.SN));
            }
        }
        
        FLEXCOMM2_SendStr("AT+AT+IMEI?\r\n");
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, 300);
        if(strstr((char *)g_Cat1RxBuffer,"OK") != NULL){
            char *s = substr((char *)g_Cat1RxBuffer,"+IMEI:");
            if(s){
                strncpy(g_sys_flash_para.IMEI, s, sizeof(g_sys_flash_para.IMEI));
            }
        }
        
        FLEXCOMM2_SendStr("AT+AT+ICCID?\r\n");
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, 300);
        if(strstr((char *)g_Cat1RxBuffer,"OK") != NULL){
            char *s = substr((char *)g_Cat1RxBuffer,"+ICCID:");
            if(s){
                strncpy(g_sys_flash_para.ICCID, s, sizeof(g_sys_flash_para.ICCID));
            }
        }
        
		CAT1_SendCmd("AT+WKMOD=NET\r\n", "OK", 200);//�������
		
		CAT1_SendCmd("AT+SOCKAEN=ON\r\n" ,"OK", 200);
#if 0
        CAT1_SendCmd("AT+HEARTEN=OFF\r\n" ,"OK", 200);//�ָ���������
		CAT1_SendCmd("AT+REGEN=ON\r\n" ,"OK", 200);//ע�������
		CAT1_SendCmd("AT+REGTP=USER\r\n" ,"OK", 200);//�Զ�������
		CAT1_SendCmd("AT+REGSND=LINK\r\n" ,"OK", 200);//Ϊ��������ʱ����
#endif
		CAT1_SendCmd("AT+SOCKA=TCP,183.230.40.40,1811\r\n" ,"OK", 1000);
		CAT1_SendCmd("AT+S\r\n" ,"OK", 200);
        
		g_sys_flash_para.Cat1InitFlag = 0xAA;
		Flash_SavePara();
        //AT+S������ģ��,�ڴ˴��ȴ�ģ�鷢��"WH-GM5"
		xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
	}
    
    FLEXCOMM2_SendStr("AT+CSQ\r\n");
    if(strstr((char *)g_Cat1RxBuffer,"OK") != NULL){
        char *s = substr((char *)g_Cat1RxBuffer,"+CSQ:");
        if(s){
            strncpy(g_sys_flash_para.ICCID, s, sizeof(g_sys_flash_para.ICCID));
        }
    }
	if(CAT1_SendCmd("AT+SOCKALK?\r\n" ,"Connected", 1000) == true)
	{
		g_sys_para.Cat1LinkStatus = true;
		CAT1_SendCmd("*388752#20201130#server*","OK",10000);
	}
	else
	{
		g_sys_para.Cat1LinkStatus = false;
	}
}


/* ������ͨ��NBģ���ϴ���OneNet*/
void CAT1_UploadSampleData(void)
{
    uint8_t xReturn = pdFALSE;
    uint32_t sid = 0;
    uint32_t len = 0;
    uint8_t  retry = 0;
    
    PWR_CAT1_ON;//����
    CAT1_Init();//��ʼ��CAT1ģ��
    if(g_sys_para.Cat1LinkStatus == false){
        return;
    }
    
NEXT_SID:
    
    memset(g_commTxBuf, 0, FLEXCOMM_BUFF_LEN);
    len = PacketUploadSampleData(g_commTxBuf, sid);
    USART_WriteBlocking(FLEXCOMM2_PERIPHERAL, g_commTxBuf, len);
    
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, 10000);//�ȴ��������ظ�����,��ʱʱ��10S
    if(pdTRUE == xReturn){
        if(cat1_event == EVT_UART_TIMTOUT && strstr((char *)g_Cat1RxBuffer, "OK") != NULL) {//�ɹ����ܵ����������͵�OK�ַ�
            sid ++;
            if(sid < g_sys_para.sampPacksByWifiCat1){//�������ݰ�δ����
                goto NEXT_SID;
            }
        }
    }else if(retry < 3){//��ʱ,�����Դ���С��3
        retry++;
        goto NEXT_SID;
    }
    
    //�������ݰ�������ɺ�,����Ҫ���͵�ǰ״̬��������
    memset(g_commTxBuf, 0, FLEXCOMM_BUFF_LEN);
    CAT1_SendCmd((char *)g_commTxBuf, "OK", 10000);
    
    PWR_CAT1_OFF;//�ػ�
}


/* NB-IOTģ���ʼ������ */
void CAT1_AppTask(void)
{
	uint8_t xReturn = pdFALSE;
	//CAT1_Init();
	DEBUG_PRINTF("CAT1_AppTask Running\r\n");
	while(1)
	{
		/*wait task notify*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
		if ( pdTRUE == xReturn) 
		{
			if(cat1_event == EVT_UPLOAD_SAMPLE)//�������,�����������ϴ�
            {
                CAT1_UploadSampleData();
            }
		}
		//��ս��ܵ�������
        memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
        g_Cat1RxCnt = 0;
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
		g_Cat1StartRx = true;
		if(g_Cat1RxCnt < sizeof(g_Cat1RxBuffer)) {
			/* �����ܵ������ݱ��浽����*/
			g_Cat1RxBuffer[g_Cat1RxCnt++] = ucTemp;
			
		}
	}
}

void FLEXCOMM2_TimeTick(void)
{
    if(g_Cat1StartRx )
    {
        g_Cat1RxTimeCnt++;
		if(g_Cat1RxTimeCnt >= 10) { //10msδ���ܵ�����,��ʾ�������ݳ�ʱ
			g_Cat1RxTimeCnt = 0;
			g_Cat1StartRx = false;
			xTaskNotify(CAT1_TaskHandle, EVT_UART_TIMTOUT, eSetBits);
        }
    }
}

