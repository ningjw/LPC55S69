#include "main.h"

TaskHandle_t CAT1_TaskHandle = NULL;

uint8_t g_Cat1RxBuffer[1024] = {0};
uint8_t g_Cat1TxBuffer[1024] = {0};

uint16_t g_Cat1RxCnt = 0;
uint8_t  g_Cat1StartRx = false;
uint32_t g_Cat1RxTimeCnt = 0;

uint32_t cat1_event = 0;

MD5_CTX md5_ctx;
unsigned char md5_t[16];
char md5_result[40] = {0};
/***************************************************************************************
  * @brief   发送一个字符串
  * @input   base:选择端口; data:将要发送的数据
  * @return
***************************************************************************************/
void FLEXCOMM2_SendStr(const char *str)
{
    g_Cat1RxCnt = 0;
    memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
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
    if(strlen(cmd) == 0){
        return false;
    }
nb_retry:
    g_Cat1RxCnt = 0;
	memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
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

//从CAT1返回的结果中截取需要的字符串
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


void CAT1_EnterATMode(void)
{
    while(CAT1_SendCmd("+++", "a", 1000) == false){
		vTaskDelay(10);
	}

	if(CAT1_SendCmd("a","+ok", 1000)==false)
	{
		DEBUG_PRINTF("********** WIFI Init error \r\n");
		g_sys_para.sampLedStatus = WORK_FATAL_ERR;
		return;
	}
}



/* CAT1-IoT 从OneNet服务器上检测是否有新的固件可更新 */
void CAT1_CheckVersion(void)
{
    uint8_t haveNewVersion = false;
	uint32_t one_packet_len = 512;
	BaseType_t xReturn = pdFALSE;
	PWR_CAT1_OFF;
	vTaskDelay(100);
    PWR_CAT1_ON;//开机
	//wait "WH-GM5"
	xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, CAT1_WAIT_TICK);
	if(xReturn == pdFALSE)return;
	CAT1_EnterATMode();
    
    //从ntp服务器同步时间****************************************************************
    CAT1_SendCmd("AT+NTPEN=ON\r\n" ,"OK", 200);
    CAT1_SendCmd("AT+CCLK\r\n" ,"OK", CAT1_WAIT_TICK);
    char *s = strstr((char *)g_Cat1RxBuffer,"+CCLK: ");
    if(s){
        s = s+8;
        char *temp_data = strtok(s,",");
        char *temp_time = strtok(NULL,"+");
        
        char *year = strtok(temp_data,"/");
        char *mon = strtok(NULL,"/");
        char *day = strtok(NULL,"/");
        
        char *hour = strtok(temp_time,":");
        char *min = strtok(NULL,":");
        char *sec = strtok(NULL,":");
        
        sysTime.year = atoi(year) + 2000;
        sysTime.month = atoi(mon);
        sysTime.day = atoi(day);
        sysTime.hour = atoi(hour);
        sysTime.minute = atoi(min);
        sysTime.second = atoi(sec);
        /*设置日期和时间*/
        RTC_SetDatetime(RTC, &sysTime);
    }
    
	//配置SOCKA****************************************************************
	FLEXCOMM2_SendStr("AT+SOCKA?\r\n");
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, 300);
    if(strstr((char *)g_Cat1RxBuffer,"183.230.40.50") == NULL)
    {
        CAT1_SendCmd("AT+SOCKAEN=ON\r\n" ,"OK", 200);
        CAT1_SendCmd("AT+SOCKA=TCP,183.230.40.50,80\r\n" ,"OK", 1000);
        CAT1_SendCmd("AT+S\r\n" ,"OK", 200);
        //AT+S会重启模块,在此处等待模块发送"WH-GM5"
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
        CAT1_EnterATMode();
    }

    while(CAT1_SendCmd("AT+SOCKALK?\r\n" ,"Connected", 1000) == false){//等待连接服务器成功
        vTaskDelay(1000);
    }
    CAT1_SendCmd("AT+ENTM\r\n" ,"OK", 200);//进入透传模式

    //上报升级状态****************************************************************
    if(g_sys_flash_para.firmCore0Update == REPORT_VERSION){
        
        //上报升级成功状态
        g_Cat1RxCnt = 0;
        memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
        memset(g_Cat1TxBuffer, 0, sizeof(g_Cat1TxBuffer));
        snprintf((char *)g_Cat1TxBuffer, sizeof(g_Cat1TxBuffer),
            "POST /ota/south/device/download/ota_3lI1NIMLB6E4u260cLRd/progress?dev_id=%s HTTP/1.1\r\n"
            "Authorization:%s\r\n"
            "host:ota.heclouds.com\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length:12\r\n\r\n"
            "{\"step\":201}",
            g_sys_flash_para.device_id,AUTHORIZATION);
        FLEXCOMM2_SendStr((char *)g_Cat1TxBuffer);
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, CAT1_WAIT_TICK);
        char *json_string = strstr((char *)g_Cat1RxBuffer,"{");
        if(json_string && xReturn == pdTRUE){
            cJSON *pJson = cJSON_Parse(json_string);
            if(NULL == pJson) {
                return;
            }
            cJSON * pSub = cJSON_GetObjectItem(pJson, "errno");
            if(pSub->valueint == 0){//版本上报升级状态成功
                g_sys_flash_para.firmCore0Update = NO_VERSION;
                Flash_SavePara();
            }
        }
    }
    //上报版本****************************************************************
    g_Cat1RxCnt = 0;
    memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
    memset(g_Cat1TxBuffer, 0, sizeof(g_Cat1TxBuffer));
    snprintf((char *)g_Cat1TxBuffer, sizeof(g_Cat1TxBuffer),
          "POST http://ota.heclouds.com/ota/device/version?dev_id=%s HTTP/1.1\r\n"
          "Content-Type: application/json\r\n"
          "Authorization:%s\r\n"
          "Host:ota.heclouds.com\r\n"
          "Content-Length:%d\r\n\r\n"
          "{\"s_version\":\"%s\"}",
          g_sys_flash_para.device_id, AUTHORIZATION,(strlen(SOFT_VERSION)+16), SOFT_VERSION);
    
    FLEXCOMM2_SendStr((char *)g_Cat1TxBuffer);
    xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, CAT1_WAIT_TICK);
    
    //检测升级任务****************************************************************
    g_Cat1RxCnt = 0;
    memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
    memset(g_Cat1TxBuffer, 0, sizeof(g_Cat1TxBuffer));
    snprintf((char *)g_Cat1TxBuffer, sizeof(g_Cat1TxBuffer),
              "GET /ota/south/check?dev_id=%s&manuf=100&model=10001&type=2&version=V11&cdn=false HTTP/1.1\r\n"
              "Authorization:%s\r\n"
              "host: ota.heclouds.com\r\n\r\n",
              g_sys_flash_para.device_id,AUTHORIZATION);
    
    FLEXCOMM2_SendStr((char *)g_Cat1TxBuffer);
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
    char *json_string = strstr((char *)g_Cat1RxBuffer,"{");
    g_sys_flash_para.firmCore0Size = 0;
    if(json_string){
        cJSON *pJson = cJSON_Parse(json_string);
        if(NULL == pJson) {
            return;
        }
        cJSON * pSub = cJSON_GetObjectItem(pJson, "errno");
        if(pSub->valueint == 0){
            
            pSub = cJSON_GetObjectItem(pJson, "data");
            cJSON* item;
            item = cJSON_GetObjectItem(pSub,"target");
            if(item->valuestring){
				//判断当前版本号与目标版本号
				if(strcmp(item->valuestring, SOFT_VERSION) == 0){
					haveNewVersion = true;//有新的版本
					memset(g_sys_flash_para.firmUpdateTargetV, 0, sizeof(g_sys_flash_para.firmUpdateTargetV));
					strcpy(g_sys_flash_para.firmUpdateTargetV,item->valuestring);
				}
            }
            item = cJSON_GetObjectItem(pSub,"token");
            if(item->valuestring){
                memset(g_sys_flash_para.firmUpdateToken, 0, sizeof(g_sys_flash_para.firmUpdateToken));
                strcpy(g_sys_flash_para.firmUpdateToken,item->valuestring);
            }
            item = cJSON_GetObjectItem(pSub,"size");
            if(item->valueint){
                g_sys_flash_para.firmCore0Size = item->valueint;
                g_sys_flash_para.firmPacksTotal = g_sys_flash_para.firmCore0Size/one_packet_len + 
				                                 (g_sys_flash_para.firmCore0Size%one_packet_len?1:0);
				/* 按照文件大小擦除对应大小的空间 */
				memory_erase(CORE0_DATA_ADDR, g_sys_flash_para.firmCore0Size);
			}
            item = cJSON_GetObjectItem(pSub,"md5");
            if(item->valuestring){
                memset(g_sys_flash_para.firmUpdateMD5, 0, sizeof(g_sys_flash_para.firmUpdateMD5));
                strcpy(g_sys_flash_para.firmUpdateMD5,item->valuestring);
            }
        }
        cJSON_Delete(pJson);
    }
    
    //获取固件*****************************************************************************************************
    if(haveNewVersion){
		g_sys_flash_para.firmPacksCount = 0;
        uint32_t app_data_addr = CORE0_DATA_ADDR;
		
		MD5_Init(&md5_ctx);
		
GET_NEXT:
        g_Cat1RxCnt = 0;
        memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
        memset(g_Cat1TxBuffer, 0, sizeof(g_Cat1TxBuffer));
        snprintf((char *)g_Cat1TxBuffer, sizeof(g_Cat1TxBuffer),
                  "GET /ota/south/download/%s HTTP/1.1\r\n"
                  "Range:bytes=%d-%d\r\n"
                  "host: ota.heclouds.com\r\n\r\n",
                  g_sys_flash_para.firmUpdateToken, 
		           g_sys_flash_para.firmPacksCount*one_packet_len, 
		          (g_sys_flash_para.firmPacksCount+1)*one_packet_len-1);
        FLEXCOMM2_SendStr((char *)g_Cat1TxBuffer);
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
        
        char *data_ptr = strstr((char *)g_Cat1RxBuffer, "WH-GM5");
        if(data_ptr != NULL){//模块重启了,重新获取
            goto GET_NEXT;
        }
		
        //查找第一次出现0xD 0xA 0xD 0xA的位置
        data_ptr = strstr((char *)g_Cat1RxBuffer, "\r\n\r\n");
        char* md5_data_ptr = data_ptr + 4;
        if(data_ptr){
			data_ptr += 4;
            g_sys_flash_para.firmCurrentAddr = app_data_addr+g_sys_flash_para.firmPacksCount * one_packet_len;//
            g_sys_flash_para.firmPacksCount++;
            memory_write(g_sys_flash_para.firmCurrentAddr,(uint8_t *)(data_ptr), one_packet_len);
        }else{
            
        }
		
        //判断是否为最后一个包
		if(g_sys_flash_para.firmPacksCount < g_sys_flash_para.firmPacksTotal){
			MD5_Update(&md5_ctx, (unsigned char *)md5_data_ptr, one_packet_len);
			goto GET_NEXT;
		}else{
			char md5_t1[4] = {0, 0, 0, 0};
            uint32_t last_pack_len = 0;
            if(g_sys_flash_para.firmCore0Size%one_packet_len == 0){
                last_pack_len = one_packet_len;
            }else{
                last_pack_len = g_sys_flash_para.firmCore0Size%one_packet_len;
            }
			MD5_Update(&md5_ctx, (unsigned char *)md5_data_ptr, last_pack_len);
			MD5_Final(&md5_ctx, md5_t);

			memset(md5_result, 0, sizeof(md5_result));
			for(uint8_t i = 0; i < 16; i++)
			{
				if(md5_t[i] <= 0x0f)
					sprintf(md5_t1, "0%x", md5_t[i]);
				else
					sprintf(md5_t1, "%x", md5_t[i]);
				
				strcat(md5_result, md5_t1);
			}
			if(strcmp(md5_result, g_sys_flash_para.firmUpdateMD5) == 0)//md5校验成功
			{
                PWR_CAT1_OFF;
				g_sys_flash_para.firmCore0Update = BOOT_NEW_VERSION;
				Flash_SavePara();
				NVIC_SystemReset();
			}
		}
    }
    PWR_CAT1_OFF;
}

