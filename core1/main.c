#include "mcmgr.h"
#include "EventRecorder.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "fsl_gpio.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_mailbox.h"
#include "fsl_ctimer.h"
#include <stdio.h>

#define START_EVENT 1234
#define CTIMER CTIMER1


typedef struct {  //定义结构体
	uint32_t flag;
	uint32_t len;
	uint32_t spdData[1024];
}msg_t;
msg_t msg;


void ctimer1_callback(uint32_t flags) 
{
	if(msg.len < 1024){
		msg.spdData[msg.len++] = CTIMER_GetTimerCountValue(CTIMER);
		CTIMER_Reset(CTIMER);
	}
}
ctimer_callback_t ctimer_callback[1] = {ctimer1_callback};

/*******************************************************************************
 * Code
 ******************************************************************************/
void MAILBOX_IRQHandler()
{
	msg.flag = MAILBOX_GetValue(MAILBOX, kMAILBOX_CM33_Core1);
    
    MAILBOX_ClearValueBits(MAILBOX, kMAILBOX_CM33_Core1, 0xffffffff);
    
	if(msg.flag == 1){//开始采集信号
		msg.len = 0;
		CTIMER_Reset(CTIMER);
		CTIMER_StartTimer(CTIMER);
	}else if(msg.flag == 2){//结束采集信号
		CTIMER_StopTimer(CTIMER);
	}
}


void main()
{
	uint32_t startupData;
	mcmgr_status_t status;
	ctimer_config_t config;
	
	BOARD_InitBootClocks();
	BOARD_InitPins();
	
    config.input = kCTIMER_Capture_0;
	config.prescale = 15;
	config.mode = kCTIMER_TimerMode;
    CTIMER_Init(CTIMER, &config);
    CTIMER_RegisterCallBack(CTIMER, &ctimer_callback[0], kCTIMER_SingleCallback);
    CTIMER_SetupCapture(CTIMER, kCTIMER_Capture_0, kCTIMER_Capture_BothEdge, true);
	
	/* Initialize Mailbox */
    MAILBOX_Init(MAILBOX);
	
	 /* Enable mailbox interrupt */
    NVIC_EnableIRQ(MAILBOX_IRQn);
	
	msg.flag = 0xFFFFFFFF;
    /* Let the other side know the application is up and running */
    MAILBOX_SetValue(MAILBOX, kMAILBOX_CM33_Core0, (uint32_t)&msg);
	
    while (1)
    {
		__WFI();
    }
}

