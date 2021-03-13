/*
- TgInitAsTarget, to configure the PN532 as a target,
- TgGetData, to wait for data coming from the initiator,
- TgSetData, to respond to the initiator.
*/
#include "main.h"
#ifdef CAT1_VERSION
TaskHandle_t NFC_TaskHandle = NULL;  /* LED������ */
uint8_t nfc_response[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
uint8_t nfc_init_down[] = {0x00 ,0x00 ,0xFF ,0x14 ,0xEC ,0xD5 ,0x8D ,0x16 ,0x11 ,0xD4 ,0x00 ,0x01,
         0xFE ,0x12 ,0x34 ,0x56 ,0x78 ,0x90 ,0x12 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0xEE ,0x00};

uint8_t g_NfcTxBuffer[256] = {0};
uint8_t g_NfcRxBuffer[256] = {0};
uint8_t* g_NfcRxData = NULL;
uint8_t  g_NfcRxDataLen = 0;
uint16_t g_NfcRxCnt = 0;
uint8_t  g_NfcStartRx = false;
uint32_t g_NfcRxTimeCnt = 0;
uint32_t nfc_event = 0;
uint8_t  nfc_step = 0;
/***************************************************************************************
  * @brief   ����һ���ַ���
  * @input   base:ѡ��˿�; data:��Ҫ���͵�����
  * @return
***************************************************************************************/
void NFC_SendData(uint8_t *buff, uint8_t len)
{
	g_NfcRxCnt = 0;
	memset(g_NfcRxBuffer, 0, sizeof(g_NfcRxBuffer));
	USART_WriteBlocking(FLEXCOMM5_PERIPHERAL, buff, len);
}

uint8_t NFC_CalcChecksum(uint8_t *buff, uint8_t len)
{
	uint8_t checksum = 0;
	uint32_t sum = 0;
	for(uint8_t i=0;i<len;i++){
		sum += buff[i];
	}
	checksum = 0x100 - (0xff & sum);
	return checksum;
}

void NFC_SendCommand(uint8_t *buff, uint8_t len)
{
	uint8_t i = 0;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0xFF;
	g_NfcTxBuffer[i++] = len+1;
	g_NfcTxBuffer[i++] = 0x100-(len+1);
	g_NfcTxBuffer[i++] = 0xD4;
	memcpy(g_NfcTxBuffer+i, buff, len);
	i += len;
	g_NfcTxBuffer[i++] = NFC_CalcChecksum(buff,len+1);
	g_NfcTxBuffer[i++] = 0x00;
	NFC_SendData(g_NfcTxBuffer, i);
}
void nfc_TgInitAsTarget(void)
{
	uint8_t i = 0;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0xFF;
	g_NfcTxBuffer[i++] = 0x2B;
	g_NfcTxBuffer[i++] = 0xD5;
	g_NfcTxBuffer[i++] = 0xD4;
	g_NfcTxBuffer[i++] = 0x8C;
	g_NfcTxBuffer[i++] = 0x02;
	g_NfcTxBuffer[i++] = 0x08;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0x12;
	g_NfcTxBuffer[i++] = 0x34;
	g_NfcTxBuffer[i++] = 0x56;
	g_NfcTxBuffer[i++] = 0x40;
	g_NfcTxBuffer[i++] = 0x01;
	g_NfcTxBuffer[i++] = 0xFE;
	g_NfcTxBuffer[i++] = 0x12;
	g_NfcTxBuffer[i++] = 0x34;
	g_NfcTxBuffer[i++] = 0x56;
	g_NfcTxBuffer[i++] = 0x78;
	g_NfcTxBuffer[i++] = 0x90;
	g_NfcTxBuffer[i++] = 0x12;
	g_NfcTxBuffer[i++] = 0xC0;
	g_NfcTxBuffer[i++] = 0xC1;
	g_NfcTxBuffer[i++] = 0xC2;
	g_NfcTxBuffer[i++] = 0xC3;
	g_NfcTxBuffer[i++] = 0xC4;
	g_NfcTxBuffer[i++] = 0xC5;
	g_NfcTxBuffer[i++] = 0xC6;
	g_NfcTxBuffer[i++] = 0xC7;
	g_NfcTxBuffer[i++] = 0x0F;
	g_NfcTxBuffer[i++] = 0xAB;
	g_NfcTxBuffer[i++] = 0x12;
	g_NfcTxBuffer[i++] = 0x34;
	g_NfcTxBuffer[i++] = 0x56;
	g_NfcTxBuffer[i++] = 0x78;
	g_NfcTxBuffer[i++] = 0x9A;
	g_NfcTxBuffer[i++] = 0xBC;
	g_NfcTxBuffer[i++] = 0xDE;
	g_NfcTxBuffer[i++] = 0xFF;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0x04;
	g_NfcTxBuffer[i++] = 0x12;
	g_NfcTxBuffer[i++] = 0x34;
	g_NfcTxBuffer[i++] = 0x56;
	g_NfcTxBuffer[i++] = 0x78;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0xD0;
	g_NfcTxBuffer[i++] = 0x00;
	NFC_SendData(g_NfcTxBuffer, i);
}
void nfc_setParameters(void)
{
	uint8_t i=0;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0xFF;
	g_NfcTxBuffer[i++] = 0x03;
	g_NfcTxBuffer[i++] = 0xFD;
	g_NfcTxBuffer[i++] = 0xD4;
	g_NfcTxBuffer[i++] = 0x12;
	g_NfcTxBuffer[i++] = 0x14;
	g_NfcTxBuffer[i++] = 0x06;
	g_NfcTxBuffer[i++] = 0x00;
	NFC_SendData(g_NfcTxBuffer, i);
}

void nfc_awake(void)
{
	uint8_t i = 0;
	g_NfcTxBuffer[i++] = 0x55;
	g_NfcTxBuffer[i++] = 0x55;
	for(uint8_t j=0; j<14; j++){
		g_NfcTxBuffer[i++] = 0x00;
	}
	
	g_NfcTxBuffer[i++] = 0xFF;
	g_NfcTxBuffer[i++] = 0x03;
	g_NfcTxBuffer[i++] = 0xFD;
	g_NfcTxBuffer[i++] = 0xD4;
	g_NfcTxBuffer[i++] = 0x14;
	g_NfcTxBuffer[i++] = 0x01;
	g_NfcTxBuffer[i++] = 0x17;
	g_NfcTxBuffer[i++] = 0x00;
	NFC_SendData(g_NfcTxBuffer, i);
}


void nfc_TgGetData(void)
{
	uint8_t i = 0;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0xFF;
	g_NfcTxBuffer[i++] = 0x02;
	g_NfcTxBuffer[i++] = 0xFE;
	g_NfcTxBuffer[i++] = 0xD4;
	g_NfcTxBuffer[i++] = 0x86;
	g_NfcTxBuffer[i++] = 0xA6;
	g_NfcTxBuffer[i++] = 0x00;
	NFC_SendData(g_NfcTxBuffer, i);
}

void nfc_TgSetData(uint8_t *buff, uint8_t len)
{
	uint8_t i = 0;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0x00;
	g_NfcTxBuffer[i++] = 0xFF;
	g_NfcTxBuffer[i++] = len+2;
	g_NfcTxBuffer[i++] = 0x100-(len+2);
	g_NfcTxBuffer[i++] = 0xD4;
	g_NfcTxBuffer[i++] = 0x8E;
	memcpy(g_NfcTxBuffer+i, buff, len);
	i += len;
	g_NfcTxBuffer[i++] = NFC_CalcChecksum(g_NfcTxBuffer+5,len+2);
	g_NfcTxBuffer[i++] = 0x00;
	NFC_SendData(g_NfcTxBuffer, i);
}

bool NFC_Parse(uint8_t *src,uint16_t len, uint8_t desiredCmd)
{
    bool result = false;
	uint16_t dataLen = 0;
	uint8_t *p = src;
	if(src[0] != 0x00 || src[1] != 0x00 || src[2] != 0xFF || len <= 6)
	{
		return false;
	}
	
	if(memcmp(src,nfc_response,6) == 0)
	{
		p = src + 6;
	}

    if(p[5]== 0xD5 && p[6] == desiredCmd){
        result = true;
    }
    
    return result;
}

void NFC_AppTask(void)
{
	uint8_t xReturn = pdFALSE;
	DEBUG_PRINTF("NFC_AppTask Running\r\n");
	while(1)
	{
        switch(nfc_step)
        {
        case 0:
            nfc_awake();//����ָ���NFC
            xTaskNotifyWait(pdFALSE, ULONG_MAX, &nfc_event, 3000);//�ȴ�NFCģ��ظ�
            if(NFC_Parse(g_NfcRxBuffer,g_NfcRxCnt,0x15))//�ж��Ƿ��ѳɹ�
            {
				//����NFC�ɹ�, ��ת��case 1
                nfc_step++;
            }else{
                //error
            }
            break;
        case 1:
            nfc_TgInitAsTarget();//��NFCģ���ʼ��ΪTarget
            xTaskNotifyWait(pdFALSE, ULONG_MAX, &nfc_event, portMAX_DELAY);//�ȴ�NFCģ��ظ�
            if(NFC_Parse(g_NfcRxBuffer,g_NfcRxCnt,0x8D))
            {
				//�ڷ���nfc_TgInitAsTargetָ��ʱ, nfc�Ѿ��ǿ�����״̬,ֱ����תcase2
                nfc_step++;
            }else{
                xTaskNotifyWait(pdFALSE, ULONG_MAX, &nfc_event, portMAX_DELAY);
				if(NFC_Parse(g_NfcRxBuffer,g_NfcRxCnt,0x8D))
				{
					//android����,��ִ�е��˴�
					nfc_step++;
				}
            }
            break;
        case 2:
            nfc_TgGetData();
            xTaskNotifyWait(pdFALSE, ULONG_MAX, &nfc_event, portMAX_DELAY);
            if(NFC_Parse(g_NfcRxBuffer,g_NfcRxCnt,0x87))
            {
                g_NfcRxDataLen = g_NfcRxBuffer[9] - 3;
                g_NfcRxData = g_NfcRxBuffer + 14;
                if(g_NfcRxDataLen > 0){//��ȡ���ֻ��˷��͵�����
                    nfc_step++;
                    break;
                }
            }
            vTaskDelay(100);//100ms���ٴλ�ȡ����
            break;
        case 3:
            if(g_NfcRxData)
            {
                uint8_t* sendBuf = ParseProtocol(g_NfcRxData);
                
                /* �Ƿ��������������ݰ� */
                if( g_sys_flash_para.firmCore0Update == BOOT_NEW_VERSION) {
                    //����������Nor Flash
                    Flash_SavePara();
                    //�ر������ж�,����λϵͳ
                    NVIC_SystemReset();
                }
                
                if( NULL != sendBuf )
                {
                    nfc_TgSetData(sendBuf,strlen((char *)sendBuf));
                    xTaskNotifyWait(pdFALSE, ULONG_MAX, &nfc_event, portMAX_DELAY);
                    if(NFC_Parse(g_NfcRxBuffer,g_NfcRxCnt,0x8F))
                    {
                        nfc_step = 2;//�����ȴ���ȡ����
                    }
                    free(sendBuf);
                    sendBuf = NULL;
                }
            }
            break;
        default:
            break;
        }
        
		//��ս��ܵ�������
        memset(g_NfcRxBuffer, 0, sizeof(g_NfcRxBuffer));
        g_NfcRxCnt = 0;
	}
}


/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void FLEXCOMM5_IRQHandler(void)
{
	/*���ڽ��յ�����*/
    if ( USART_GetStatusFlags(FLEXCOMM5_PERIPHERAL) & (kUSART_RxFifoNotEmptyFlag | kUSART_RxError) )
    {
		uint8_t ucTemp = USART_ReadByte(FLEXCOMM5_PERIPHERAL);
		g_NfcStartRx = true;
		if(g_NfcRxCnt < sizeof(g_NfcRxBuffer)) {
			/* �����ܵ������ݱ��浽����*/
			g_NfcRxBuffer[g_NfcRxCnt++] = ucTemp;
		}
	}
}


void FLEXCOMM5_TimeTick(void)
{
    if(g_NfcStartRx )
    {
        g_NfcRxTimeCnt++;
		if(g_NfcRxTimeCnt >= 10) { //10msδ���ܵ�����,��ʾ�������ݳ�ʱ
			g_NfcRxTimeCnt = 0;
			g_NfcStartRx = false;
			xTaskNotify(NFC_TaskHandle, NFC_RXIDLE_OK, eSetBits);
        }
    }
}

#endif
