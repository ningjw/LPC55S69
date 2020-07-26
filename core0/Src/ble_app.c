#include "main.h"


#define DEVICE_BLE_NAME "BLE Communication"

#define SET_COMMOND_MODE()       GPIO_PinWrite(GPIO, BOARD_BT_MODE_PORT, BOARD_BT_MODE_PIN, 1);
#define SET_THROUGHPUT_MODE()    GPIO_PinWrite(GPIO, BOARD_BT_MODE_PORT, BOARD_BT_MODE_PIN, 0);
#define BLE_POWER_ON()           GPIO_PinWrite(GPIO, BOARD_BT_PWR_EN_PORT, BOARD_BT_PWR_EN_PIN, 1);
#define BLE_RESET()              GPIO_PinWrite(GPIO, BOARD_BT_PWR_EN_PORT, BOARD_BT_PWR_EN_PIN, 0);


extern void LPUART2_init(void);


uint8_t g_flexcomm3TxBuf[FLEXCOMM3_BUFF_LEN] = {0};//串口发送缓冲区
uint8_t g_flexcomm3Buf[FLEXCOMM3_BUFF_LEN] = {0};//串口接收缓冲区

uint16_t g_flexcomm3RxCnt = 0;
uint16_t g_puart2TxCnt = 0;
uint8_t g_flexcomm3StartRx = false;
uint32_t  g_flexcomm3RxTimeCnt = 0;
uint32_t ble_event = 0;
uint32_t BleStartFlag = false;

TaskHandle_t        BLE_TaskHandle = NULL;//蓝牙任务句柄


/***************************************************************************************
  * @brief   发送一个字符串
  * @input   base:选择端口; data:将要发送的数据
  * @return
***************************************************************************************/
void FLEXCOMM3_SendStr(const char *str)
{
	USART_WriteBlocking(FLEXCOMM3_PERIPHERAL, (uint8_t *)str, strlen(str));
}

/***************************************************************************************
  * @brief   设置WIFI模块为Ap工作模式
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
* 功能：发送AT指令
* 输入: send_buf:发送的字符串
		recv_str：期待回令中包含的子字符串
        p_at_cfg：AT配置
* 输出：执行结果代码
******************************************************************/
uint8_t BLE_SendCmd(const char *cmd, const char *recv_str, uint16_t time_out)
{
    uint8_t try_cnt = 0;
	g_flexcomm3RxCnt = 0;
	memset(g_flexcomm3Buf, 0, FLEXCOMM3_BUFF_LEN);
retry:
    FLEXCOMM3_SendStr(cmd);//发送AT指令
    /*wait resp_time*/
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, time_out);
    //接收到的数据中包含响应的数据
    if(strstr((char *)g_flexcomm3Buf, recv_str) != NULL) {
        return true;
    } else {
        if(try_cnt++ > 3) {
            return false;
        }
        goto retry;//重试
    }
}
/***************************************************************************************
  * @brief   设置蓝牙模块
  * @input   
  * @return
***************************************************************************************/
void BLE_Init(void)
{
	SET_COMMOND_MODE();
	BLE_SendCmd("AT\r\n","OK",500);
	BLE_SendCmd("AT+BAUD=115200\r\n","OK",300);
	BLE_SendCmd("AT+NAME=BLE Communication\r\n","OK",300);/* 设置蓝牙名称 */
	BLE_SendCmd("AT+VER\r\n","OK",300);/* 读取版本号 */
	BLE_SendCmd("AT+LPM=0\r\n","OK",300);/*关闭低功耗模式*/
    BLE_SendCmd("AT+TPMODE=1\r\n","OK",300);/* 开启透传模式 */
	SET_THROUGHPUT_MODE();
	g_sys_para.BleWifiLedStatus = BLE_READY;
}

/***********************************************************************
  * @ 函数名  ： BLE_AppTask
  * @ 功能说明：
  * @ 参数    ： 无
  * @ 返回值  ： 无
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

            /* 处理蓝牙数据协议 */
            sendBuf = ParseProtocol(g_flexcomm3Buf);

			/* 是否接受完成整个数据包 */
			if( g_sys_para.firmUpdate == true) {
				//将参数存入Nor Flash
//				NorFlash_SaveUpgradePara();
				//关闭所有中断,并复位系统
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
        else if(pdTRUE == xReturn && ble_event == EVT_TIMTOUT) { //接受蓝牙数据超时
			g_flexcomm3StartRx = false;
			
			uint8_t id = 100;
			if(g_flexcomm3Buf[7] == ',' && g_flexcomm3Buf[6] >= '0' && g_flexcomm3Buf[6] <= '9'){
				id = 100 + (g_flexcomm3Buf[6]-0x30);
			}else if(g_flexcomm3Buf[6] >= '0' && g_flexcomm3Buf[6] <= '9' && 
			         g_flexcomm3Buf[7] >= '0' && g_flexcomm3Buf[7] <= '9'){
				id = 100 + (g_flexcomm3Buf[6] - 0x30) * 10 + (g_flexcomm3Buf[7] - 0x30);
			}
			/*制作cjson格式的回复消息*/
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
        //清空接受到的数据
        memset(g_flexcomm3Buf, 0, FLEXCOMM3_BUFF_LEN);
        g_flexcomm3RxCnt = 0;

        /* 判断蓝牙连接状态*/
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
				printf("\n接受数据超时,当前接受%d个数据\n", g_flexcomm3RxCnt);
				for(uint8_t i = 0;i<g_flexcomm3RxCnt; i++){
					printf("%02x ",g_flexcomm3Buf[i]);
				}
				xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
			}
		}
		else if(g_flexcomm3RxTimeCnt >= 30) { //30ms未接受到数据,表示接受数据超时
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

    /*串口接收到数据*/
    if ( USART_GetStatusFlags(FLEXCOMM3_PERIPHERAL) & (kUSART_RxFifoNotEmptyFlag | kUSART_RxError) )
    {
        /*读取数据*/
        ucTemp = USART_ReadByte(FLEXCOMM3_PERIPHERAL);
		g_flexcomm3StartRx = true;
		g_flexcomm3RxTimeCnt = 0;
		g_sys_para.inactiveCount = 0;/* 接受到蓝牙数据就清0计数器*/
		if(g_flexcomm3RxCnt < FLEXCOMM3_BUFF_LEN) {
			/* 将接受到的数据保存到数组*/
			g_flexcomm3Buf[g_flexcomm3RxCnt++] = ucTemp;
		}
		
		if(g_sys_para.BleWifiLedStatus != BLE_UPDATE && g_flexcomm3Buf[g_flexcomm3RxCnt-1] == '}'){
			/* 接受完成,该标志清0*/
			g_flexcomm3StartRx = false;
			xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
		}else if (g_sys_para.BleWifiLedStatus==BLE_UPDATE && g_flexcomm3RxCnt >= FIRM_ONE_PACKE_LEN){
			/* 接受完成,该标志清0*/
			g_flexcomm3StartRx = false;
			xTaskNotify(BLE_TaskHandle, EVT_OK, eSetBits);
		}
    }
    __DSB();
}


