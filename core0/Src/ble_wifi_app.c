#include "main.h"


#if defined(BLE_VERSION) || defined(WIFI_VERSION)

#define DEVICE_BLE_NAME "BLE Communication"

extern void LPUART2_init(void);


uint8_t g_flexcomm3Buf[FLEXCOMM_BUFF_LEN] = {0};//串口接收缓冲区

uint16_t g_flexcomm3RxCnt = 0;
uint8_t g_flexcomm3StartRx = false;
uint32_t  g_flexcomm3RxTimeCnt = 0;
uint32_t ble_event = 0;
uint32_t BleStartFlag = false;

TaskHandle_t        BLE_WIFI_TaskHandle = NULL;//蓝牙任务句柄


/***************************************************************************************
  * @brief   发送一个字符串
  * @input   base:选择端口; data:将要发送的数据
  * @return
***************************************************************************************/
void FLEXCOMM3_SendStr(const char *str)
{
	USART_WriteBlocking(FLEXCOMM3_PERIPHERAL, (uint8_t *)str, strlen(str));
}
#ifdef WIFI_VERSION

/*****************************************************************
* 功能：发送AT指令
* 输入: send_buf:发送的字符串
		recv_str：期待回令中包含的子字符串
        p_at_cfg：AT配置
* 输出：执行结果代码
******************************************************************/
uint8_t WIFI_SendCmd(const char *cmd, const char *recv_str, uint16_t time_out)
{
    uint8_t try_cnt = 0;
	g_flexcomm3RxCnt = 0;
	memset(g_flexcomm3Buf, 0, FLEXCOMM_BUFF_LEN);
retry:
    FLEXCOMM3_SendStr(cmd);//发送AT指令
    /*wait resp_time*/
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &ble_event, time_out);
    //接收到的数据中包含响应的数据
    if(strstr((char *)g_flexcomm3Buf, recv_str) != NULL) {
		DEBUG_PRINTF("%s\r\n",g_flexcomm3Buf);
        return true;
    } else {
        if(try_cnt++ > 3) {
			DEBUG_PRINTF("send AT cmd fail\r\n");
            return false;
        }
		DEBUG_PRINTF("retry: %s\r\n",cmd);
        goto retry;//重试
    }
}

/***************************************************************************************
  * @brief   设置WIFI模块为Ap工作模式
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
	memset(g_flexcomm3Buf, 0, FLEXCOMM_BUFF_LEN);
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
	if(g_sys_para.WifiBleInitFlag != 0xAA){
		SET_COMMOND_MODE();
		BLE_SendCmd("AT\r\n","OK",500);
		BLE_SendCmd("AT+BAUD=115200\r\n","OK",300);
		BLE_SendCmd("AT+NAME=BLE Communication\r\n","OK",300);/* 设置蓝牙名称 */
		BLE_SendCmd("AT+VER\r\n","OK",300);/* 读取版本号 */
		BLE_SendCmd("AT+LPM=0\r\n","OK",300);/*关闭低功耗模式*/
		BLE_SendCmd("AT+TPMODE=1\r\n","OK",300);/* 开启透传模式 */
		g_sys_para.WifiBleInitFlag = 0xAA;
		Flash_SavePara();
	}
	SET_THROUGHPUT_MODE();
	g_sys_para.BleWifiLedStatus = BLE_WIFI_READY;
}
#endif
/***********************************************************************
  * @ 函数名  ： BLE_WIFI_AppTask
  * @ 功能说明：
  * @ 参数    ： 无
  * @ 返回值  ： 无
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
		
            /* 处理蓝牙/wifi数据 */
            sendBuf = ParseProtocol(g_flexcomm3Buf);
			
			/* 是否接受完成整个数据包 */
			if( g_sys_para.firmCore0Update == BOOT_NEW_VERSION) {
				//将参数存入Nor Flash
				Flash_SavePara();
				//关闭所有中断,并复位系统
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
        else if(xReturn && ble_event == EVT_UART_TIMTOUT && BleStartFlag) { //接受蓝牙数据超时
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
			DEBUG_PRINTF("%s: reply wifi data:\r\n%s\r\n", __func__,p_reply);
			
			cJSON_Delete(pJsonRoot);
			free(p_reply);
			p_reply = NULL;
        }
#endif
        //清空接受到的数据
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
		else if(g_flexcomm3RxTimeCnt >= 100) { //10ms未接受到数据,表示接受数据超时
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

    /*串口接收到数据*/
    if ( USART_GetStatusFlags(FLEXCOMM3_PERIPHERAL) & (kUSART_RxFifoNotEmptyFlag | kUSART_RxError) )
    {
        /*读取数据*/
        ucTemp = USART_ReadByte(FLEXCOMM3_PERIPHERAL);
		
		g_flexcomm3StartRx = true;
		g_flexcomm3RxTimeCnt = 0;
		g_sys_para.sysIdleCount = 0;/* 接受到蓝牙数据就清0计数器*/
		if(g_flexcomm3RxCnt < FLEXCOMM_BUFF_LEN) {
			/* 将接受到的数据保存到数组*/
			g_flexcomm3Buf[g_flexcomm3RxCnt++] = ucTemp;
		}
		
		if(g_sys_para.BleWifiLedStatus != BLE_WIFI_UPDATE && g_flexcomm3Buf[g_flexcomm3RxCnt-1] == '}'){
			/* 接受完成,该标志清0*/
			g_flexcomm3StartRx = false;
			xTaskNotify(BLE_WIFI_TaskHandle, EVT_UART_OK, eSetBits);
		}else if (g_sys_para.BleWifiLedStatus==BLE_WIFI_UPDATE && g_flexcomm3RxCnt >= FIRM_ONE_PACKE_LEN){
			/* 接受完成,该标志清0*/
			g_flexcomm3StartRx = false;
			xTaskNotify(BLE_WIFI_TaskHandle, EVT_UART_OK, eSetBits);
		}
    }
    __DSB();
}

#endif
