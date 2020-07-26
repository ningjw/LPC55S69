#include "main.h"


#define DEVICE_BLE_NAME "BLE Communication"

#define SET_COMMOND_MODE()       GPIO_PinWrite(GPIO, BOARD_BT_MODE_PORT, BOARD_BT_MODE_PIN, 1);
#define SET_THROUGHPUT_MODE()    GPIO_PinWrite(GPIO, BOARD_BT_MODE_PORT, BOARD_BT_MODE_PIN, 0);
#define BLE_POWER_ON()           GPIO_PinWrite(GPIO, BOARD_BT_PWR_EN_PORT, BOARD_BT_PWR_EN_PIN, 1);
#define BLE_RESET()              GPIO_PinWrite(GPIO, BOARD_BT_PWR_EN_PORT, BOARD_BT_PWR_EN_PIN, 0);


extern void LPUART2_init(void);


uint8_t g_flexcomm3TxBuf[FLEXCOMM3_BUFF_LEN] = {0};//���ڷ��ͻ�����
uint8_t g_flexcomm3Buf[FLEXCOMM3_BUFF_LEN] = {0};//���ڽ��ջ�����

uint16_t g_flexcomm3RxCnt = 0;
uint16_t g_puart2TxCnt = 0;
uint8_t g_flexcomm3StartRx = false;
uint32_t  g_flexcomm3RxTimeCnt = 0;
uint32_t ble_event = 0;
uint32_t BleStartFlag = false;

TaskHandle_t        BLE_TaskHandle = NULL;//����������


/***************************************************************************************
  * @brief   ����һ���ַ���
  * @input   base:ѡ��˿�; data:��Ҫ���͵�����
  * @return
***************************************************************************************/
void FLEXCOMM3_SendStr(const char *str)
{
	USART_WriteBlocking(FLEXCOMM3_PERIPHERAL, (uint8_t *)str, strlen(str));
}

/***************************************************************************************
  * @brief   ����WIFIģ��ΪAp����ģʽ
  * @input   
  * @return
***************************************************************************************/
void WIFI_Init(void)
{
	FLEXCOMM3_SendStr("+++");
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 300);/*wait task notify*/
	
	FLEXCOMM3_SendStr("a");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 300);/*wait task notify*/
	
//	FLEXCOMM3_SendStr("AT+RELD\r\n");
//	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 1000);/*wait task notify*/
	
	FLEXCOMM3_SendStr("AT+E=off\r\n");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);
	
	FLEXCOMM3_SendStr("AT+WMODE=AP\r\n");
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/
	
	FLEXCOMM3_SendStr("AT+WAP=USR-C322-,88888888\r\n");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/
	
	FLEXCOMM3_SendStr("AT+CHANNEL=1\r\n");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/
	
	FLEXCOMM3_SendStr("AT+LANN=192.168.1.1,255.255.255.0\r\n");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/
	
	FLEXCOMM3_SendStr("AT+SOCKA=TCPS,192.168.1.1,8899\r\n");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/
	
//	FLEXCOMM3_SendStr("AT+WKMOD=TRANS\r\n");
//	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/

	FLEXCOMM3_SendStr("AT+ENTM\r\n");
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, 200);/*wait task notify*/
}


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
	memset(g_flexcomm3Buf, 0, FLEXCOMM3_BUFF_LEN);
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
	SET_COMMOND_MODE();
	BLE_SendCmd("AT\r\n","OK",500);
	BLE_SendCmd("AT+BAUD=115200\r\n","OK",300);
	BLE_SendCmd("AT+NAME=BLE Communication\r\n","OK",300);/* ������������ */
	BLE_SendCmd("AT+VER\r\n","OK",300);/* ��ȡ�汾�� */
	BLE_SendCmd("AT+LPM=0\r\n","OK",300);/*�رյ͹���ģʽ*/
    BLE_SendCmd("AT+TPMODE=1\r\n","OK",300);/* ����͸��ģʽ */
	SET_THROUGHPUT_MODE();
	g_sys_para.BleWifiLedStatus = BLE_READY;
}

