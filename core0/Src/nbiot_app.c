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
uint8_t NB_SendCmd(const char *cmd, const char *recv_str, uint16_t time_out)
{
    uint8_t try_cnt = 0;
	memset(g_NbRxBuffer, 0, sizeof(g_NbRxBuffer));
nb_retry:
    FLEXCOMM2_SendStr(cmd);//发送AT指令
	
    /*wait resp_time*/
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &nb_event, time_out);
	
    //接收到的数据中包含响应的数据
    if(strstr((char *)g_NbRxBuffer, recv_str) != NULL) {
        return true;
    } else {
        if(try_cnt++ > 3) {
            return false;
        }
        goto nb_retry;//重试
    }
}


/* NB-IoT 模块初始化 */
void NB_Init()
{
	NB_SendCmd("AT\r\n","OK",1000);//进入AT指令模式
	
	NB_SendCmd("AT+WKMOD=ONENET\r\n", "OK", 200);//检查网络
	
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
	
	FLEXCOMM2_SendStr("AT+ENTM\r\n");//退出AT指令模式
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

/* 将数据通过NB模块上传到OneNet*/
void NB_UploadData(void)
{
	//等待模块附着网络
	NB_SendCmd("AT+CGATT?\r\n" ,"+CGATT:1", 2000);
	
	//
	NB_SendCmd("AT+MIPLCLOSE=0\r\n","OK",200);
	
	NB_SendCmd("AT+MIPLDELETE=0\r\n","OK",200);
	
	//创建通信套件
	NB_SendCmd("AT+MIPLCREATE\r\n" ,"+MIPL", 2000);
	
	//添加温度对象
	NB_SendCmd("AT+MIPLADDOBJ=0,3303,1,\"1\",6,1\r\n" ,"OK", 2000);
	
	//添加时域总值对象
	NB_SendCmd("AT+MIPLADDOBJ=0,3304,1,\"1\",6,1\r\n","OK", 2000);
	
	//添加XXX对象
	NB_SendCmd("AT+MIPLADDOBJ=0,3201,1,\"1\",3,0\r\n","OK", 2000);
	
	//发起注册请求
	NB_SendCmd("AT+MIPLOPEN=0,3600,30\r\n","OK", 2000);
	
	//资源发现
	NB_SendCmd("AT+MIPLOPEN\r\n" ,"+MIPLEVENT:6", 2000);
}


/* NB-IOT模块初始化函数 */
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
		//清空接受到的数据
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
	/*串口接收到数据*/
    if( USART_GetStatusFlags(FLEXCOMM2_PERIPHERAL) & (kUSART_RxFifoNotEmptyFlag | kUSART_RxError) )
    {
		/*读取数据*/
        ucTemp = USART_ReadByte(FLEXCOMM2_PERIPHERAL);
		g_NbStartRx = true;
		if(g_NbRxCnt < sizeof(g_NbRxBuffer)) {
			/* 将接受到的数据保存到数组*/
			g_NbRxBuffer[g_NbRxCnt++] = ucTemp;
			
		}
	}
}

void FLEXCOMM2_TimeTick(void)
{
    if(g_NbStartRx )
    {
        g_NbRxTimeCnt++;
		if(g_NbRxTimeCnt >= 10) { //10ms未接受到数据,表示接受数据超时
			g_NbRxTimeCnt = 0;
			g_NbStartRx = false;
			xTaskNotify(NB_TaskHandle, EVT_TIMTOUT, eSetBits);
        }
    }
}

