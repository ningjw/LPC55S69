#include "mcmgr.h"
#include "EventRecorder.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "fsl_gpio.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_mailbox.h"
#include <stdio.h>

#define START_EVENT 1234

/* Pointer to shared variable by both cores, before changing of this variable the
   cores must first take Mailbox mutex, after changing the shared variable must
   retrun mutex */
volatile uint32_t *g_shared = NULL;

typedef struct  {
    uint16_t  len;
	uint16_t  spdData[1024];
}msg_t;

msg_t my_msg;

/*******************************************************************************
 * Code
 ******************************************************************************/
void MAILBOX_IRQHandler()
{
    uint32_t data = MAILBOX_GetValue(MAILBOX, kMAILBOX_CM33_Core1);
	
    MAILBOX_ClearValueBits(MAILBOX, kMAILBOX_CM33_Core1, 0xffffffff);
	
    __DSB();
}


void main()
{
	uint32_t startupData;
	mcmgr_status_t status;
	/* Initialize MCMGR, install generic event handlers */
 //   MCMGR_Init();
	
	BOARD_InitBootClocks();
	BOARD_InitPins();
	
	/* Initialize Mailbox */
    MAILBOX_Init(MAILBOX);
	
	 /* Enable mailbox interrupt */
    NVIC_EnableIRQ(MAILBOX_IRQn);

	while(1)
	{
		MAILBOX_SetValue(MAILBOX, kMAILBOX_CM33_Core0, (uint32_t)&my_msg);
		
		/* Get Mailbox mutex */
        while (MAILBOX_GetMutex(MAILBOX) == 0)
			
		/* The core1 has mutex, can change shared variable g_shared */
        my_msg.len = 0xAA;
		
		/* Set mutex to allow access other core to shared variable */
        MAILBOX_SetMutex(MAILBOX);
		
		/* Add several nop instructions to allow the opposite core to get the mutex */
		for(int i =0; i<100; i++ )
			__asm("nop");
	}
}





