/*
- TgInitAsTarget, to configure the PN532 as a target,
- TgGetData, to wait for data coming from the initiator,
- TgSetData, to respond to the initiator.
*/
#include "main.h"

TaskHandle_t NFC_TaskHandle = NULL;  /* LED任务句柄 */
uint8_t g_NfcTxBuffer[32] = {0};
uint8_t g_NfcRxBuffer[32] = {0};
uint8_t g_NfcAck[6] = {0x00,0x00,0xFF,0x00,0xFF,0x00};
uint16_t g_NfcRxCnt = 0;
uint8_t  g_NfcStartRx = false;
uint32_t g_NfcRxTimeCnt = 0;

uint32_t nfc_event = 0;

/***************************************************************************************
  * @brief   发送一个字符串
  * @input   base:选择端口; data:将要发送的数据
  * @return
***************************************************************************************/
void NFC_SendData(uint8_t *buff, uint8_t len)
{
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
	NFC_SendData(g_NfcTxBuffer, 24);
}


void NFC_AppTask(void)
{
	uint8_t xReturn = pdFALSE;
	
	while(1)
	{
		/*wait task notify*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &nfc_event, portMAX_DELAY);
        if ( xReturn && nfc_event == NFC_EVT_OK) {
			
		}
		//清空接受到的数据
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
	/*串口接收到数据*/
    if ( USART_GetStatusFlags(FLEXCOMM5_PERIPHERAL) & (kUSART_RxFifoNotEmptyFlag | kUSART_RxError) )
    {
		uint8_t ucTemp = USART_ReadByte(FLEXCOMM5_PERIPHERAL);
		g_NfcStartRx = true;
		if(g_NfcRxCnt < sizeof(g_NfcRxBuffer)) {
			/* 将接受到的数据保存到数组*/
			g_NfcRxBuffer[g_NfcRxCnt++] = ucTemp;
		}
	}
}


void FLEXCOMM5_TimeTick(void)
{
    if(g_NfcStartRx )
    {
        g_NfcRxTimeCnt++;
		if(g_NfcRxTimeCnt >= 10) { //10ms未接受到数据,表示接受数据超时
			g_NfcRxTimeCnt = 0;
			g_NfcStartRx = false;
			xTaskNotify(NFC_TaskHandle, NFC_EVT_OK, eSetBits);
        }
    }
}