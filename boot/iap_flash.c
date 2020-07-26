#include "board.h"
#include "fsl_iap.h"
#include "fsl_iap_ffr.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "EventRecorder.h"
#include "memory.h"
#define APP_DATA_ADDR     0x28000       // 32k+128k的位置
#define APP_START_ADDR    0x8000		// APP代码起始地址
#define APP_ADDR_INFO     0x7E00

#define SECTOR_SIZE       0x1000
#define PAGE_SIZE         0x200

typedef void (*iapFun)(void);
iapFun appMain; 

//该结构体定义了升级参数
typedef struct{
    uint32_t  firmUpdate;    //固件更新
    uint32_t  firmSizeTotal; //固件总大小
}UpdatePara_t;
UpdatePara_t UpdatePara;

static void verify_status(status_t status);
static void error_trap();
////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
#define BUFFER_LEN 512 / 4
uint32_t s_buffer_rbc[BUFFER_LEN];
const uint32_t s_buffer[BUFFER_LEN] = {1, 2, 3, 4};

void printf(const char *__res, ...)
{

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
	
    status = FLASH_Erase(&flashInstance, APP_ADDR_INFO, flashInstance.PFlashPageSize, kFLASH_ApiEraseKey);
    verify_status(status);

    /* Start programming specified flash region */
    printf("Calling FLASH_Program() API...\r\n");
    status = FLASH_Program(&flashInstance, APP_ADDR_INFO, (uint8_t *)s_buffer, sizeof(s_buffer));
    verify_status(status);

    /* Verify programming by reading back from flash directly */
    for (uint32_t i = 0; i < BUFFER_LEN; i++)
    {
        s_buffer_rbc[i] = *(volatile uint32_t *)(APP_ADDR_INFO + i * 4);
        if (s_buffer_rbc[i] != s_buffer[i])
        {
            error_trap();
        }
    }

	//读取升级参数
	UpdatePara.firmUpdate = *(volatile uint32_t *)(APP_ADDR_INFO);
	UpdatePara.firmSizeTotal = *(volatile uint32_t *)(APP_ADDR_INFO+4);
	if (UpdatePara.firmUpdate == true){//需要更新系统
	
//		for(int i=0; i<=UpdatePara.firmSizeTotal/SECTOR_SIZE; i++){
//			FLASH_Erase(&flashInstance, APP_START_ADDR+i*SECTOR_SIZE,SECTOR_SIZE, kFLASH_ApiEraseKey);
//			FLASH_Program(&flashInstance,APP_START_ADDR+i*SECTOR_SIZE, (void *)(APP_DATA_ADDR+i*SECTOR_SIZE) ,SECTOR_SIZE);
//		}
		
//		//将擦除flash中的标识位
//		memcpy(page_buf, (void*)(APP_START_ADDR-PAGE_SIZE), PAGE_SIZE);
//		memset(page_buf+PAGE_SIZE-8, 0, 8);
//		FLASH_Program(&flashInstance, APP_START_ADDR-PAGE_SIZE, sector_buf, sizeof(sector_buf));
	}
    printf("Jump to app\n");
	
	//在flash执行一次写操作后,才能以指针的方式操作flash
	memset((uint8_t *)s_buffer_rbc, 0xFF, PAGE_SIZE);
	memory_read(APP_START_ADDR, (uint8_t *)s_buffer_rbc, PAGE_SIZE);
	status = FLASH_Program(&flashInstance, APP_START_ADDR, (uint8_t *)s_buffer_rbc, PAGE_SIZE);
    verify_status(status);
	
	SCB->VTOR = APP_START_ADDR;
	
//	MSR_MSP(*(volatile uint32_t*)APP_START_ADDR);
	__set_MSP(*(volatile uint32_t*)(APP_START_ADDR));
	
	appMain = (iapFun)*(volatile uint32_t*)(APP_START_ADDR+4);
	
	__ASM volatile ("cpsie i" : : : "memory");
	
	appMain();
	
    while (1)
    {
    }
}

void verify_status(status_t status)
{
    char *tipString = "Unknown status";
    switch (status)
    {
        case kStatus_Success:
            tipString = "Done.";
            break;
        case kStatus_InvalidArgument:
            tipString = "Invalid argument.";
            break;
        case kStatus_FLASH_AlignmentError:
            tipString = "Alignment Error.";
            break;
        case kStatus_FLASH_AccessError:
            tipString = "Flash Access Error.";
            break;
        case kStatus_FLASH_CommandNotSupported:
            tipString = "This API is not supported in current target.";
            break;
        default:
            break;
    }
    printf("%s\r\n\r\n", tipString);
}

/*
 * @brief Gets called when an error occurs.
 */
void error_trap(void)
{
    printf("\r\n\r\n\r\n\t---- HALTED DUE TO FLASH ERROR! ----");
    while (1)
    {
    }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
