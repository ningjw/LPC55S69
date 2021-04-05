#ifndef __CAT1_APP_H
#define __CAT1_APP_H

#define USE_ONENET

#define CAT1_PWD  "usr.cn#"

#define CAT1_WAIT_TICK        10000
#define CAT1_RETRY_TIMES      5

#define DATA_SERVER_IP        "183.230.40.40"
#define DATA_SERVER_PORT      1811

//#define DATA_SERVER_IP        "120.197.216.227"
//#define DATA_SERVER_PORT      7003

#define REGISTER_SERVER_IP    "183.230.40.33"
#define REGISTER_SERVER_PORT  80

#define UPGRADE_SERVER_IP     "183.230.40.50"
#define UPGRADE_SERVER_PORT   80


#define CONNECTED     1
#define DISCONNECTED  0

#define LINKA_STATUS()          GPIO_PinRead(GPIO, BOARD_LINKA_PORT, BOARD_LINKA_PIN)

extern TaskHandle_t CAT1_TaskHandle;

void CAT1_AppTask(void);


#endif
