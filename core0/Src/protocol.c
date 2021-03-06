#include "main.h"

extern uint8_t s_nor_program_buffer[];
extern AdcInfoTotal adcInfoTotal;
extern AdcInfo adcInfo;


rtc_datetime_t sampTime;

uint16_t ble_wait_time = 8;

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
uint16_t CRC16(uint8_t *data,uint32_t length)
{
    uint16_t result = 0xFFFF;
    uint32_t i,j;

    if(length!=0)
    {
        for(i=0; i<length; i++)
        {
            result^=(uint16_t)(data[i]);
            for(j=0; j<8; j++)
            {
                if((result&0x0001) != 0)
                {
                    result>>=1;
                    result^=0xA001;	//a001
                }
                else result>>=1;
            }
        }
    }
    return result;
}

/***************************************************************************************
  * @brief   处理消息id为1的消息, 该消息设置点检仪RTC时间
  * @input
  * @return
***************************************************************************************/
static char* SetTime(cJSON *pJson, cJSON * pSub)
{
    /*解析消息内容, 获取日期和时间*/
    pSub = cJSON_GetObjectItem(pJson, "Y");
    if (NULL != pSub) {
        sysTime.year = pSub->valueint;
    }else return NULL;

    pSub = cJSON_GetObjectItem(pJson, "Mon");
    if (NULL != pSub) {
        sysTime.month = pSub->valueint+1;
    }

    pSub = cJSON_GetObjectItem(pJson, "D");
    if (NULL != pSub) {
        sysTime.day = pSub->valueint;
    }


    pSub = cJSON_GetObjectItem(pJson, "H");
    if (NULL != pSub) {
        sysTime.hour = pSub->valueint;
    }

    pSub = cJSON_GetObjectItem(pJson, "Min");
    if (NULL != pSub) {
        sysTime.minute = pSub->valueint;
    }

    pSub = cJSON_GetObjectItem(pJson, "S");
    if (NULL != pSub) {
        sysTime.second = pSub->valueint;
    }

    /*设置日期和时间*/
    RTC_SetDatetime(RTC, &sysTime);

    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) return NULL;
    cJSON_AddNumberToObject(pJsonRoot, "Id", 1);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为2的消息, 该消息获取点检仪RTC时间
  * @input
  * @return
***************************************************************************************/
static char * GetTime(void)
{
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 2);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "Y", sysTime.year);
    cJSON_AddNumberToObject(pJsonRoot, "Mon", sysTime.month);
    cJSON_AddNumberToObject(pJsonRoot, "D", sysTime.day);
    cJSON_AddNumberToObject(pJsonRoot, "H", sysTime.hour);
    cJSON_AddNumberToObject(pJsonRoot, "Min", sysTime.minute);
    cJSON_AddNumberToObject(pJsonRoot, "S", sysTime.second);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为3的消息, 该消息为仪器自检
  * @input
  * @return
***************************************************************************************/
static char * CheckSelf(void)
{
    float voltageADS1271;
    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }

    //指示灯自检
    LED_CheckSelf();

    //红外测温模块自检
    g_sys_para.objTemp = TMP101_ReadTemp();

    //震动传感器电压
    while (ADC_READY == 0);  //wait ads1271 ready
    voltageADS1271 = ADS1271_ReadData() * g_sys_flash_para.bias * 1.0f / 0x800000;

    cJSON_AddNumberToObject(pJsonRoot, "Id", 3);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "AdcV", voltageADS1271);  //振动传感器偏置电压
    cJSON_AddNumberToObject(pJsonRoot, "Temp", g_sys_para.objTemp);         //温度传感器的温度
    cJSON_AddNumberToObject(pJsonRoot, "PwrV", g_sys_para.batVoltage);      //电池电压值
    cJSON_AddNumberToObject(pJsonRoot, "BatC", (int)g_sys_para.batRemainPercent);//电池电量
    cJSON_AddNumberToObject(pJsonRoot, "Flash",true);//文件系统是否OK
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为4的消息, 该消息为获取电量
  * @input
  * @return
***************************************************************************************/
static char * GetBatCapacity(void)
{
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 4);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "BatC", (int)g_sys_para.batRemainPercent);//电池电量
    cJSON_AddNumberToObject(pJsonRoot, "PwrV", g_sys_para.batVoltage);//电池电压值
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为5的消息, 该消息为获取版本号
  * @input
  * @return
***************************************************************************************/
static char * GetVersion(void)
{
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 5);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddStringToObject(pJsonRoot, "HV", HARD_VERSION);//硬件版本号
    cJSON_AddStringToObject(pJsonRoot, "SV", SOFT_VERSION);//软件版本号

    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为6的消息, 该消息为系统参数设置
  * @input
  * @return
***************************************************************************************/
static char * SetSysPara(cJSON *pJson, cJSON * pSub)
{
    /*解析消息内容,*/
    pSub = cJSON_GetObjectItem(pJson, "OffT");
    if (NULL != pSub)
        g_sys_flash_para.autoPwrOffIdleTime = pSub->valueint;//触发自动关机时间

    pSub = cJSON_GetObjectItem(pJson, "OffC");
    if (NULL != pSub)
        g_sys_flash_para.autoPwrOffCondition = pSub->valueint;//触发自动关机条件
    
    pSub = cJSON_GetObjectItem(pJson, "BatL");
    if (NULL != pSub)
        g_sys_flash_para.batAlarmValue = pSub->valueint;//电池电量报警值

    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 6);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为7的消息, 该消息为采集参数设置
  * @input
  * @return