/* CAT1-IoT 模块初始化 */
void CAT1_SelfRegister()
{
	if(g_sys_flash_para.SelfRegisterFlag != 0xAA || strlen(g_sys_flash_para.SN) ==0 )
	{
		PWR_CAT1_OFF;
		vTaskDelay(100);
		PWR_CAT1_ON;
		//wait "WH-GM5"
		xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);

		CAT1_EnterATMode();
		
		CAT1_SendCmd("AT+CSQ\r\n" ,"OK", 300);
		
		CAT1_SendCmd("AT+E=OFF\r\n" ,"OK", 200);
		
		CAT1_SendCmd("AT+WKMOD=NET\r\n", "OK", 200);//检查网络

		CAT1_SendCmd("AT+SOCKAEN=ON\r\n" ,"OK", 200);
        CAT1_SendCmd("AT+SOCKBEN=OFF\r\n" ,"OK", 200);
        CAT1_SendCmd("AT+SOCKCEN=OFF\r\n" ,"OK", 200);
        CAT1_SendCmd("AT+SOCKDEN=OFF\r\n" ,"OK", 200);
		CAT1_SendCmd("AT+SOCKA=TCP,183.230.40.33,80\r\n" ,"OK", 1000);

        CAT1_SendCmd("AT+S\r\n" ,"OK", 200);
        //AT+S会重启模块,在此处等待模块发送"WH-GM5"
		xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
        CAT1_EnterATMode();
        
        FLEXCOMM2_SendStr("AT+SN?\r\n");
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, 300);
        char *s = strstr((char *)g_Cat1RxBuffer,"+SN:");
        if(s){
            strtok(s,"\r\n");
            strncpy(g_sys_flash_para.SN, s+4, sizeof(g_sys_flash_para.SN));
        }
        
        FLEXCOMM2_SendStr("AT+IMEI?\r\n");
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, 300);
        s = strstr((char *)g_Cat1RxBuffer,"+IMEI:");
        if(s){
            strtok(s,"\r\n");
            strncpy(g_sys_flash_para.IMEI, s+6, sizeof(g_sys_flash_para.IMEI));
        }
        
        FLEXCOMM2_SendStr("AT+ICCID?\r\n");
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, 300);
        s = strstr((char *)g_Cat1RxBuffer,"+ICCID:");
        if(s){
            strtok(s,"\r\n");
            strncpy(g_sys_flash_para.ICCID, s+7, sizeof(g_sys_flash_para.ICCID));
        }
        while(CAT1_SendCmd("AT+SOCKALK?\r\n" ,"Connected", 1000) == false){
            vTaskDelay(1000);
        }
        CAT1_SendCmd("AT+ENTM\r\n" ,"OK", 200);//进入透传模式
        
        //自注册设备
        g_Cat1RxCnt = 0;
        memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
        memset(g_Cat1TxBuffer, 0, sizeof(g_Cat1TxBuffer));
        char jsonData[128] = {0};
        snprintf(jsonData,sizeof(jsonData),"{\"sn\":\"%s\",\"title\":\"%s\"}",g_sys_flash_para.SN,g_sys_flash_para.SN);
        snprintf((char *)g_Cat1TxBuffer, sizeof(g_Cat1TxBuffer),
                           "POST http://api.heclouds.com/register_de?register_code=v3LzB6dSMS8xYIpm HTTP/1.1\r\n"
                          "User-Agent: Fiddler\r\n"
                          "Host: api.heclouds.com\r\n"
                          "Content-Length:%d\r\n\r\n%s",strlen(jsonData),jsonData);
        FLEXCOMM2_SendStr((char *)g_Cat1TxBuffer);
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, 300);
        char *json_string = strstr((char *)g_Cat1RxBuffer,"{");
        if(json_string){
            cJSON *pJson = cJSON_Parse(json_string);
            if(NULL == pJson) {
                return;
            }
            // get string from json
            cJSON * pSub = cJSON_GetObjectItem(pJson, "errno");
            if(pSub->valueint == 0){
                pSub = cJSON_GetObjectItem(pJson, "data");
                cJSON* item;
                item = cJSON_GetObjectItem(pSub,"device_id");
                if(item->valuestring){
                    memset(g_sys_flash_para.device_id, 0, sizeof(g_sys_flash_para.device_id));
                    strcpy(g_sys_flash_para.device_id,item->valuestring);
                }
                item = cJSON_GetObjectItem(pSub,"key");
                if(item->valuestring){
                    memset(g_sys_flash_para.key, 0, sizeof(g_sys_flash_para.key));
                    strcpy(g_sys_flash_para.key,item->valuestring);
                }
            }
            cJSON_Delete(pJson);
        }
        //注册设备成功, 将设备id保存到flash
        g_sys_flash_para.SelfRegisterFlag = 0xAA;
		Flash_SavePara();

        CAT1_EnterATMode();
        
        CAT1_SendCmd("AT+SOCKAEN=ON\r\n" ,"OK", 200);
        CAT1_SendCmd("AT+SOCKA=TCP,183.230.40.50,80\r\n" ,"OK", 1000);
        CAT1_SendCmd("AT+S\r\n" ,"OK", 200);
        PWR_CAT1_OFF;
	}
}


