#ifndef __NFC_APP_H
#define __NFC_APP_H

#define NFC_EVT_OK       (1 << 0)

extern TaskHandle_t NFC_TaskHandle;
void NFC_AppTask(void);
void FLEXCOMM5_TimeTick(void);

#endif