***************************************************************************************/
static char * SetSamplePara(cJSON *pJson, cJSON * pSub)
{
    uint8_t sid = 0;
    /*解析消息内容,*/
    pSub = cJSON_GetObjectItem(pJson, "SId");
    sid = pSub->valueint;
    switch(pSub->valueint)
    {
    case 0:
        pSub = cJSON_GetObjectItem(pJson, "IP");
        if (NULL != pSub) {
            memset(g_sample_para.IDPath, 0, sizeof(g_sample_para.IDPath));
            strcpy(g_sample_para.IDPath, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pJson, "NP");
        if (NULL != pSub) {
            memset(g_sample_para.NamePath, 0, sizeof(g_sample_para.NamePath));
            strcpy(g_sample_para.NamePath, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pJson, "WT");
        if (NULL != pSub) {
            ble_wait_time = pSub->valueint;
        }
        break;
    case 1:
        pSub = cJSON_GetObjectItem(pJson, "SU");
        if (NULL != pSub) {
            memset(g_sample_para.SpeedUnits, 0, sizeof(g_sample_para.SpeedUnits));
            strcpy(g_sample_para.SpeedUnits, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pJson, "PU");
        if (NULL != pSub) {
            memset(g_sample_para.ProcessUnits, 0, sizeof(g_sample_para.ProcessUnits));
            strcpy(g_sample_para.ProcessUnits, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pJson, "DT");
        if (NULL != pSub) {
            g_sample_para.DetectType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "ST");
        if (NULL != pSub) {
            g_sample_para.Senstivity = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pJson, "ZD");
        if (NULL != pSub) {
            g_sample_para.Zerodrift = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pJson, "ET");
        if (NULL != pSub) {
            g_sample_para.EUType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "EU");
        if (NULL != pSub) {
            memset(g_sample_para.EU, 0, sizeof(g_sample_para.EU));
            strcpy(g_sample_para.EU, pSub->valuestring);
        }
        pSub = cJSON_GetObjectItem(pJson, "W");
        if (NULL != pSub) {
            g_sample_para.WindowsType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "SF");
        if (NULL != pSub) {
            g_sample_para.StartFrequency = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "EF");
        if (NULL != pSub) {
            g_sample_para.EndFrequency = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "SR");
        if (NULL != pSub) {
            g_sample_para.SampleRate = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "L");
        if (NULL != pSub) {
            g_sample_para.Lines = pSub->valueint;
        }
#if 0
        pSub = cJSON_GetObjectItem(pJson, "B");
        if (NULL != pSub) {
            g_sys_flash_para.bias = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pJson, "RV");
        if (NULL != pSub) {
            g_sys_flash_para.refV = pSub->valuedouble;
        }
#endif
        //计算采样点数
        g_sample_para.sampNumber = 2.56 * g_sample_para.Lines * g_sample_para.Averages * (1 - g_sample_para.AverageOverlap)
                                + 2.56 * g_sample_para.Lines * g_sample_para.AverageOverlap;
        break;
    case 2:
        pSub = cJSON_GetObjectItem(pJson, "A");
        if (NULL != pSub) {
            g_sample_para.Averages = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "OL");
        if (NULL != pSub) {
            g_sample_para.AverageOverlap = pSub->valuedouble;
        }
        pSub = cJSON_GetObjectItem(pJson, "AT");
        if (NULL != pSub) {
            g_sample_para.AverageType = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "EFL");
        if (NULL != pSub) {
            g_sample_para.EnvFilterLow = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "EFH");
        if (NULL != pSub) {
            g_sample_para.EnvFilterHigh = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "IM");
        if (NULL != pSub) {
            g_sample_para.IncludeMeasurements = pSub->valueint;
        }
        pSub = cJSON_GetObjectItem(pJson, "interval");
        if (NULL != pSub) {
            g_sample_para.sampleInterval = pSub->valueint;
        }
        //计算采样点数
        g_sample_para.sampNumber = 2.56 * g_sample_para.Lines * g_sample_para.Averages * (1 - g_sample_para.AverageOverlap)
                                + 2.56 * g_sample_para.Lines * g_sample_para.AverageOverlap;
        break;
    default:
        break;
    }

    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 7);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",sid);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为8的消息, 该消息为开始采样信号
  * @input
  * @return
***************************************************************************************/
static char * StartSample(cJSON *pJson, cJSON * pSub)
{
    pSub = cJSON_GetObjectItem(pJson, "SampleRate");
    if (NULL != pSub) {
        g_sample_para.SampleRate = pSub->valueint;
    }


    cJSON *pJsonReply = cJSON_CreateObject();
    if(NULL == pJsonReply) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonReply, "Id",  8);
    cJSON_AddNumberToObject(pJsonReply, "Sid", 0);
    char *sendBuf = cJSON_PrintUnformatted(pJsonReply);
    cJSON_Delete(pJsonReply);
#ifdef CAT1_VERSION
//	FLEXCOMM5_SendStr((char *)sendBuf);
#else
    FLEXCOMM3_SendStr((char *)sendBuf);
#endif
    free(sendBuf);
    sendBuf = NULL;
    sampTime = sysTime;
    /*start sample*/
    ADC_SampleStart(HAND_SAMPLE);

    /*wait task notify*/
    ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

    if(g_sample_para.sampNumber != 0) { //Android发送了中断采集命令
        cJSON *pJsonRoot = cJSON_CreateObject();
		char dataTime[15] = {0};
		memcpy(dataTime, adcInfo.AdcDataTime, sizeof(adcInfo.AdcDataTime));
        if(NULL == pJsonRoot) {
            return NULL;
        }
        cJSON_AddNumberToObject(pJsonRoot, "Id",  8);
        cJSON_AddNumberToObject(pJsonRoot, "Sid", 1);
        cJSON_AddStringToObject(pJsonRoot, "F", dataTime);
        cJSON_AddNumberToObject(pJsonRoot, "Kb", adcInfoTotal.freeOfKb);
#if defined(BLE_VERSION) || defined(CAT1_VERSION)
		cJSON_AddNumberToObject(pJsonRoot,"PK",  g_sys_para.sampPacksByBleNfc);
#else 
        cJSON_AddNumberToObject(pJsonRoot,"PK",  g_sys_para.sampPacksByWifiCat1);
#endif
        cJSON_AddNumberToObject(pJsonRoot, "V", g_sample_para.shkCount);
        cJSON_AddNumberToObject(pJsonRoot, "S", g_sample_para.spdCount);
		cJSON_AddNumberToObject(pJsonRoot, "SS", g_sys_para.spdStartSid);
        sendBuf = cJSON_PrintUnformatted(pJsonRoot);
        cJSON_Delete(pJsonRoot);
    }

    return sendBuf;
}


/***************************************************************************************
  * @brief   处理消息id为9的消息, 该消息为获取采样数据
  * @input
  * @return
***************************************************************************************/
char * GetSampleDataByBleNfc(cJSON *pJson, cJSON * pSub)
{
    uint32_t sid = 0, i = 0;
    uint32_t index = 0;
    uint32_t flag_get_all_data = 0;
    g_sys_para.sampPacksCnt = 0;
    cJSON *pJsonRoot = NULL;
    char *p_reply = NULL;
    /*解析消息内容,并打包需要回复的内容*/
    pSub = cJSON_GetObjectItem(pJson, "Sid");
    sid = pSub->valueint;
    if(sid == g_sys_para.sampPacksByBleNfc) 
    {
        flag_get_all_data = 1;
    }else{
		flag_get_all_data = 0;
	}

SEND_DATA:
    if(flag_get_all_data) {//连续获取所有的采样数据
        sid = g_sys_para.sampPacksCnt;
    }
	if(sid<=2){
		pJsonRoot = cJSON_CreateObject();
		if(NULL == pJsonRoot) {
			return NULL;
		}
		cJSON_AddNumberToObject(pJsonRoot, "Id", 9);
		cJSON_AddNumberToObject(pJsonRoot, "Sid",sid);
	}
    switch(sid)	{
    case 0:
        cJSON_AddNumberToObject(pJsonRoot, "WT", ble_wait_time);
        cJSON_AddStringToObject(pJsonRoot, "DP", g_sample_para.IDPath);//硬件版本号
        cJSON_AddStringToObject(pJsonRoot, "NP", g_sample_para.NamePath);//硬件版本号
        p_reply = cJSON_PrintUnformatted(pJsonRoot);
        break;
    case 1:
        cJSON_AddStringToObject(pJsonRoot, "SU", g_sample_para.SpeedUnits);
        cJSON_AddStringToObject(pJsonRoot, "PU", g_sample_para.ProcessUnits);
        cJSON_AddNumberToObject(pJsonRoot, "DT", g_sample_para.DetectType);
        cJSON_AddNumberToObject(pJsonRoot, "ST", g_sample_para.Senstivity);
        cJSON_AddNumberToObject(pJsonRoot, "ZD", g_sample_para.Zerodrift);
        cJSON_AddNumberToObject(pJsonRoot, "ET", g_sample_para.EUType);
        cJSON_AddStringToObject(pJsonRoot, "EU", g_sample_para.EU);
        cJSON_AddNumberToObject(pJsonRoot, "W", g_sample_para.WindowsType);
        cJSON_AddNumberToObject(pJsonRoot, "SF", g_sample_para.StartFrequency);
        cJSON_AddNumberToObject(pJsonRoot, "EF", g_sample_para.EndFrequency);
        cJSON_AddNumberToObject(pJsonRoot, "SR", g_sample_para.SampleRate);
        cJSON_AddNumberToObject(pJsonRoot, "L", g_sample_para.Lines);
#if 0
        cJSON_AddNumberToObject(pJsonRoot, "B", g_sample_para.bias);
        cJSON_AddNumberToObject(pJsonRoot, "RV", g_sample_para.refV);
#endif
        p_reply = cJSON_PrintUnformatted(pJsonRoot);
        break;
    case 2:
        cJSON_AddNumberToObject(pJsonRoot, "A", g_sample_para.Averages);
        cJSON_AddNumberToObject(pJsonRoot, "OL", g_sample_para.AverageOverlap);
        cJSON_AddNumberToObject(pJsonRoot, "AT", g_sample_para.AverageType);
        cJSON_AddNumberToObject(pJsonRoot, "EFL", g_sample_para.EnvFilterLow);
        cJSON_AddNumberToObject(pJsonRoot, "EFH", g_sample_para.EnvFilterHigh);
        cJSON_AddNumberToObject(pJsonRoot, "IM", g_sample_para.IncludeMeasurements);
        cJSON_AddNumberToObject(pJsonRoot, "SP", g_sample_para.Speed);
        cJSON_AddNumberToObject(pJsonRoot, "P", g_sample_para.Process);
        cJSON_AddNumberToObject(pJsonRoot, "PL", g_sample_para.ProcessMin);
        cJSON_AddNumberToObject(pJsonRoot, "PH", g_sample_para.ProcessMax);
		cJSON_AddNumberToObject(pJsonRoot,"PK",  g_sys_para.sampPacksByBleNfc);
        cJSON_AddNumberToObject(pJsonRoot, "Y", sampTime.year);
        cJSON_AddNumberToObject(pJsonRoot, "M", sampTime.month);
        cJSON_AddNumberToObject(pJsonRoot, "D", sampTime.day);
        cJSON_AddNumberToObject(pJsonRoot, "H", sampTime.hour);
        cJSON_AddNumberToObject(pJsonRoot, "Min", sampTime.minute);
        cJSON_AddNumberToObject(pJsonRoot, "S", sampTime.second);
        p_reply = cJSON_PrintUnformatted(pJsonRoot);
        break;
    default:
    {
		memset(g_commTxBuf, 0, FLEXCOMM_BUFF_LEN);
		i = 0;
		g_commTxBuf[i++] = 0xE7;
		g_commTxBuf[i++] = 0xE8;
		g_commTxBuf[i++] = sid & 0xff;
		g_commTxBuf[i++] = (sid >> 8) & 0xff;

        if(sid < g_sys_para.shkPacksByBleNfc + 3)
        {
			index = ADC_NUM_BLE_NFC*(sid - 3);
            for(uint16_t j =0; j<ADC_NUM_BLE_NFC; j++)//每个数据占用3个byte;每包可以上传58个数据. 58*3=174
            {
				g_commTxBuf[i++] = ((uint32_t)ShakeADC[index] >> 0) & 0xff;
				g_commTxBuf[i++] = ((uint32_t)ShakeADC[index] >> 8) & 0xff;
				g_commTxBuf[i++] = ((uint32_t)ShakeADC[index] >> 16)& 0xff;
				index++;
			}
        }
        else if(sid < g_sys_para.sampPacksByBleNfc)
        {
            index = (sid - 3 - g_sys_para.shkPacksByBleNfc) * ADC_NUM_BLE_NFC;
            for(uint16_t j=0; j<ADC_NUM_BLE_NFC; j++)
            {
				g_commTxBuf[i++] = (spd_msg->spdData[index] >> 0) & 0xff;
				g_commTxBuf[i++] = (spd_msg->spdData[index] >> 8) & 0xff;
				g_commTxBuf[i++] = (spd_msg->spdData[index] >> 16)& 0xff;
				index++;
			}
        }
		g_commTxBuf[i++] = 0xEA;
		g_commTxBuf[i++] = 0xEB;
		g_commTxBuf[i++] = 0xEC;
		g_commTxBuf[i++] = 0xED;
        break;
    }
    }

	g_sys_para.sysIdleCount = 0;
	
#if 0
	int timeOut = 0;
	while(BLE_RTS_LEVEL() == 0){//BLE的RTS引脚为低电平,表示
		vTaskDelay(1);
		if(timeOut++ > 100)
			break;
	}
#endif
	
	if(sid <= 2)
    {
#ifdef CAT1_VERSION
//		FLEXCOMM5_SendStr((char *)p_reply);
#else
		FLEXCOMM3_SendStr((char *)p_reply);
#endif
		cJSON_Delete(pJsonRoot);
		free(p_reply);
		p_reply = NULL;
	}
    else
    {
#ifdef CAT1_VERSION
//		USART_WriteBlocking(FLEXCOMM5_PERIPHERAL, g_flexcomm5TxBuf, i);
#else
		USART_WriteBlocking(FLEXCOMM3_PERIPHERAL, g_commTxBuf, i);
#endif
	}
	
	DEBUG_PRINTF("\r\nsid = %d ; len = %d\r\n",sid, i);
	
	//获取所有的数据包
	g_sys_para.sampPacksCnt++;
	if(g_sys_para.sampPacksCnt < g_sys_para.sampPacksByBleNfc && flag_get_all_data) 
    {
		vTaskDelay(ble_wait_time);
		goto SEND_DATA;
	}
    return p_reply;
}


/***************************************************************************************
  * @brief   处理消息id为10的消息, 该消息为开始发送升级固件包
  * @input
  * @return
***************************************************************************************/
static char * StartUpgrade(cJSON *pJson, cJSON * pSub)
{
    /* 开始升级固件后, 初始化一些必要的变量*/
    g_sys_flash_para.firmCore0Update = false;
    g_sys_flash_para.firmPacksCount = 0;
    g_sys_flash_para.firmSizeCurrent = 0;
    g_sys_flash_para.firmCurrentAddr = CORE0_DATA_ADDR;
	
    /*解析消息内容,*/
    pSub = cJSON_GetObjectItem(pJson, "Packs");
    if (NULL != pSub)
        g_sys_flash_para.firmPacksTotal = pSub->valueint;

    pSub = cJSON_GetObjectItem(pJson, "Size");
    if (NULL != pSub)
        g_sys_flash_para.firmCore0Size = pSub->valueint;

    pSub = cJSON_GetObjectItem(pJson, "CRC16");
    if (NULL != pSub)
        g_sys_flash_para.firmCrc16 = pSub->valueint;

	pSub = cJSON_GetObjectItem(pJson, "Core");
    if (NULL != pSub)
        g_sys_flash_para.firmCoreIndex = pSub->valueint;
	
	g_sys_flash_para.firmPacksTotal = g_sys_flash_para.firmCore0Size % 1000 ? 
	                            g_sys_flash_para.firmCore0Size/1000 + 1: 
	                            g_sys_flash_para.firmCore0Size/1000;
	g_sys_flash_para.firmCoreIndex = 0;
    g_sys_flash_para.firmPacksCount = 0;
    g_sys_flash_para.firmSizeCurrent = 0;
    g_sys_flash_para.firmCore0Update = false;

    /* 按照文件大小擦除对应大小的空间 */
	memory_erase(CORE0_DATA_ADDR, g_sys_flash_para.firmCore0Size);

    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id",  10);
    cJSON_AddNumberToObject(pJsonRoot, "Sid", 0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
#if defined( WIFI_VERSION) || defined(BLE_VERSION)
    g_sys_para.BleWifiLedStatus = BLE_WIFI_UPDATE;
    g_flexcomm3StartRx = true;//开始超市检测,5s中未接受到数据则超时
#endif
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为11的消息, 该消息为获取红外传感器温度
  * @input
  * @return
***************************************************************************************/
static char * GetObjTemp(void)
{
    //红外测温模块自检
    g_sys_para.objTemp = TMP101_ReadTemp();

    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 11);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "Temp", g_sys_para.objTemp);
//	cJSON_AddNumberToObject(pJsonRoot, "env", g_sys_para.envTemp);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为12的消息, 该消息为终止采样
  * @input
  * @return
***************************************************************************************/
static char* StopSample(void)
{
    //如果此时正在采集数据, 该代码会触发采集完成信号
    g_sample_para.sampNumber = 0;
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 12);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}
/***************************************************************************************
  * @brief   处理消息id为13的消息, 该消息为获取manage文件内容
  * @input
  * @return
***************************************************************************************/
static char* GetManageInfo(cJSON *pJson, cJSON * pSub)
{
    char   *fileStr;
    int sid = 0, num = 0, si = 0, len = 0;

    /*解析消息内容,*/
    pSub = cJSON_GetObjectItem(pJson, "S");
    if(pSub != NULL) {
        si = pSub->valueint;
    } else {
        si = 0;
    }

    pSub = cJSON_GetObjectItem(pJson, "N");
    if(pSub != NULL) {
        num = pSub->valueint;
    } else {
        num = 1;
    }
    if(num > 10) num = 10;
    if(num < 1) num = 1;

    //申请内存用于保存文件内容
    len = num * 13 + 1;
    fileStr = malloc(len);
    memset(fileStr, 0U, len);

    W25Q128_ReadAdcInfo(si, num, fileStr);

    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 13);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",sid);
    cJSON_AddStringToObject(pJsonRoot, "Name", fileStr);
    cJSON_AddNumberToObject(pJsonRoot, "T",adcInfoTotal.totalAdcInfo);
    cJSON_AddNumberToObject(pJsonRoot, "Kb",adcInfoTotal.freeOfKb);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    free(fileStr);
    fileStr = NULL;
    return p_reply;
}


/***************************************************************************************
  * @brief   处理消息id为14的消息, 该消息为通过文件名获取采集数据
  * @input
  * @return
***************************************************************************************/
static char* GetSampleDataInFlash(cJSON *pJson, cJSON * pSub)
{
    extern AdcInfo adcInfo;
    char ret;
    char fileName[15] = {0};

    /*解析消息内容,*/
    pSub = cJSON_GetObjectItem(pJson, "fileName");
    if(pSub != NULL && strlen(pSub->valuestring) == 12) {
        strcpy(fileName,pSub->valuestring);
    }

    /*从flash读取文件*/
    ret = W25Q128_ReadAdcData(fileName);
    if(ret == true) {
		sampTime.year = (fileName[0] - '0')*10 + (fileName[1]-'0') + 2000;
		sampTime.month =(fileName[2] - '0')*10 + (fileName[3]-'0');
		sampTime.day =  (fileName[4] - '0')*10 + (fileName[5]-'0');
		sampTime.hour = (fileName[6] - '0')*10 + (fileName[7]-'0');
		sampTime.minute=(fileName[8] - '0')*10 + (fileName[9]-'0');
		sampTime.second=(fileName[10] - '0')*10 + (fileName[11]-'0');

#if defined(BLE_VERSION) || defined(CAT1_VERSION)
        //计算通过蓝牙(NFC)发送震动信号需要多少个包
        g_sys_para.shkPacksByBleNfc = (g_sample_para.shkCount / ADC_NUM_BLE_NFC) +  (g_sample_para.shkCount % ADC_NUM_BLE_NFC ? 1 : 0);
        
        //计算通过蓝牙(NFC)发送转速信号需要多少个包
        g_sys_para.spdPacksByBleNfc = (g_sample_para.spdCount / ADC_NUM_BLE_NFC) +  (g_sample_para.spdCount % ADC_NUM_BLE_NFC ? 1 : 0);
        
        //计算将所有数据通过通过蓝牙(NFC)发送需要多少个包
        g_sys_para.sampPacksByBleNfc = g_sys_para.shkPacksByBleNfc + g_sys_para.spdPacksByBleNfc + 3;//wifi需要加上3个采样参数包
        
        //转速信号从哪个sid开始发送
        g_sys_para.spdStartSid = g_sys_para.shkPacksByBleNfc + 3;//需要加上3个采样参数包
#elif defined(WIFI_VERSION)
        //计算通过WIFI发送震动信号需要多少个包
        g_sys_para.shkPacksByWifiCat1 = (g_sample_para.shkCount / ADC_NUM_WIFI_CAT1) +  (g_sample_para.shkCount % ADC_NUM_WIFI_CAT1 ? 1 : 0);
        
        //计算通过WIFI发送转速信号需要多少个包
        g_sys_para.spdPacksByWifiCat1 = (g_sample_para.spdCount / ADC_NUM_WIFI_CAT1) +  (g_sample_para.spdCount % ADC_NUM_WIFI_CAT1 ? 1 : 0);
        
        //计算将所有数据通过WIFI上传需要多少个包
        g_sys_para.sampPacksByBleNfc = g_sys_para.shkPacksByWifiCat1 + g_sys_para.spdPacksByWifiCat1 + 1;//wifi需要加上1个采样参数包
        
        //转速信号从哪个sid开始发送
        g_sys_para.spdStartSid = g_sys_para.shkPacksByBleNfc + 1;//需要加上1个采样参数包
#endif
	}else{
		g_sys_para.sampPacksByBleNfc = 0;
        g_sys_para.sampPacksByWifiCat1 = 0;
		g_sample_para.shkCount = 0;
        g_sample_para.spdCount = 0;
		spd_msg->len = 0;
	}
    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) return NULL;

    cJSON_AddNumberToObject(pJsonRoot, "Id", 14);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
#if defined(BLE_VERSION) || defined(CAT1_VERSION)
	cJSON_AddNumberToObject(pJsonRoot,"PK",  g_sys_para.sampPacksByBleNfc);
#else
    cJSON_AddNumberToObject(pJsonRoot,"PK",  g_sys_para.sampPacksByWifiCat1);
#endif
    cJSON_AddNumberToObject(pJsonRoot, "V", g_sample_para.shkCount);
    cJSON_AddNumberToObject(pJsonRoot, "S", g_sample_para.spdCount);
    cJSON_AddNumberToObject(pJsonRoot, "SS", g_sys_para.spdStartSid);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);

    return p_reply;
}


