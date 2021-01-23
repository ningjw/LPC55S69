#ifndef __CAT1_APP_H
#define __CAT1_APP_H


#define NB_OK       (1 << 0)
#define NB_TIMTOUT  (1 << 1)




extern TaskHandle_t CAT1_TaskHandle;

void CAT1_AppTask(void);




#endif
