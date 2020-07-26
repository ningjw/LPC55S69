#include "main.h"

SysPara  g_sys_para;
ADC_Set  g_adc_set;

void main(void)
{
	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
	BOARD_BootClockRUN();
	BOARD_InitPins();
	BOARD_InitPeripherals();

	/* 初始化EventRecorder并开启*/
	EventRecorderInitialize(EventRecordAll, 1U);
	EventRecorderStart();
	printf("app start\n");
	
	/* Init output LED GPIO. */
    LED_RED_INIT(LOGIC_LED_OFF);
	LED_RED_INIT(1);
	
	/* Enter sleep mode. */
//	POWER_EnterDeepSleep(kPDRUNCFG_PD_FRO1M, 0x0, WAKEUP_UTICK, 0x0);
	
	    /* 创建LED_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )LED_AppTask,"LED_Task",256,NULL, 1,&LED_TaskHandle);
    
//    /* 创建Battery_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
//    xTaskCreate((TaskFunction_t )BAT_AppTask,"BAT_Task",1024,NULL, 2,&BAT_TaskHandle);

//    /* 创建BLE_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
//    xTaskCreate((TaskFunction_t )BLE_AppTask,"BLE_Task",1024,NULL, 3,&BLE_TaskHandle);

//    /* 创建ADC_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
//    xTaskCreate((TaskFunction_t )ADC_AppTask, "ADC_Task",1024,NULL, 4,&ADC_TaskHandle);
//	
//	/* 创建LPM_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
//    xTaskCreate((TaskFunction_t )LPM_AppTask, "ADC_Task",512,NULL, 5,&LPM_TaskHandle);

    vTaskStartScheduler();   /* 启动任务，开启调度 */

    while(1){
		LED_RED_TOGGLE();
		SDK_DelayAtLeastUs(60000U,SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
	}
}

void PINT0_CallBack(pint_pin_int_t pintr, uint32_t pmatch_status)
{
	
}