/***************************************************************************************
  * @brief   处理消息id为15的消息, 该消息为擦除flash中保存的所有采样数据
  * @input
  * @return
***************************************************************************************/
char *EraseAdcDataInFlash(void)
{
	SPI_Flash_Erase_Sector(0);

    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 15);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddBoolToObject(pJsonRoot, "Status",true);
    uint32_t free = (SPI_FLASH_SIZE_BYTE - ADC_DATA_ADDR)/1024;
    cJSON_AddNumberToObject(pJsonRoot, "Kb", free);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}


#ifndef CAT1_VERSION
/***************************************************************************************
  * @brief   处理消息id为16的消息, 该消息为设置电池电量
  * @input
  * @return
***************************************************************************************/
static char *SetBatCapacity(cJSON *pJson, cJSON * pSub)
{
    uint8_t batC = 0;
    uint8_t batM = 0;
    pSub = cJSON_GetObjectItem(pJson, "BatC");
    if (NULL != pSub) {
        batC = pSub->valueint;
        if( batC != 0) {
            if(batC > 100) batC = 100;
            LTC2942_SetAC(batC / 100 * 0xFFFF);
        }
    }

    pSub = cJSON_GetObjectItem(pJson, "BatM");
    if (NULL != pSub) {
        batM = pSub->valueint;
        if(batM != 0) {
            if(batM == 1) {
                LTC2942_SetPrescaler(LTC2942_PSCM_1);
            } else if(batM == 2) {
                LTC2942_SetPrescaler(LTC2942_PSCM_2);
            } else if(batM == 4) {
                LTC2942_SetPrescaler(LTC2942_PSCM_4);
            } else if(batM == 8) {
                LTC2942_SetPrescaler(LTC2942_PSCM_8);
            } else if(batM == 16) {
                LTC2942_SetPrescaler(LTC2942_PSCM_16);
            } else if(batM == 32) {
                LTC2942_SetPrescaler(LTC2942_PSCM_32);
            } else if(batM == 64) {
                LTC2942_SetPrescaler(LTC2942_PSCM_64);
            } else if(batM == 128) {
                LTC2942_SetPrescaler(LTC2942_PSCM_128);
            }
        }
    }

    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) return NULL;
    cJSON_AddNumberToObject(pJsonRoot, "Id", 16);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

