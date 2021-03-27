#ifndef __CAT1_APP_H
#define __CAT1_APP_H



#define CAT1_WAIT_TICK        10000
#define CAT1_RETRY_TIMES      5

#define DATA_SERVER_IP        "183.230.40.40"
#define DATA_SERVER_PORT      1811

#define REGISTER_SERVER_IP    "183.230.40.33"
#define REGISTER_SERVER_PORT  80

#define UPGRADE_SERVER_IP     "183.230.40.50"
#define UPGRADE_SERVER_PORT   80


extern TaskHandle_t CAT1_TaskHandle;

void CAT1_AppTask(void);


#endif
