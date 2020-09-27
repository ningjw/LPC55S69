#include "board.h"
#include "fsl_iap.h"
#include "fsl_iap_ffr.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "EventRecorder.h"
#include "memory.h"
#include <stdlib.h>

#define CORE0_START_ADDR    0x00018000	   // core0代码起始地址
#define CORE0_DATA_ADDR     0x00060000     // core0升级数据地址

#define CORE1_START_ADDR    0x00008000     // core1代码起始地址
#define CORE1_DATA_ADDR     0x00050000     // core1升级数据地址

#define PARA_ADDR           0x00098000
#define PAGE_SIZE           0x200

#define BUFFER_LEN          128

typedef void (*iapFun)(void);
iapFun appMain; 


//该结构体定义了升级参数
typedef struct{
    uint32_t  firmCore0Update;//core0固件更新
	uint32_t  firmCore1Update;//core1固件更新
    uint32_t  firmCore0Size;  //core0固件总大小
	uint32_t  firmCore1Size;  //core1固件总大小
	uint32_t  firmCrc16;
}UpdatePara_t;
UpdatePara_t g_sys_para;


uint32_t inFlashBuf[BUFFER_LEN] = {0};
void Flash_SavePara(void)
{
	uint16_t i = 0;
	memcpy(&inFlashBuf[i++],&g_sys_para.firmCore0Update, 4);
	memcpy(&inFlashBuf[i++],&g_sys_para.firmCore1Update, 4);
	memcpy(&inFlashBuf[i++],&g_sys_para.firmCore0Size, 4);
	memcpy(&inFlashBuf[i++],&g_sys_para.firmCore1Size, 4);
	memcpy(&inFlashBuf[i++],&g_sys_para.firmCrc16, 4);


	memory_erase(PARA_ADDR,PAGE_SIZE);
	memory_write(PARA_ADDR,(uint8_t *)inFlashBuf, PAGE_SIZE);
}

void Flash_ReadPara(void)
{
	uint16_t i = 0;
	memory_read(PARA_ADDR, (uint8_t *)inFlashBuf, PAGE_SIZE);
	
	memcpy(&g_sys_para.firmCore0Update,&inFlashBuf[i++],4);
	memcpy(&g_sys_para.firmCore1Update,&inFlashBuf[i++],4);
	memcpy(&g_sys_para.firmCore0Size, &inFlashBuf[i++],4);
	memcpy(&g_sys_para.firmCore1Size, &inFlashBuf[i++],4);
	memcpy(&g_sys_para.firmCrc16,     &inFlashBuf[i++],4);
}

int main()
{
    flash_config_t flashInstance;
    static uint32_t status;
    uint32_t failedAddress, failedData;

    BOARD_InitPins();
    BOARD_BootClockFROHF96M();
	
	memory_init();
	
    /* 初始化EventRecorder并开启*/
	EventRecorderInitialize(EventRecordAll, 1U);
	EventRecorderStart();
	
    FLASH_Init(&flashInstance);
	
	//读取升级参数
	Flash_ReadPara();

	if (g_sys_para.firmCore0Update == true){//需要更新core0系统
		memory_copy(CORE0_START_ADDR, CORE0_DATA_ADDR, g_sys_para.firmCore0Size);
		g_sys_para.firmCore0Update = false;
	}
	
	if (g_sys_para.firmCore1Update == true){//需要更新core1系统
		memory_copy(CORE1_START_ADDR, CORE1_DATA_ADDR, g_sys_para.firmCore1Size);
		g_sys_para.firmCore1Update = false;
	}
	
//    printf("Jump to app\n");
	
	SCB->VTOR = CORE0_START_ADDR;
	
	__set_MSP(*(volatile uint32_t*)(CORE0_START_ADDR));
	
	appMain = (iapFun)*(volatile uint32_t*)(CORE0_START_ADDR+4);
	
	__ASM volatile ("cpsie i" : : : "memory");
	
	appMain();
	
    while (1)
    {
    }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