#endif

/***************************************************************************************
  * @brief   处理消息id为17的消息, 该消息为通过wifi设置采集参数
  * @input
  * @return
***************************************************************************************/
static char * SetSampleParaByWifi(cJSON *pJson, cJSON * pSub)
{
    pSub = cJSON_GetObjectItem(pJson, "IP");
    if (NULL != pSub) {
        memset(g_sample_para.IDPath, 0, sizeof(g_sample_para.IDPath));
        strcpy(g_sample_para.IDPath, pSub->valuestring);
    }
    pSub = cJSON_GetObjectItem(pJson, "NP");
    if (NULL != pSub) {
        memset(g_sample_para.NamePath, 0, sizeof(g_sample_para.NamePath));
        strcpy(g_sample_para.NamePath, pSub->valuestring);
    }
    pSub = cJSON_GetObjectItem(pJson, "WT");
    if (NULL != pSub) {
        ble_wait_time = pSub->valueint;
    }

    pSub = cJSON_GetObjectItem(pJson, "SU");
    if (NULL != pSub) {
        memset(g_sample_para.SpeedUnits, 0, sizeof(g_sample_para.SpeedUnits));
        strcpy(g_sample_para.SpeedUnits, pSub->valuestring);
    }
    pSub = cJSON_GetObjectItem(pJson, "PU");
    if (NULL != pSub) {
        memset(g_sample_para.ProcessUnits, 0, sizeof(g_sample_para.ProcessUnits));
        strcpy(g_sample_para.ProcessUnits, pSub->valuestring);
    }
    pSub = cJSON_GetObjectItem(pJson, "DT");
    if (NULL != pSub) {
        g_sample_para.DetectType = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "ST");
    if (NULL != pSub) {
        g_sample_para.Senstivity = pSub->valuedouble;
    }
    pSub = cJSON_GetObjectItem(pJson, "ZD");
    if (NULL != pSub) {
        g_sample_para.Zerodrift = pSub->valuedouble;
    }
    pSub = cJSON_GetObjectItem(pJson, "ET");
    if (NULL != pSub) {
        g_sample_para.EUType = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "EU");
    if (NULL != pSub) {
        memset(g_sample_para.EU, 0, sizeof(g_sample_para.EU));
        strcpy(g_sample_para.EU, pSub->valuestring);
    }
    pSub = cJSON_GetObjectItem(pJson, "W");
    if (NULL != pSub) {
        g_sample_para.WindowsType = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "SF");
    if (NULL != pSub) {
        g_sample_para.StartFrequency = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "EF");
    if (NULL != pSub) {
        g_sample_para.EndFrequency = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "SR");
    if (NULL != pSub) {
        g_sample_para.SampleRate = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "L");
    if (NULL != pSub) {
        g_sample_para.Lines = pSub->valueint;
    }
#if 0
    pSub = cJSON_GetObjectItem(pJson, "B");
    if (NULL != pSub) {
        g_sys_flash_para.bias = pSub->valuedouble;
    }
    pSub = cJSON_GetObjectItem(pJson, "RV");
    if (NULL != pSub) {
        g_sys_flash_para.refV = pSub->valuedouble;
    }
#endif
    pSub = cJSON_GetObjectItem(pJson, "A");
    if (NULL != pSub) {
        g_sample_para.Averages = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "OL");
    if (NULL != pSub) {
        g_sample_para.AverageOverlap = pSub->valuedouble;
    }
    pSub = cJSON_GetObjectItem(pJson, "AT");
    if (NULL != pSub) {
        g_sample_para.AverageType = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "EFL");
    if (NULL != pSub) {
        g_sample_para.EnvFilterLow = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "EFH");
    if (NULL != pSub) {
        g_sample_para.EnvFilterHigh = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "IM");
    if (NULL != pSub) {
        g_sample_para.IncludeMeasurements = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "interval");
    if (NULL != pSub) {
        g_sample_para.sampleInterval = pSub->valueint;
    }
    //计算采样点数
    g_sample_para.sampNumber = 2.56 * g_sample_para.Lines * g_sample_para.Averages * (1 - g_sample_para.AverageOverlap)
                            + 2.56 * g_sample_para.Lines * g_sample_para.AverageOverlap;

    /*制作cjson格式的回复消息*/
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 17);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

