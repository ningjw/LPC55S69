#ifndef __BLE_APP_H
#define __BLE_APP_H

#define EVT_OK       (1 << 0)
#define EVT_TIMTOUT  (1 << 1)


#define FLEXCOMM3_BUFF_LEN 1024


extern uint16_t g_flexcomm3RxCnt;
extern TaskHandle_t BLE_WIFI_TaskHandle ;  /* À¶ÑÀÈÎÎñ¾ä±ú */
extern uint8_t g_flexcomm3TxBuf[];
extern uint8_t g_flexcomm3Buf[];
extern uint8_t g_flexcomm3StartRx;
extern uint32_t  g_flexcomm3RxTimeCnt;

void BLE_WIFI_AppTask(void);
void FLEXCOMM3_SendStr(const char *str);


#endif