/***********************************************************************
  * @ ������  �� BLE_AppTask
  * @ ����˵����
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void BLE_AppTask(void)
{
    uint8_t xReturn = pdFALSE;
    printf("BLE/WIFI Task Create and Running\r\n");
    uint8_t* sendBuf = NULL;

	#ifdef BLE_VERSION
	BLE_Init();
	#elif defined WIFI_VERSION
	WIFI_Init();
	#endif
    
	BleStartFlag = true;
    memset(g_flexcomm3Buf, 0, FLEXCOMM3_BUFF_LEN);
    g_flexcomm3RxCnt = 0;

    while(1)
    {
        /*wait task notify*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, portMAX_DELAY);
        if ( pdTRUE == xReturn && ble_event == EVT_OK) {

            /* ������������Э�� */
            sendBuf = ParseProtocol(g_flexcomm3Buf);

			/* �Ƿ��������������ݰ� */
			if( g_sys_para.firmUpdate == true) {
				//����������Nor Flash
//				NorFlash_SaveUpgradePara();
				//�ر������ж�,����λϵͳ
				NVIC_SystemReset();
			}

			if( NULL != sendBuf )
            {
                FLEXCOMM3_SendStr((char *)sendBuf);
                printf("%s",sendBuf);
                free(sendBuf);
                sendBuf = NULL;
            }
        }
#ifdef BLE_VERSION
        else if(pdTRUE == xReturn && ble_event == EVT_TIMTOUT) { //�����������ݳ�ʱ
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
			printf("%s", p_reply);
			
			cJSON_Delete(pJsonRoot);
			free(p_reply);
			p_reply = NULL;
        }
#endif
        //��ս��ܵ�������
        memset(g_flexcomm3Buf, 0, FLEXCOMM3_BUFF_LEN);
        g_flexcomm3RxCnt = 0;

        /* �ж���������״̬*/
		if(g_sys_para.BleWifiLedStatus != BLE_UPDATE){
			if(!BLE_WIFI_STATUS()) { //Disconnected
				g_sys_para.BleWifiLedStatus = BLE_READY;
			} else { 
				g_sys_para.BleWifiLedStatus = BLE_CONNECT;//Connected
			}
		}
    }
}


/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void FLEXCOMM3_TimeTick(void)
{
//	extern volatile uint32_t g_eventTimeMilliseconds;
//	g_eventTimeMilliseconds++;
	
    if(g_flexcomm3StartRx)
    {
        g_flexcomm3RxTimeCnt++;
		if(g_sys_para.BleWifiLedStatus == BLE_UPDATE){
			if(g_flexcomm3RxTimeCnt >= 1000 ){
				g_flexcomm3RxTimeCnt = 0;
				printf("\n�������ݳ�ʱ,��ǰ����%d������\n", g_flexcomm3RxCnt);
				for(uint8_t i = 0;i<g_flexcomm3RxCnt; i++){
					printf("%02x ",g_flexcomm3Buf[i]);
				}
				xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
			}
		}
		else if(g_flexcomm3RxTimeCnt >= 30) { //30msδ���ܵ�����,��ʾ�������ݳ�ʱ
			g_flexcomm3RxTimeCnt = 0;
			g_flexcomm3StartRx = false;
			xTaskNotify(BLE_TaskHandle, EVT_TIMTOUT, eSetBits);
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
		g_sys_para.inactiveCount = 0;/* ���ܵ��������ݾ���0������*/
		if(g_flexcomm3RxCnt < FLEXCOMM3_BUFF_LEN) {
			/* �����ܵ������ݱ��浽����*/
			g_flexcomm3Buf[g_flexcomm3RxCnt++] = ucTemp;
		}
		
		if(g_sys_para.BleWifiLedStatus != BLE_UPDATE && g_flexcomm3Buf[g_flexcomm3RxCnt-1] == '}'){
			/* �������,�ñ�־��0*/
			g_flexcomm3StartRx = false;
			xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
		}else if (g_sys_para.BleWifiLedStatus==BLE_UPDATE && g_flexcomm3RxCnt >= FIRM_ONE_PACKE_LEN){
			/* �������,�ñ�־��0*/
			g_flexcomm3StartRx = false;
			xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
		}
    }
    __DSB();
}