/***************************************************************************************
  * @brief   处理消息id为18的消息, 该消息为通过wifi获取采样数据
  * @input
  * @return
***************************************************************************************/
char * GetSampleDataByWifi(cJSON *pJson, cJSON * pSub)
{
    uint32_t sid = 0, i =0;
    uint32_t index = 0;
    uint32_t flag_get_all_data = false;
    g_sys_para.sampPacksCnt = 0;
    cJSON *pJsonRoot = NULL;
    char *p_reply = NULL;
    /*解析消息内容,并打包需要回复的内容*/
    pSub = cJSON_GetObjectItem(pJson, "Sid");
    sid = pSub->valueint;
    if(sid == g_sys_para.sampPacksByWifiCat1){
        flag_get_all_data = true;
    }

SEND_DATA:
    if(flag_get_all_data) {
        sid = g_sys_para.sampPacksCnt;
    }
    
    switch(sid)	{
    case 0:
		pJsonRoot = cJSON_CreateObject();
		if(NULL == pJsonRoot) {
			return NULL;
		}
		cJSON_AddNumberToObject(pJsonRoot, "Id", 18);
		cJSON_AddNumberToObject(pJsonRoot, "Sid",sid);
        cJSON_AddNumberToObject(pJsonRoot, "WT", ble_wait_time);
        cJSON_AddStringToObject(pJsonRoot, "DP", g_sample_para.IDPath);//硬件版本号
        cJSON_AddStringToObject(pJsonRoot, "NP", g_sample_para.NamePath);//硬件版本号
        cJSON_AddStringToObject(pJsonRoot, "SU", g_sample_para.SpeedUnits);
        cJSON_AddStringToObject(pJsonRoot, "PU", g_sample_para.ProcessUnits);
        cJSON_AddNumberToObject(pJsonRoot, "DT", g_sample_para.DetectType);
        cJSON_AddNumberToObject(pJsonRoot, "ST", g_sample_para.Senstivity);
        cJSON_AddNumberToObject(pJsonRoot, "ZD", g_sample_para.Zerodrift);
        cJSON_AddNumberToObject(pJsonRoot, "ET", g_sample_para.EUType);
        cJSON_AddStringToObject(pJsonRoot, "EU", g_sample_para.EU);
        cJSON_AddNumberToObject(pJsonRoot, "W", g_sample_para.WindowsType);
        cJSON_AddNumberToObject(pJsonRoot, "SF", g_sample_para.StartFrequency);
        cJSON_AddNumberToObject(pJsonRoot, "EF", g_sample_para.EndFrequency);
        cJSON_AddNumberToObject(pJsonRoot, "SR", g_sample_para.SampleRate);
        cJSON_AddNumberToObject(pJsonRoot, "L", g_sample_para.Lines);
        cJSON_AddNumberToObject(pJsonRoot, "B", g_sys_flash_para.bias);
        cJSON_AddNumberToObject(pJsonRoot, "RV", g_sys_flash_para.refV);
        cJSON_AddNumberToObject(pJsonRoot, "A", g_sample_para.Averages);
        cJSON_AddNumberToObject(pJsonRoot, "OL", g_sample_para.AverageOverlap);
        cJSON_AddNumberToObject(pJsonRoot, "AT", g_sample_para.AverageType);
        cJSON_AddNumberToObject(pJsonRoot, "EFL", g_sample_para.EnvFilterLow);
        cJSON_AddNumberToObject(pJsonRoot, "EFH", g_sample_para.EnvFilterHigh);
        cJSON_AddNumberToObject(pJsonRoot, "IM", g_sample_para.IncludeMeasurements);
        cJSON_AddNumberToObject(pJsonRoot, "SP", g_sample_para.Speed);
        cJSON_AddNumberToObject(pJsonRoot, "P", g_sample_para.Process);
        cJSON_AddNumberToObject(pJsonRoot, "PL", g_sample_para.ProcessMin);
        cJSON_AddNumberToObject(pJsonRoot, "PH", g_sample_para.ProcessMax);
        cJSON_AddNumberToObject(pJsonRoot,"PK",  g_sys_para.sampPacksByWifiCat1);
        cJSON_AddNumberToObject(pJsonRoot, "Y", sampTime.year);
        cJSON_AddNumberToObject(pJsonRoot, "M", sampTime.month);
        cJSON_AddNumberToObject(pJsonRoot, "D", sampTime.day);
        cJSON_AddNumberToObject(pJsonRoot, "H", sampTime.hour);
        cJSON_AddNumberToObject(pJsonRoot, "Min", sampTime.minute);
        cJSON_AddNumberToObject(pJsonRoot, "S", sampTime.second);
        p_reply = cJSON_PrintUnformatted(pJsonRoot);
        break;
    default:
        memset(g_commTxBuf, 0, FLEXCOMM_BUFF_LEN);
		i = 0;
		g_commTxBuf[i++] = 0xE7;
		g_commTxBuf[i++] = 0xE8;
		g_commTxBuf[i++] = sid & 0xff;
		g_commTxBuf[i++] = (sid >> 8) & 0xff;
        if(sid-1 < g_sys_para.shkPacksByWifiCat1)
        {
			index = ADC_NUM_WIFI_CAT1 * (sid - 1);
			for(uint16_t j =0; j<ADC_NUM_WIFI_CAT1; j++){//每个数据占用3个byte;每包可以上传335个数据. 335*3=1005
				g_commTxBuf[i++] = ((uint32_t)ShakeADC[index] >> 0) & 0xff;
				g_commTxBuf[i++] = ((uint32_t)ShakeADC[index] >> 8) & 0xff;
				g_commTxBuf[i++] = ((uint32_t)ShakeADC[index] >> 16)& 0xff;
				index++;
			}
        }
        else if(sid < g_sys_para.sampPacksByWifiCat1)
        {
            index = (sid - 1 - g_sys_para.shkPacksByWifiCat1) * ADC_NUM_WIFI_CAT1;
            //每个数据占用3个byte;每包可以上传335个数据. 335*3=174
            for(uint16_t j =0; j<ADC_NUM_WIFI_CAT1; j++){//每个数据占用3个byte;每包可以上传335个数据. 335*3=1005
				g_commTxBuf[i++] = (spd_msg->spdData[index] >> 0) & 0xff;
				g_commTxBuf[i++] = (spd_msg->spdData[index] >> 8) & 0xff;
				g_commTxBuf[i++] = (spd_msg->spdData[index] >> 16)& 0xff;
				index++;
			}
        }
		g_commTxBuf[i++] = 0xEA;
		g_commTxBuf[i++] = 0xEB;
		g_commTxBuf[i++] = 0xEC;
		g_commTxBuf[i++] = 0xED;
        break;
    }
	g_sys_para.sysIdleCount = 0;
	printf("sid = %d\r\n",sid);
	
	if(sid == 0){
#ifdef CAT1_VERSION
//		FLEXCOMM5_SendStr((char *)p_reply);
#else
		FLEXCOMM3_SendStr((char *)p_reply);
#endif
		cJSON_Delete(pJsonRoot);
		free(p_reply);
		p_reply = NULL;
	}else{
#ifdef CAT1_VERSION
//		USART_WriteBlocking(FLEXCOMM5_PERIPHERAL, g_flexcomm5TxBuf, i);
#else
		USART_WriteBlocking(FLEXCOMM3_PERIPHERAL, g_commTxBuf, i);
#endif
	}
	
	//获取所有的数据包
	g_sys_para.sampPacksCnt++;
	if(g_sys_para.sampPacksCnt < g_sys_para.sampPacksByWifiCat1 && flag_get_all_data) {
		vTaskDelay(ble_wait_time);
		goto SEND_DATA;
	}
	
    return p_reply;
}

