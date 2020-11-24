#ifndef __NFC_APP_H
#define __NFC_APP_H

#define NFC_RXIDLE_OK    (1 << 0)
#define NFC_AWAKE_OK     (1 << 1)
#define NFC_SETPARA_OK   (1 << 2)
#define NFC_TGINIT_OK    (1 << 3)

extern TaskHandle_t NFC_TaskHandle;
void NFC_AppTask(void);
void FLEXCOMM5_TimeTick(void);

#endif

