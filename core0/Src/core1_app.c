#include "main.h"


extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;

/* For the flow control */
volatile bool g_secondary_core_started = false;

msg_t *spd_msg = NULL;        //定义结构体指针。

TaskHandle_t CORE1_TaskHandle = NULL;  /* LED任务句柄 */

void start_secondary_core(uint32_t sec_core_boot_addr)
{
    /* Boot source for Core 1 from flash */
    SYSCON->CPUCFG |= SYSCON_CPUCFG_CPU1ENABLE_MASK;
    SYSCON->CPBOOT = SYSCON_CPBOOT_CPBOOT(sec_core_boot_addr);

    int32_t temp = SYSCON->CPUCTRL;
    temp |= 0xc0c48000;
    SYSCON->CPUCTRL = temp | SYSCON_CPUCTRL_CPU1RSTEN_MASK | SYSCON_CPUCTRL_CPU1CLKEN_MASK;
    SYSCON->CPUCTRL = (temp | SYSCON_CPUCTRL_CPU1CLKEN_MASK) & (~SYSCON_CPUCTRL_CPU1RSTEN_MASK);
}

void stop_secondary_core(void)
{
    uint32_t temp = SYSCON->CPUCTRL;
    temp |= 0xc0c48000U;

    /* hold in reset and disable clock */
    SYSCON->CPUCTRL = (temp | SYSCON_CPUCTRL_CPU1RSTEN_MASK) & (~SYSCON_CPUCTRL_CPU1CLKEN_MASK);
}

void start_spd_caputer(void)
{
	MAILBOX_SetValue(MAILBOX, kMAILBOX_CM33_Core1, 1);
}

void stop_spd_caputer(void)
{
	MAILBOX_SetValue(MAILBOX, kMAILBOX_CM33_Core1, 2);
}

void MAILBOX_IRQHandler(void)
{
	spd_msg = (msg_t *)MAILBOX_GetValue(MAILBOX, kMAILBOX_CM33_Core0);
	MAILBOX_ClearValueBits(MAILBOX, kMAILBOX_CM33_Core0, 0xffffffff);
	DEBUG_PRINTF("rev core1 msg, len=%d, D[0]=%d, D[1]=%d \r\n",
				spd_msg->len,spd_msg->spdData[0],spd_msg->spdData[1]);
}

void CORE1_AppTask(void)
{
	/* Init Mailbox */
    MAILBOX_Init(MAILBOX);
	
    /* Enable mailbox interrupt */
    NVIC_EnableIRQ(MAILBOX_IRQn);
	
	/* Copy Secondary core application from FLASH to the target memory. */
    memcpy((void *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, CORE1_IMAGE_SIZE);
	
	/* Boot Secondary core application */
	start_secondary_core(CORE1_BOOT_ADDRESS);
	
	/* Wait for start and initialization of secondary core */
    while (spd_msg == NULL){
		vTaskDelay(10);
	}
	DEBUG_PRINTF("core1 start, spd_msg=0x%x\r\n",spd_msg);

	vTaskDelete(CORE1_TaskHandle);
}