/***************************************************************************************
  * @brief   cat1上传采样数据是,通过该函数封装数据包
  * @input
  * @return
***************************************************************************************/
uint32_t PacketUploadSampleData(uint8_t *txBuf, uint32_t sid)
{
    uint32_t i = 0,index = 0;
    cJSON *pJsonRoot = NULL;
    char *p_reply = NULL;
    
    switch(sid)	{
    case 0:
		pJsonRoot = cJSON_CreateObject();
		if(NULL == pJsonRoot) {
			return 0;
		}
		cJSON_AddNumberToObject(pJsonRoot, "Id", 18);
		cJSON_AddNumberToObject(pJsonRoot, "Sid",sid);
        cJSON_AddNumberToObject(pJsonRoot, "WT", ble_wait_time);
        cJSON_AddStringToObject(pJsonRoot, "DP", g_sample_para.IDPath);//硬件版本号
        cJSON_AddStringToObject(pJsonRoot, "NP", g_sample_para.NamePath);//硬件版本号
        cJSON_AddStringToObject(pJsonRoot, "SU", g_sample_para.SpeedUnits);
        cJSON_AddStringToObject(pJsonRoot, "PU", g_sample_para.ProcessUnits);
        cJSON_AddNumberToObject(pJsonRoot, "DT", g_sample_para.DetectType);
        cJSON_AddNumberToObject(pJsonRoot, "ST", g_sample_para.Senstivity);
        cJSON_AddNumberToObject(pJsonRoot, "ZD", g_sample_para.Zerodrift);
        cJSON_AddNumberToObject(pJsonRoot, "ET", g_sample_para.EUType);
        cJSON_AddStringToObject(pJsonRoot, "EU", g_sample_para.EU);
        cJSON_AddNumberToObject(pJsonRoot, "W", g_sample_para.WindowsType);
        cJSON_AddNumberToObject(pJsonRoot, "SF", g_sample_para.StartFrequency);
        cJSON_AddNumberToObject(pJsonRoot, "EF", g_sample_para.EndFrequency);
        cJSON_AddNumberToObject(pJsonRoot, "SR", g_sample_para.SampleRate);
        cJSON_AddNumberToObject(pJsonRoot, "L", g_sample_para.Lines);
        cJSON_AddNumberToObject(pJsonRoot, "B", g_sys_flash_para.bias);
        cJSON_AddNumberToObject(pJsonRoot, "RV", g_sys_flash_para.refV);
        cJSON_AddNumberToObject(pJsonRoot, "A", g_sample_para.Averages);
        cJSON_AddNumberToObject(pJsonRoot, "OL", g_sample_para.AverageOverlap);
        cJSON_AddNumberToObject(pJsonRoot, "AT", g_sample_para.AverageType);
        cJSON_AddNumberToObject(pJsonRoot, "EFL", g_sample_para.EnvFilterLow);
        cJSON_AddNumberToObject(pJsonRoot, "EFH", g_sample_para.EnvFilterHigh);
        cJSON_AddNumberToObject(pJsonRoot, "IM", g_sample_para.IncludeMeasurements);
        cJSON_AddNumberToObject(pJsonRoot, "SP", g_sample_para.Speed);
        cJSON_AddNumberToObject(pJsonRoot, "P", g_sample_para.Process);
        cJSON_AddNumberToObject(pJsonRoot, "PL", g_sample_para.ProcessMin);
        cJSON_AddNumberToObject(pJsonRoot, "PH", g_sample_para.ProcessMax);
        cJSON_AddNumberToObject(pJsonRoot,"PK",  g_sys_para.sampPacksByWifiCat1);
        cJSON_AddNumberToObject(pJsonRoot, "Y", sampTime.year);
        cJSON_AddNumberToObject(pJsonRoot, "M", sampTime.month);
        cJSON_AddNumberToObject(pJsonRoot, "D", sampTime.day);
        cJSON_AddNumberToObject(pJsonRoot, "H", sampTime.hour);
        cJSON_AddNumberToObject(pJsonRoot, "Min", sampTime.minute);
        cJSON_AddNumberToObject(pJsonRoot, "S", sampTime.second);
        p_reply = cJSON_PrintUnformatted(pJsonRoot);
        
        i = strlen(p_reply);
        memcpy(txBuf, p_reply, i);
        if(p_reply){
            free(p_reply);
            p_reply = NULL;
        }
        break;
    default:
        memset(g_commTxBuf, 0, FLEXCOMM_BUFF_LEN);
		i = 0;
		txBuf[i++] = 0xE7;
		txBuf[i++] = 0xE8;
		txBuf[i++] = sid & 0xff;
		txBuf[i++] = (sid >> 8) & 0xff;
        if(sid-1 < g_sys_para.shkPacksByWifiCat1)
        {
			index = ADC_NUM_WIFI_CAT1 * (sid - 1);
			for(uint16_t j =0; j<ADC_NUM_WIFI_CAT1; j++){//每个数据占用3个byte;每包可以上传335个数据. 335*3=1005
				txBuf[i++] = ((uint32_t)ShakeADC[index] >> 0) & 0xff;
				txBuf[i++] = ((uint32_t)ShakeADC[index] >> 8) & 0xff;
				txBuf[i++] = ((uint32_t)ShakeADC[index] >> 16)& 0xff;
				index++;
			}
        }
        else if(sid < g_sys_para.sampPacksByWifiCat1)
        {
            index = (sid - 1 - g_sys_para.shkPacksByWifiCat1) * ADC_NUM_WIFI_CAT1;
            //每个数据占用3个byte;每包可以上传335个数据. 335*3=174
            for(uint16_t j =0; j<ADC_NUM_WIFI_CAT1; j++){//每个数据占用3个byte;每包可以上传335个数据. 335*3=1005
				txBuf[i++] = (spd_msg->spdData[index] >> 0) & 0xff;
				txBuf[i++] = (spd_msg->spdData[index] >> 8) & 0xff;
				txBuf[i++] = (spd_msg->spdData[index] >> 16)& 0xff;
				index++;
			}
        }
		txBuf[i++] = 0xEA;
		txBuf[i++] = 0xEB;
		txBuf[i++] = 0xEC;
		txBuf[i++] = 0xED;
        break;
    }
    return i;
}

