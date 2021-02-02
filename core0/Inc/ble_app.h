#ifndef __BLE_APP_H
#define __BLE_APP_H



extern uint16_t g_flexcomm3RxCnt;
extern TaskHandle_t BLE_WIFI_TaskHandle ;  /* ���������� */
extern uint8_t g_commTxBuf[];
extern uint8_t g_flexcomm3Buf[];
extern uint8_t g_flexcomm3StartRx;
extern uint32_t  g_flexcomm3RxTimeCnt;

void BLE_WIFI_AppTask(void);
void FLEXCOMM3_SendStr(const char *str);


#endif
