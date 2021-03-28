#include "main.h"


#if defined(BLE_VERSION) || defined(WIFI_VERSION)

#define DEVICE_BLE_NAME "BLE Communication"

extern void LPUART2_init(void);


uint8_t g_flexcomm3Buf[FLEXCOMM_BUFF_LEN] = {0};//���ڽ��ջ�����

uint16_t g_flexcomm3RxCnt = 0;
uint8_t g_flexcomm3StartRx = false;
uint32_t  g_flexcomm3RxTimeCnt = 0;
uint32_t ble_event = 0;
uint32_t BleStartFlag = false;

TaskHandle_t        BLE_WIFI_TaskHandle = NULL;//����������


/***************************************************************************************
  * @brief   ����һ���ַ���
  * @input   base:ѡ��˿�; data:��Ҫ���͵�����
  * @return
***************************************************************************************/
void FLEXCOMM3_SendStr(const char *str)
{
	USART_WriteBlocking(FLEXCOMM3_PERIPHERAL, (uint8_t *)str, strlen(str));
}
#ifdef WIFI_VERSION

/*****************************************************************
* ���ܣ�����ATָ��
* ����: send_buf:���͵��ַ���
		recv_str���ڴ������а��������ַ���
        p_at_cfg��AT����
* �����ִ�н������
******************************************************************/
uint8_t WIFI_SendCmd(const char *cmd, const char *recv_str, uint16_t time_out)
{
    uint8_t try_cnt = 0;
	g_flexcomm3RxCnt = 0;
	memset(g_flexcomm3Buf, 0, FLEXCOMM_BUFF_LEN);
retry:
    FLEXCOMM3_SendStr(cmd);//����ATָ��
    /*wait resp_time*/
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, time_out);
    //���յ��������а�����Ӧ������
    if(strstr((char *)g_flexcomm3Buf, recv_str) != NULL) {
		DEBUG_PRINTF("%s\r\n",g_flexcomm3Buf);
        return true;
    } else {
        if(try_cnt++ > 3) {
			DEBUG_PRINTF("send AT cmd fail\r\n");
            return false;
        }
		DEBUG_PRINTF("retry: %s\r\n",cmd);
        goto retry;//����
    }
}

/***************************************************************************************
  * @brief   ����WIFIģ��ΪAp����ģʽ
  * @input   
  * @return
***************************************************************************************/
void WIFI_Init(void)
{
	vTaskDelay(2000);
	
	while(WIFI_SendCmd("+++", "a", 1000) == false){
		vTaskDelay(10);
	}
	
	if(WIFI_SendCmd("a","OK", 1000)==false)
	{
		DEBUG_PRINTF("********** WIFI Init error \r\n");
		g_sys_para.sysLedStatus = WORK_FATAL_ERR;
		return;
	}

	WIFI_SendCmd("AT+E=off\r\n","OK", 300);
	
	WIFI_SendCmd("AT+UART=115200,8,1,NONE,NFC\r\n", "OK", 300);
	
	WIFI_SendCmd("AT+UARTTE\r\n", "OK", 300);
	
	WIFI_SendCmd("AT+WMODE=AP\r\n","OK", 300);
	
	WIFI_SendCmd("AT+WAP=USR-C322-,88888888\r\n","OK", 300);
	
	WIFI_SendCmd("AT+CHANNEL=1\r\n", "OK", 300);

	WIFI_SendCmd("AT+LANN=192.168.1.1,255.255.255.0\r\n", "OK", 300);

	WIFI_SendCmd("AT+SOCKA=TCPS,192.168.1.1,8899\r\n", "OK", 300);
	
	WIFI_SendCmd("AT+ENTM\r\n", "OK", 300);
	
	DEBUG_PRINTF("USR-C322 Init OK\r\n");
	g_sys_para.WifiBleInitFlag++;
	g_sys_para.BleWifiLedStatus = BLE_WIFI_READY;
	Flash_SavePara();
}
#endif