char *GetIdentityInfoByNfc(cJSON *pJson, cJSON * pSub)
{
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 20);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",0);
    cJSON_AddNumberToObject(pJsonRoot, "IMEI", sysTime.year);
    cJSON_AddNumberToObject(pJsonRoot, "ICCID", sysTime.month);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    return p_reply;
}

uint32_t PacketBatteryInfo(uint8_t *txBuf)
{
    uint32_t len = 0;
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return len;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 23);
    cJSON_AddNumberToObject(pJsonRoot, "BatC", g_sys_para.batRemainPercent);
    cJSON_AddNumberToObject(pJsonRoot, "BatV", g_sys_para.batVoltage);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    len = strlen(p_reply);
    if(p_reply){
        free(p_reply);
        p_reply = NULL;
    }
    return len;
}

uint32_t PacketVersionInfo(uint8_t *txBuf)
{
    uint32_t len = 0;
    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return len;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 26);
    cJSON_AddStringToObject(pJsonRoot, "HV", HARD_VERSION);//硬件版本号
    cJSON_AddStringToObject(pJsonRoot, "SV", SOFT_VERSION);//软件版本号
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    len = strlen(p_reply);
    if(p_reply){
        free(p_reply);
        p_reply = NULL;
    }
    return len;
}


/***************************************************************************************
  * @brief   解析json数据包
  * @input
  * @return
***************************************************************************************/
uint8_t* ParseJson(char *pMsg)
{
    char *p_reply = NULL;

    cJSON *pJson = cJSON_Parse((char *)pMsg);
    if(NULL == pJson) {
        return NULL;
    }

    // get string from json
    cJSON * pSub = cJSON_GetObjectItem(pJson, "Id");
    if(NULL == pSub) {
        return NULL;
    }

    switch(pSub->valueint)
    {
    case 1:
        p_reply = SetTime(pJson, pSub);//设置日期
        break;
    case 2:
        p_reply = GetTime();//获取日期
        break;
    case 3:
        p_reply = CheckSelf();//控制自检
        break;
    case 4:
        p_reply = GetBatCapacity();//获取电量
        break;
    case 5:
        p_reply = GetVersion();//获取版本号
        break;
#ifndef CAT1_VERSION
    case 6:
        p_reply = SetSysPara(pJson, pSub);//系统参数设置
        break;
#endif
    case 7:
        p_reply = SetSamplePara(pJson, pSub);//采集参数设置
        break;
    case 8:
        p_reply = StartSample(pJson, pSub);//开始采样
        break;
    case 9:
        p_reply = GetSampleDataByBleNfc(pJson, pSub);//获取采样结果
        break;
    case 10:
        p_reply = StartUpgrade(pJson, pSub);//升级
        break;
    case 11:
        p_reply = GetObjTemp();//获取温度
        break;
    case 12:
        p_reply = StopSample();//停止采集
        break;
    case 13:
        p_reply = GetManageInfo(pJson, pSub);
        break;
    case 14:
        p_reply = GetSampleDataInFlash(pJson, pSub);//获取flash中的采样数据
        break;
    case 15:
        p_reply = EraseAdcDataInFlash();//擦除flash中的采样数据
        break;
#ifndef	CAT1_VERSION
    case 16:
        p_reply = SetBatCapacity(pJson, pSub);//校正电池容量
        break;

	case 17:
		p_reply = SetSampleParaByWifi(pJson, pSub);//通过wifi设置采样参数
		break;
	case 18:
		p_reply = GetSampleDataByWifi(pJson, pSub);//
		break;
	case 19:
		break;

    case 20:
        p_reply = GetIdentityInfoByNfc(pJson, pSub);
		break;
#endif
    default:break;
    }

    cJSON_Delete(pJson);

    return (uint8_t *)p_reply;
}