/* 将数据通过CAT1模块上传到OneNet*/
void CAT1_UploadSampleData(void)
{
    uint8_t xReturn = pdFALSE;
    uint32_t sid = 0;
    uint32_t len = 0;
    uint8_t  retry = 0;

	PWR_CAT1_OFF;
	vTaskDelay(100);
    PWR_CAT1_ON;//开机
	//wait "WH-GM5"
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
	
	CAT1_EnterATMode();
    
	//配置SOCKA,上传采样数据******************************************
    
    FLEXCOMM2_SendStr("AT+SOCKA?\r\n");
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, 300);
    if(strstr((char *)g_Cat1RxBuffer,"183.230.40.40") == NULL)
    {
        CAT1_SendCmd("AT+SOCKAEN=ON\r\n" ,"OK", 200);
        CAT1_SendCmd("AT+SOCKA=TCP,183.230.40.40,1811\r\n" ,"OK", 1000);
        CAT1_SendCmd("AT+S\r\n" ,"OK", 200);
        //AT+S会重启模块,在此处等待模块发送"WH-GM5"
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
        CAT1_EnterATMode();
    }
    while(CAT1_SendCmd("AT+SOCKALK?\r\n" ,"Connected", 1000) == false){//等待连接服务器成功
        vTaskDelay(1000);
    }
    CAT1_SendCmd("AT+ENTM\r\n" ,"OK", 200);//进入透传模式
	
    //登录OneNet系统
	char login[32] = {0};
	sprintf(login,"*%s#%s#server*",PRODUCT_ID,g_sys_flash_para.SN);
	CAT1_SendCmd(login, "OK", 3000);

