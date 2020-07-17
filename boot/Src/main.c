#include "fsl_common.h"
#include "clock_config.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "EventRecorder.h"
#include "stdio.h"
#include "fsl_iap.h"

#define APP_DATA_ADDR     0x4F000       // 300k��λ��
#define APP_ADDR          0x4000		// APP������ʼ��ַ
#define APP_ADDR_INFO     0x3E00
#define SECTOR_SIZE       0x1000
#define PAGE_SIZE         0x200

typedef void (*iapFun)(void);
iapFun appMain; 
flash_config_t flashInstance;
uint32_t pflashBlockBase            = 0;
uint32_t pflashTotalSize            = 0;
uint32_t pflashSectorSize           = 0;
uint32_t PflashPageSize             = 0;
uint8_t sector_buf[4096] = {0};
uint8_t page_buf[512] = {0};

//�ýṹ�嶨������������
typedef struct{
    uint32_t  firmUpdate;    //�̼�����
    uint32_t firmSizeTotal; //�̼��ܴ�С
}UpdatePara_t;
UpdatePara_t UpdatePara;

void MSR_MSP(uint32_t addr)
{
	__ASM volatile("MSR MSP, r0"); 
	__ASM volatile("BX r14");
}


void main(void)
{
	static uint32_t status;
//	BOARD_BootClockRUN();
	BOARD_BootClockFROHF96M();
	BOARD_InitPins();
	BOARD_InitPeripherals();
	
	/* ��ʼ��EventRecorder������*/
	EventRecorderInitialize(EventRecordAll, 1U);
	EventRecorderStart();

	FLASH_Init(&flashInstance);

	status = FLASH_Erase(&flashInstance, APP_ADDR_INFO, PflashPageSize, kFLASH_ApiEraseKey);


    status = FLASH_VerifyErase(&flashInstance, APP_ADDR_INFO, PflashPageSize);

	
	//��ȡ��������
	UpdatePara.firmUpdate = *(volatile uint32_t *)(APP_ADDR_INFO);
	UpdatePara.firmSizeTotal = *(volatile uint32_t *)(APP_ADDR_INFO+4);
	if (UpdatePara.firmUpdate == true){//��Ҫ����ϵͳ
	
		for(int i=0; i<=UpdatePara.firmSizeTotal/SECTOR_SIZE; i++){
			FLASH_Erase(&flashInstance, APP_ADDR+i*SECTOR_SIZE,SECTOR_SIZE, kFLASH_ApiEraseKey);
			FLASH_Program(&flashInstance,APP_ADDR+i*SECTOR_SIZE, (void *)(APP_DATA_ADDR+i*SECTOR_SIZE) ,SECTOR_SIZE);
		}
		
		//������flash�еı�ʶλ
		memcpy(page_buf, (void*)(APP_ADDR-PAGE_SIZE), PAGE_SIZE);
		memset(page_buf+PAGE_SIZE-8, 0, 8);
		FLASH_Program(&flashInstance, APP_ADDR-PAGE_SIZE, sector_buf, sizeof(sector_buf));
	}

	printf("Jump to app\n");
	//�����ж�������
	SCB->VTOR = APP_ADDR;
	appMain = (iapFun)*(volatile uint32_t*)(APP_ADDR+4);
	MSR_MSP(*(volatile uint32_t*)APP_ADDR);
	appMain();
}
