#ifndef __BLE_APP_H
#define __BLE_APP_H

#define EVT_OK       (1 << 0)
#define EVT_TIMTOUT  (1 << 1)


#define FLEXCOMM3_BUFF_LEN 1024
#define BLE_WIFI_STATUS()             GPIO_PinRead(GPIO, BOARD_BT_STATUS_PORT, BOARD_BT_STATUS_PIN)


extern uint16_t g_puart2TxCnt;
extern uint16_t g_flexcomm3RxCnt;
extern TaskHandle_t BLE_TaskHandle ;  /* ���������� */
extern uint8_t g_flexcomm3TxBuf[];
extern uint8_t g_flexcomm3Buf[];
extern uint8_t g_flexcomm3StartRx;
extern uint32_t  g_flexcomm3RxTimeCnt;

void BLE_AppTask(void);
void FLEXCOMM3_SendStr(const char *str);


#endif
