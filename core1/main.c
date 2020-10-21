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
volatile uint32_t g_msg;

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
    g_msg = MAILBOX_GetValue(MAILBOX, kMAILBOX_CM33_Core1);
    MAILBOX_ClearValueBits(MAILBOX, kMAILBOX_CM33_Core1, 0xffffffff);
    g_msg++;
    MAILBOX_SetValue(MAILBOX, kMAILBOX_CM33_Core0, g_msg);
}


void main()
{
	uint32_t startupData;
	mcmgr_status_t status;

	BOARD_InitBootClocks();
	BOARD_InitPins();
	
	/* Initialize Mailbox */
    MAILBOX_Init(MAILBOX);
	
	 /* Enable mailbox interrupt */
    NVIC_EnableIRQ(MAILBOX_IRQn);

    /* Let the other side know the application is up and running */
    MAILBOX_SetValue(MAILBOX, kMAILBOX_CM33_Core0, (uint32_t)START_EVENT);

    while (1)
    {
        __WFI();
    }
}





