#ifndef __CORE1_APP_H
#define __CORE1_APP_H

#define CORE1_BOOT_ADDRESS 0x20033000
	
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#define CORE1_IMAGE_SIZE  (uint32_t)&Image$$CORE1_REGION$$Length

typedef struct {  //定义结构体
	uint32_t flag;
	uint16_t len;
	uint16_t spdData[1024];
}msg_t;
extern msg_t *spd_msg;

void start_spd_caputer(void);
void stop_spd_caputer(void);
extern TaskHandle_t CORE1_TaskHandle;
void CORE1_AppTask(void);

#endif

