#include "main.h"

TaskHandle_t CAT1_TaskHandle = NULL;
usart_transfer_t Cat1Xfer;

uint8_t g_Cat1RxBuffer[1024] = {0};
uint8_t g_Cat1TxBuffer[1024] = {0};

uint16_t g_Cat1RxCnt = 0;
uint8_t  g_Cat1StartRx = false;
uint32_t g_Cat1RxTimeCnt = 0;

uint32_t cat1_event = 0;

/***************************************************************************************
  * @brief   发送一个字符串
  * @input   base:选择端口; data:将要发送的数据
  * @return
***************************************************************************************/
void FLEXCOMM2_SendStr(const char *str)
{
	USART_WriteBlocking(FLEXCOMM2_PERIPHERAL, (uint8_t *)str, strlen(str));
}

/*****************************************************************
* 功能：发送AT指令
* 输入: send_buf:发送的字符串
		recv_str：期待回令中包含的子字符串
        p_at_cfg：AT配置
* 输出：执行结果代码
******************************************************************/
uint8_t CAT1_SendCmd(const char *cmd, const char *recv_str, uint16_t time_out)
{
    uint8_t try_cnt = 0;
	g_Cat1RxCnt = 0;
	memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
nb_retry:
    FLEXCOMM2_SendStr(cmd);//发送AT指令
	
    /*wait resp_time*/
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, time_out);
	
    //接收到的数据中包含响应的数据
    if(strstr((char *)g_Cat1RxBuffer, recv_str) != NULL) {
        return true;
    } else {
        if(try_cnt++ > 3) {
            return false;
        }
        goto nb_retry;//重试
    }
}

void NB_Reset(void)
{
	GPIO_PinWrite(GPIO, BOARD_NB_RST_PORT, BOARD_NB_RST_PIN, 0);
	vTaskDelay(300);
	GPIO_PinWrite(GPIO, BOARD_NB_RST_PORT, BOARD_NB_RST_PIN, 1);
	vTaskDelay(200);
}


/* NB-IoT 模块初始化 */
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
	
	if(g_sys_para.Cat1InitFlag != 0xAA)
	{
		CAT1_SendCmd("AT+E=OFF\r\n" ,"OK", 200);

		CAT1_SendCmd("AT+HEARTEN=OFF\r\n" ,"OK", 200);//恢复出厂设置
		
		CAT1_SendCmd("AT+CSQ\r\n" ,"OK", 1000);
		
		CAT1_SendCmd("AT+WKMOD=NET\r\n", "OK", 200);//检查网络
		
		CAT1_SendCmd("AT+SOCKAEN=ON\r\n" ,"OK", 200);
#if 0
		CAT1_SendCmd("AT+REGEN=ON\r\n" ,"OK", 200);//注册包功能
		CAT1_SendCmd("AT+REGTP=USER\r\n" ,"OK", 200);//自定义数据
		CAT1_SendCmd("AT+REGSND=LINK\r\n" ,"OK", 200);//为建立连接时发送
#endif
		CAT1_SendCmd("AT+SOCKA=TCP,183.230.40.40,1811\r\n" ,"OK", 1000);
		CAT1_SendCmd("AT+S\r\n" ,"OK", 200);
		g_sys_para.Cat1InitFlag = 0xAA;
		Flash_SavePara();
		xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
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


/* 将数据通过NB模块上传到OneNet*/
void CAT1_UploadData(void)
{

}


/* NB-IOT模块初始化函数 */
void CAT1_AppTask(void)
{
	uint8_t xReturn = pdFALSE;
	uint32_t mid;
	uint32_t oid;
	uint32_t eid;
	uint8_t  value;
	
	NB_Reset();
	
	Cat1Xfer.data = g_Cat1RxBuffer;
	Cat1Xfer.dataSize = sizeof(g_Cat1RxBuffer);
	CAT1_Init();
	DEBUG_PRINTF("CAT1_AppTask Running\r\n");
	while(1)
	{
		/*wait task notify*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
		if ( pdTRUE == xReturn && EVT_TIMTOUT == EVT_OK) 
		{
			
		}
		//清空接受到的数据
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
	/*串口接收到数据*/
    if( USART_GetStatusFlags(FLEXCOMM2_PERIPHERAL) & (kUSART_RxFifoNotEmptyFlag | kUSART_RxError) )
    {
		/*读取数据*/
        ucTemp = USART_ReadByte(FLEXCOMM2_PERIPHERAL);
		g_Cat1StartRx = true;
		if(g_Cat1RxCnt < sizeof(g_Cat1RxBuffer)) {
			/* 将接受到的数据保存到数组*/
			g_Cat1RxBuffer[g_Cat1RxCnt++] = ucTemp;
			
		}
	}
}

void FLEXCOMM2_TimeTick(void)
{
    if(g_Cat1StartRx )
    {
        g_Cat1RxTimeCnt++;
		if(g_Cat1RxTimeCnt >= 10) { //10ms未接受到数据,表示接受数据超时
			g_Cat1RxTimeCnt = 0;
			g_Cat1StartRx = false;
			xTaskNotify(CAT1_TaskHandle, EVT_TIMTOUT, eSetBits);
        }
    }
}

