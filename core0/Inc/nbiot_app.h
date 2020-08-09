#ifndef __NBIOT_APP_H
#define __NBIOT_APP_H


#define NB_OK       (1 << 0)
#define NB_TIMTOUT  (1 << 1)


extern TaskHandle_t NB_TaskHandle;

void NB_AppTask(void);




#endif
