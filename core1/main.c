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
	uint16_t len;
	uint16_t spdData[1024];
}msg_t;

msg_t msg;

void ctimer2_callback(uint32_t flags);
static ctimer_callback_t ctimer_callback[] = {ctimer2_callback};
uint8_t gCtimer100msFlag = 0 , gOverFlow = 0, flag = 0;
uint32_t first = 0 , second = 0;
uint32_t gDiffValue;
uint32_t gCaptureTime;


void ctimer2_callback(uint32_t flags) {

	second = CTIMER_GetTimerCountValue(CTIMER);
	if(second >= first) {
		gDiffValue = second - first;
		gOverFlow = 0;
	} else {
		gDiffValue = first - second;
		gOverFlow = 1; // overflow
	}
	CTIMER_Reset(CTIMER);
	gCaptureTime = gDiffValue / (CLOCK_GetFroHfFreq() / 1000);
	flag = 0;
	gCtimer100msFlag = 1;
}

/*******************************************************************************
 * Code
 ******************************************************************************/
void MAILBOX_IRQHandler()
{
	msg.flag = MAILBOX_GetValue(MAILBOX, kMAILBOX_CM33_Core1);
    
    MAILBOX_ClearValueBits(MAILBOX, kMAILBOX_CM33_Core1, 0xffffffff);
    
	if(msg.flag == 1){
		CTIMER_StartTimer(CTIMER);
		first = CTIMER_GetTimerCountValue(CTIMER);
		flag = 1;
	}else if(msg.flag == 2){
		CTIMER_StopTimer(CTIMER);
	}
	
	msg.len = 100;
	for(int i = 0; i<1024; i++){
		msg.spdData[i] = i;
	}
	MAILBOX_SetValue(MAILBOX, kMAILBOX_CM33_Core0, (uint32_t)&msg);
}


void main()
{
	uint32_t startupData;
	mcmgr_status_t status;
	ctimer_config_t config;
	
	BOARD_InitBootClocks();
	BOARD_InitPins();
	
	CTIMER_GetDefaultConfig(&config);
    config.input = kCTIMER_Capture_0;
    CTIMER_Init(CTIMER, &config);
    CTIMER_RegisterCallBack(CTIMER, &ctimer_callback[0], kCTIMER_SingleCallback);
    CTIMER_SetupCapture(CTIMER, kCTIMER_Capture_0, kCTIMER_Capture_BothEdge, true);
	
	/* Initialize Mailbox */
    MAILBOX_Init(MAILBOX);
	
	 /* Enable mailbox interrupt */
    NVIC_EnableIRQ(MAILBOX_IRQn);

    /* Let the other side know the application is up and running */
    MAILBOX_SetValue(MAILBOX, kMAILBOX_CM33_Core0, (uint32_t)START_EVENT);
	
	msg.flag = 0;
	
	
    while (1)
    {
		__WFI();
    }
}

#if 0

volatile uint32_t g_pwmPeriod   = 0U;
volatile uint32_t g_pulsePeriod = 0U;

/***************************************************************************************
  * @brief  定时器用于获取转速信号的周期(频率)
  * @input
  * @return
***************************************************************************************/
void CTIMER1_Callback(uint32_t flags)
{
	//清除输入捕获0的中断
	CTIMER_ClearStatusFlags(CTIMER1, kCTIMER_Capture0Flag);
	if(g_adc_set.spdCount < SPD_LEN){
		SpeedADC[g_adc_set.spdCount++] = CTIMER_GetTimerCountValue(CTIMER1);
	}
}

ctimer_callback_t CTIMER1_callback[] = {CTIMER1_Callback};
/*******************************************************************************
 * Code
 ******************************************************************************/
status_t CTIMER_GetPwmPeriodValue(uint32_t pwmFreqHz, uint8_t dutyCyclePercent, uint32_t timerClock_Hz)
{
    /* Calculate PWM period match value */
    g_pwmPeriod = (timerClock_Hz / pwmFreqHz) - 1;

    /* Calculate pulse width match value */
    if (dutyCyclePercent == 0)
    {
        g_pulsePeriod = g_pwmPeriod + 1;
    }
    else
    {
        g_pulsePeriod = (g_pwmPeriod * (100 - dutyCyclePercent)) / 100;
    }
    return kStatus_Success;
}
#endif