NEXT_SID:
    
    memset(g_commTxBuf, 0, FLEXCOMM_BUFF_LEN);
    len = PacketUploadSampleData(g_commTxBuf, sid);
    USART_WriteBlocking(FLEXCOMM2_PERIPHERAL, g_commTxBuf, len);
    xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, 10000);//等待服务器回复数据,超时时间10S
    if(pdTRUE == xReturn){
        sid ++;
        if(sid < g_sys_para.sampPacksByWifiCat1){//还有数据包未发完
            goto NEXT_SID;
        }
    }else if(retry < 3){//超时,且重试次数小于3
        retry++;
        goto NEXT_SID;
    }
	
    //采样数据包发送完成后,还需要发送当前状态到服务器
    memset(g_commTxBuf, 0, FLEXCOMM_BUFF_LEN);
    PacketBatteryInfo(g_commTxBuf);
    CAT1_SendCmd((char *)g_commTxBuf, "OK", 10000);
    PWR_CAT1_OFF;//关机
    xTaskNotify(ADC_TaskHandle, EVT_ENTER_SLEEP, eSetBits);
}



void CAT1_AppTask(void)
{
	uint8_t xReturn = pdFALSE;
    CAT1_SelfRegister();
	DEBUG_PRINTF("CAT1_AppTask Running\r\n");
	while(1)
	{
		/*wait task notify*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
		if ( pdTRUE == xReturn)
		{
			if(cat1_event == EVT_UPLOAD_SAMPLE)//采样完成,将采样数据上传
            {
                CAT1_CheckVersion();
                CAT1_UploadSampleData();
            }
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
    if( USART_GetStatusFlags(FLEXCOMM2_PERIPHERAL) & (kUSART_RxFifoNotEmptyFlag) )
    {
		/*读取数据*/
        ucTemp = USART_ReadByte(FLEXCOMM2_PERIPHERAL);
		g_Cat1StartRx = true;
        g_Cat1RxTimeCnt = 0;
		if(g_Cat1RxCnt < sizeof(g_Cat1RxBuffer)) {
			/* 将接受到的数据保存到数组*/
			g_Cat1RxBuffer[g_Cat1RxCnt++] = ucTemp;
		}else{
			g_Cat1RxCnt = 0;
        }
	}else if(USART_GetStatusFlags(FLEXCOMM2_PERIPHERAL) & kUSART_RxError){
		USART_ClearStatusFlags(FLEXCOMM2_PERIPHERAL, kUSART_RxError);
	}
	__DSB();
}

void FLEXCOMM2_TimeTick(void)
{
    if(g_Cat1StartRx )
    {
        g_Cat1RxTimeCnt++;
		if(g_Cat1RxTimeCnt >= 100) { //100ms未接受到数据,表示接受数据超时
			g_Cat1RxTimeCnt = 0;
			g_Cat1StartRx = false;
			xTaskNotify(CAT1_TaskHandle, EVT_UART_TIMTOUT, eSetBits);
        }
    }
}
