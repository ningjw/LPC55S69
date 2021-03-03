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
    g_Cat1RxCnt = 0;
    memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
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
nb_retry:
    g_Cat1RxCnt = 0;
	memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
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


/* CAT1-IoT ģ���ʼ�� */
void CAT1_Init()
{
    PWR_CAT1_ON;
	//wait "WH-GM5"
	xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
	
	CAT1_EnterATMode();
    
    CAT1_SendCmd("AT+CSQ\r\n" ,"OK", 300);
    CAT1_SendCmd("AT+UARTFL\r\n", "OK", 300);
	if(g_sys_flash_para.Cat1InitFlag != 0xAA)
	{
		CAT1_SendCmd("AT+E=OFF\r\n" ,"OK", 200);

		CAT1_SendCmd("AT+WKMOD=NET\r\n", "OK", 200);//�������

		CAT1_SendCmd("AT+SOCKAEN=ON\r\n" ,"OK", 200);
        CAT1_SendCmd("AT+SOCKBEN=OFF\r\n" ,"OK", 200);
        CAT1_SendCmd("AT+SOCKCEN=OFF\r\n" ,"OK", 200);
        CAT1_SendCmd("AT+SOCKDEN=OFF\r\n" ,"OK", 200);
		CAT1_SendCmd("AT+SOCKA=TCP,183.230.40.33,80\r\n" ,"OK", 1000);

        CAT1_SendCmd("AT+S\r\n" ,"OK", 200);
        //AT+S������ģ��,�ڴ˴��ȴ�ģ�鷢��"WH-GM5"
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
        CAT1_SendCmd("AT+ENTM\r\n" ,"OK", 200);//����͸��ģʽ
        
        //��ע���豸
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
        //ע���豸�ɹ�, ���豸id���浽flash
        g_sys_flash_para.Cat1InitFlag = 0xAA;
		Flash_SavePara();

        CAT1_EnterATMode();
//        CAT1_SendCmd("AT+SOCKAEN=ON\r\n" ,"OK", 200);
//		  CAT1_SendCmd("AT+SOCKA=TCP,183.230.40.40,1811\r\n" ,"OK", 1000);
        
//        CAT1_SendCmd("AT+SOCKBEN=OFF\r\n" ,"OK", 200);
//        CAT1_SendCmd("AT+SOCKB=TCP,183.230.40.50,80\r\n" ,"OK", 1000);
        
//        CAT1_SendCmd("AT+SOCKCEN=OFF\r\n" ,"OK", 200);
//        CAT1_SendCmd("AT+SOCKC=TCP,183.230.40.33,80\r\n" ,"OK", 1000);
        
//        CAT1_SendCmd("AT+SOCKDEN=OFF\r\n" ,"OK", 200);
//        CAT1_SendCmd("AT+SOCKD=TCP,120.197.216.227,7003\r\n" ,"OK", 1000);
	}
	
#if 0
	//����SOCKA,�ϴ���������******************************************
	CAT1_SendCmd("AT+SOCKAEN=ON\r\n" ,"OK", 200);
    CAT1_SendCmd("AT+SOCKA=TCP,183.230.40.40,1811\r\n" ,"OK", 1000);
    CAT1_SendCmd("AT+S\r\n" ,"OK", 200);
    //AT+S������ģ��,�ڴ˴��ȴ�ģ�鷢��"WH-GM5"
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
    CAT1_EnterATMode();
    while(CAT1_SendCmd("AT+SOCKALK?\r\n" ,"Connected", 1000) == false){//�ȴ����ӷ������ɹ�
        vTaskDelay(1000);
    }
	
    CAT1_SendCmd("AT+ENTM\r\n" ,"OK", 200);//����͸��ģʽ

	char login[32] = {0};
	sprintf(login,"*%s#%s#server*",PRODUCT_ID,g_sys_flash_para.SN);
	CAT1_SendCmd(login, "OK", 200);
	for(uint16_t i=0; i<512; i++){
		g_Cat1TxBuffer[i] = i+1;
	}

	USART_WriteBlocking(FLEXCOMM2_PERIPHERAL, g_Cat1TxBuffer, 512);
	return;
#endif
	
    //����SOCKA,�����������******************************************
	CAT1_SendCmd("AT+SOCKAEN=ON\r\n" ,"OK", 200);
    CAT1_SendCmd("AT+SOCKA=TCP,183.230.40.50,80\r\n" ,"OK", 1000);
    CAT1_SendCmd("AT+S\r\n" ,"OK", 200);
    //AT+S������ģ��,�ڴ˴��ȴ�ģ�鷢��"WH-GM5"
    xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
    CAT1_EnterATMode();
    while(CAT1_SendCmd("AT+SOCKALK?\r\n" ,"Connected", 1000) == false){//�ȴ����ӷ������ɹ�
        vTaskDelay(1000);
    }
    CAT1_SendCmd("AT+ENTM\r\n" ,"OK", 200);//����͸��ģʽ
    g_Cat1RxCnt = 0;
    memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
    memset(g_Cat1TxBuffer, 0, sizeof(g_Cat1TxBuffer));
    snprintf((char *)g_Cat1TxBuffer, sizeof(g_Cat1TxBuffer),
              "GET /ota/south/check?dev_id=%s&manuf=100&model=10001&type=2&version=V11&cdn=false HTTP/1.1\r\n",g_sys_flash_para.device_id);
    strcat((char *)g_Cat1TxBuffer,"Authorization:version=2018-10-31&res=products%2F388752&et=1929767259&method=sha1&sign=FdGIbibDkBdX6kN2MyPzkehd7iE\%3D\r\n");
    strcat((char *)g_Cat1TxBuffer,"host: ota.heclouds.com\r\n\r\n");
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
                memset(g_sys_flash_para.firmUpdateTargetV, 0, sizeof(g_sys_flash_para.firmUpdateTargetV));
                strcpy(g_sys_flash_para.firmUpdateTargetV,item->valuestring);
            }
            item = cJSON_GetObjectItem(pSub,"token");
            if(item->valuestring){
                memset(g_sys_flash_para.firmUpdateToken, 0, sizeof(g_sys_flash_para.firmUpdateToken));
                strcpy(g_sys_flash_para.firmUpdateToken,item->valuestring);
            }
            item = cJSON_GetObjectItem(pSub,"size");
            if(item->valueint){
                g_sys_flash_para.firmCore0Size = item->valueint;
                g_sys_flash_para.firmPacksCount = g_sys_flash_para.firmCore0Size/512 + (g_sys_flash_para.firmCore0Size%512?1:0);
				/* �����ļ���С������Ӧ��С�Ŀռ� */
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
    
    //��ȡ�̼�
    if(g_sys_flash_para.firmCore0Size){
		uint32_t one_packet_len = 256;
		uint32_t current_pack = 0;
        uint32_t app_data_addr = CORE0_DATA_ADDR;
GET_NEXT:
        g_Cat1RxCnt = 0;
        memset(g_Cat1RxBuffer, 0, sizeof(g_Cat1RxBuffer));
        memset(g_Cat1TxBuffer, 0, sizeof(g_Cat1TxBuffer));
        snprintf((char *)g_Cat1TxBuffer, sizeof(g_Cat1TxBuffer),
                  "GET /ota/south/download/%s HTTP/1.1\r\n"
                  "Range:bytes=%d-%d\r\n"
                  "host: ota.heclouds.com\r\n\r\n"
                  ,g_sys_flash_para.firmUpdateToken, current_pack*one_packet_len, (current_pack+1)*one_packet_len-1);
        FLEXCOMM2_SendStr((char *)g_Cat1TxBuffer);
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &cat1_event, portMAX_DELAY);
        
        g_sys_flash_para.firmCurrentAddr = app_data_addr+current_pack * one_packet_len;//
        current_pack++;
        //���ҵ�һ�γ���0xD 0xA 0xD 0xA��λ��
        char *data_ptr = strstr((char *)g_Cat1RxBuffer, "\r\n\r\n");
        FLASH_SaveAppData((uint8_t *)(data_ptr+4), g_sys_flash_para.firmCurrentAddr, one_packet_len);
        //�ж��Ƿ�Ϊ���һ����
		if(current_pack < g_sys_flash_para.firmPacksCount){
			goto GET_NEXT;
		}
		
		MD5_CTX md5_ctx;
		unsigned char md5_t[16];
		char md5_result[40] = {0};
		MD5_Init(&md5_ctx);
		MD5_Final(&md5_ctx, md5_t);
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

	OTA_ReportVersion(DEVICE_ID, SOFT_VERSION, ota_info.authorization);//�ϱ��汾��
	if(OTA_Check(DEVICE_ID, ota_info.authorization)){
		
	}
    PWR_CAT1_OFF;//�ػ�
}


/* NB-IOTģ���ʼ������ */
void CAT1_AppTask(void)
{
	uint8_t xReturn = pdFALSE;
    CAT1_Init();
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
    if( USART_GetStatusFlags(FLEXCOMM2_PERIPHERAL) & (kUSART_RxFifoNotEmptyFlag) )
    {
		/*��ȡ����*/
        ucTemp = USART_ReadByte(FLEXCOMM2_PERIPHERAL);
		g_Cat1StartRx = true;
        g_Cat1RxTimeCnt = 0;
		if(g_Cat1RxCnt < sizeof(g_Cat1RxBuffer)) {
			/* �����ܵ������ݱ��浽����*/
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
		if(g_Cat1RxTimeCnt >= 100) { //100msδ���ܵ�����,��ʾ�������ݳ�ʱ
			g_Cat1RxTimeCnt = 0;
			g_Cat1StartRx = false;
			xTaskNotify(CAT1_TaskHandle, EVT_UART_TIMTOUT, eSetBits);
        }
    }
}