#ifdef BLE_VERSION
/*****************************************************************
* ���ܣ�����ATָ��
* ����: send_buf:���͵��ַ���
		recv_str���ڴ������а��������ַ���
        p_at_cfg��AT����
* �����ִ�н������
******************************************************************/
uint8_t BLE_SendCmd(const char *cmd, const char *recv_str, uint16_t time_out)
{
    uint8_t try_cnt = 0;
	g_flexcomm3RxCnt = 0;
	memset(g_flexcomm3Buf, 0, FLEXCOMM_BUFF_LEN);
retry:
    FLEXCOMM3_SendStr(cmd);//����ATָ��
    /*wait resp_time*/
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, time_out);
    //���յ��������а�����Ӧ������
    if(strstr((char *)g_flexcomm3Buf, recv_str) != NULL) {
        return true;
    } else {
        if(try_cnt++ > 3) {
            return false;
        }
        goto retry;//����
    }
}
/***************************************************************************************
  * @brief   ��������ģ��
  * @input   
  * @return
***************************************************************************************/
void BLE_Init(void)
{
	if(g_sys_para.WifiBleInitFlag != 0xAA){
		SET_COMMOND_MODE();
		BLE_SendCmd("AT\r\n","OK",500);
		BLE_SendCmd("AT+BAUD=115200\r\n","OK",300);
		BLE_SendCmd("AT+NAME=BLE Communication\r\n","OK",300);/* ������������ */
		BLE_SendCmd("AT+VER\r\n","OK",300);/* ��ȡ�汾�� */
		BLE_SendCmd("AT+LPM=0\r\n","OK",300);/*�رյ͹���ģʽ*/
		BLE_SendCmd("AT+TPMODE=1\r\n","OK",300);/* ����͸��ģʽ */
		g_sys_para.WifiBleInitFlag = 0xAA;
		Flash_SavePara();
	}
	SET_THROUGHPUT_MODE();
	g_sys_para.BleWifiLedStatus = BLE_WIFI_READY;
}
#endif
/***********************************************************************
  * @ ������  �� BLE_WIFI_AppTask
  * @ ����˵����
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void BLE_WIFI_AppTask(void)
{
    uint8_t xReturn = pdFALSE;
    DEBUG_PRINTF("BLE_WIFI_AppTask Running\r\n");
    uint8_t* sendBuf = NULL;

#ifdef BLE_VERSION
	BLE_Init();
#elif defined WIFI_VERSION
	WIFI_Init();
#endif
    
	BleStartFlag = true;
    memset(g_flexcomm3Buf, 0, FLEXCOMM_BUFF_LEN);
    g_flexcomm3RxCnt = 0;
	g_flexcomm3RxTimeCnt = 0;
	g_flexcomm3StartRx = false;
    while(1)
    {
        /*wait task notify*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, portMAX_DELAY);
        if ( xReturn && ble_event == EVT_UART_OK) {
		
            /* ��������/wifi���� */
            sendBuf = ParseProtocol(g_flexcomm3Buf);
			
			/* �Ƿ��������������ݰ� */
			if( g_sys_para.firmCore0Update == BOOT_NEW_VERSION) {
				//����������Nor Flash
				Flash_SavePara();
				//�ر������ж�,����λϵͳ
				NVIC_SystemReset();
			}
			
			if( NULL != sendBuf )
            {
                FLEXCOMM3_SendStr((char *)sendBuf);
                DEBUG_PRINTF("reply wifi data:\r\n%s\r\n",sendBuf);
                free(sendBuf);
                sendBuf = NULL;
            }
        }