/***************************************************************************************
  * @brief   解析固件包
  * @input
  * @return
***************************************************************************************/
uint8_t*  ParseFirmPacket(uint8_t *pMsg)
{
    uint16_t crc = 0;
    uint8_t  err_code = 0;
	uint32_t app_data_addr = 0;
	
	if(g_sys_flash_para.firmCoreIndex == 1){
		app_data_addr = CORE1_DATA_ADDR;
	}else{
		app_data_addr = CORE0_DATA_ADDR;
	}
#if defined(BLE_VERSION) || defined(CAT1_VERSION)
    crc = CRC16(pMsg+4, FIRM_DATA_LEN_BLE_NFC);//自己计算出的CRC16
    if(pMsg[FIRM_TOTAL_LEN_BLE_NFC-2] != (uint8_t)crc || pMsg[FIRM_TOTAL_LEN_BLE_NFC-1] != (crc>>8)) {
        err_code = 1;
    } else {
        /* 包id */
        g_sys_flash_para.firmPacksCount = pMsg[2] | (pMsg[3]<<8);

        g_sys_flash_para.firmCurrentAddr = app_data_addr+g_sys_flash_para.firmPacksCount * FIRM_DATA_LEN_BLE_NFC;//
        DEBUG_PRINTF("\nADDR = 0x%x\n",g_sys_para.firmCurrentAddr);
        LPC55S69_FlashSaveData(pMsg+4, g_sys_flash_para.firmCurrentAddr, FIRM_DATA_LEN_BLE_NFC);
    }
#elif defined(WIFI_VERSION)
    crc = CRC16(pMsg+4, FIRM_DATA_LEN_WIFI_CAT1);//自己计算出的CRC16
    if(pMsg[FIRM_TOTAL_LEN_WIFI_CAT1-2] != (uint8_t)crc || pMsg[FIRM_TOTAL_LEN_WIFI_CAT1-1] != (crc>>8)) {
        err_code = 1;
    } else {
        /* 包id */
        g_sys_flash_para.firmPacksCount = pMsg[2] | (pMsg[3]<<8);

        g_sys_flash_para.firmCurrentAddr = app_data_addr+g_sys_flash_para.firmPacksCount * FIRM_DATA_LEN_WIFI_CAT1;//
        DEBUG_PRINTF("\nADDR = 0x%x\n",g_sys_para.firmCurrentAddr);
        LPC55S69_FlashSaveData(pMsg+4, g_sys_flash_para.firmCurrentAddr, FIRM_DATA_LEN_WIFI_CAT1);
    }
#endif
    /* 当前为最后一包,计算整个固件的crc16码 */
    if(g_sys_flash_para.firmPacksCount == g_sys_flash_para.firmPacksTotal - 1) {
		
        g_sys_para.BleWifiLedStatus = BLE_WIFI_CONNECT;
#if defined( WIFI_VERSION) || defined(BLE_VERSION)
        g_flexcomm3StartRx = false;
#endif
//		printf("升级文件:\r\n");
//
//		for(uint32_t i = 0;i<g_sys_para.firmCore0Size; i++){
//			if(i%16 == 0) printf("\n");
//			printf("%02X ",*(uint8_t *)(FlexSPI_AMBA_BASE + APP_START_SECTOR * SECTOR_SIZE+i));
//		}

        crc = CRC16((uint8_t *)app_data_addr, g_sys_flash_para.firmCore0Size);
        DEBUG_PRINTF("\nCRC=%d",crc);
        if(crc != g_sys_flash_para.firmCrc16) {
            g_sys_flash_para.firmCore0Update = false;
            err_code = 2;
        } else {
            DEBUG_PRINTF("\nCRC Verify OK\n");
            g_sys_flash_para.firmCore0Update = true;
        }
    }

    cJSON *pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot) {
        return NULL;
    }
    cJSON_AddNumberToObject(pJsonRoot, "Id", 10);
    cJSON_AddNumberToObject(pJsonRoot, "Sid",1);
    cJSON_AddNumberToObject(pJsonRoot, "P", g_sys_flash_para.firmPacksCount);
    cJSON_AddNumberToObject(pJsonRoot, "E", err_code);
    char *p_reply = cJSON_PrintUnformatted(pJsonRoot);
    cJSON_Delete(pJsonRoot);
    g_sys_flash_para.firmPacksCount++;
    return (uint8_t*)p_reply;
}


/***************************************************************************************
  * @brief   解析协议入口函数
  * @input
  * @return
***************************************************************************************/
uint8_t* ParseProtocol(uint8_t *pMsg)
{
    if(NULL == pMsg) {
        return NULL;
    }
	
    if(pMsg[0] == 0xE7 && pMsg[1] == 0xE7 ) { //为固件升级包
        return ParseFirmPacket(pMsg);
    } else { //为json数据包
		DEBUG_PRINTF("%s: rev wifi data:\r\n%s\r\n",__func__,pMsg);
        return ParseJson((char *)pMsg);
    }
}