#ifdef BLE_VERSION
        else if(xReturn && ble_event == EVT_UART_TIMTOUT && BleStartFlag) { //�����������ݳ�ʱ
			g_flexcomm3StartRx = false;
			
			uint8_t id = 100;
			if(g_flexcomm3Buf[7] == ',' && g_flexcomm3Buf[6] >= '0' && g_flexcomm3Buf[6] <= '9'){
				id = 100 + (g_flexcomm3Buf[6]-0x30);
			}else if(g_flexcomm3Buf[6] >= '0' && g_flexcomm3Buf[6] <= '9' && 
			         g_flexcomm3Buf[7] >= '0' && g_flexcomm3Buf[7] <= '9'){
				id = 100 + (g_flexcomm3Buf[6] - 0x30) * 10 + (g_flexcomm3Buf[7] - 0x30);
			}
			/*����cjson��ʽ�Ļظ���Ϣ*/
			cJSON *pJsonRoot = cJSON_CreateObject();
			cJSON_AddNumberToObject(pJsonRoot, "Id", id);
			char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
			FLEXCOMM3_SendStr(p_reply);
			DEBUG_PRINTF("%s: reply wifi data:\r\n%s\r\n", __func__,p_reply);
			
			cJSON_Delete(pJsonRoot);
			free(p_reply);
			p_reply = NULL;
        }
#endif
        //��ս��ܵ�������
        memset(g_flexcomm3Buf, 0, FLEXCOMM_BUFF_LEN);
        g_flexcomm3RxCnt = 0;
    }
}


/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void FLEXCOMM3_TimeTick(void)
{
    if(g_flexcomm3StartRx && BleStartFlag)
    {
        g_flexcomm3RxTimeCnt++;
		if(g_sys_para.BleWifiLedStatus == BLE_WIFI_UPDATE){
			if(g_flexcomm3RxTimeCnt >= 1000 ){
				g_flexcomm3RxTimeCnt = 0;
				g_flexcomm3StartRx = false;
				DEBUG_PRINTF("\nReceive time out\n", g_flexcomm3RxCnt);
				for(uint8_t i = 0;i<g_flexcomm3RxCnt; i++){
					DEBUG_PRINTF("%02x ",g_flexcomm3Buf[i]);
				}
				xTaskNotify(BLE_WIFI_TaskHandle, EVT_UART_OK, eSetBits);
			}
		}
		else if(g_flexcomm3RxTimeCnt >= 100) { //10msδ���ܵ�����,��ʾ�������ݳ�ʱ
			g_flexcomm3RxTimeCnt = 0;
			g_flexcomm3StartRx = false;
			xTaskNotify(BLE_WIFI_TaskHandle, EVT_UART_TIMTOUT, eSetBits);
        }
    }
}


/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void FLEXCOMM3_IRQHandler(void)
{
    uint8_t ucTemp;

    /*���ڽ��յ�����*/
    if ( USART_GetStatusFlags(FLEXCOMM3_PERIPHERAL) & (kUSART_RxFifoNotEmptyFlag | kUSART_RxError) )
    {
        /*��ȡ����*/
        ucTemp = USART_ReadByte(FLEXCOMM3_PERIPHERAL);
		
		g_flexcomm3StartRx = true;
		g_flexcomm3RxTimeCnt = 0;
		g_sys_para.sysIdleCount = 0;/* ���ܵ��������ݾ���0������*/
		if(g_flexcomm3RxCnt < FLEXCOMM_BUFF_LEN) {
			/* �����ܵ������ݱ��浽����*/
			g_flexcomm3Buf[g_flexcomm3RxCnt++] = ucTemp;
		}
		
		if(g_sys_para.BleWifiLedStatus != BLE_WIFI_UPDATE && g_flexcomm3Buf[g_flexcomm3RxCnt-1] == '}'){
			/* �������,�ñ�־��0*/
			g_flexcomm3StartRx = false;
			xTaskNotify(BLE_WIFI_TaskHandle, EVT_UART_OK, eSetBits);
		}else if (g_sys_para.BleWifiLedStatus==BLE_WIFI_UPDATE && g_flexcomm3RxCnt >= FIRM_ONE_PACKE_LEN){
			/* �������,�ñ�־��0*/
			g_flexcomm3StartRx = false;
			xTaskNotify(BLE_WIFI_TaskHandle, EVT_UART_OK, eSetBits);
		}
    }
    __DSB();
}

#endif
