#include "main.h"

SysPara  g_sys_para;
ADC_Set  g_adc_set;
rtc_datetime_t sysTime;
flash_config_t flashInstance;
static void InitSysPara();


void main(void)
{
	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
	BOARD_BootClockRUN();
	BOARD_InitPins();

	BOARD_InitPeripherals();
	CTIMER1_Init();
	memory_init();
	FLASH_Init(&flashInstance);
	SPI_Flash_Init();
	InitSysPara();
	/* 初始化EventRecorder并开启*/
	EventRecorderInitialize(EventRecordAll, 1U);
	EventRecorderStart();
	printf("app start\n");
	
	/* 创建LED_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )LED_AppTask,"LED_Task",256,NULL, 1,&LED_TaskHandle);
    
    /* 创建Battery_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )BAT_AppTask,"BAT_Task",1024,NULL, 2,&BAT_TaskHandle);

    /* 创建BLE_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )BLE_AppTask,"BLE_Task",1024,NULL, 3,&BLE_TaskHandle);

    /* 创建NB_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )NB_AppTask,"NB_Task",1024,NULL, 3,&NB_TaskHandle);

    /* 创建ADC_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )ADC_AppTask, "ADC_Task",1024,NULL, 4,&ADC_TaskHandle);
	
    vTaskStartScheduler();   /* 启动任务，开启调度 */

    while(1);
}

/***************************************************************************************
  * @brief   初始化系统变量
  * @input   
  * @return  
***************************************************************************************/
static void InitSysPara()
{
    g_sys_para.inactiveCondition = 1;//默认蓝牙没有通信是开始计时
    g_sys_para.inactiveTime = 15;    //默认15分钟没有活动后，自动关机。
    g_sys_para.batAlarmValue = 10;   //电池电量报警值
	
    g_adc_set.SampleRate = 5120;     //取样频率
    g_sys_para.sampNumber = 6144;    //12288;    //取样点数,
    g_sys_para.inactiveCount = 0;    //
    g_sys_para.sampLedStatus = WORK_FINE;
    g_sys_para.batLedStatus = BAT_NORMAL;
    g_sys_para.BleWifiLedStatus = BLE_CLOSE;
    g_adc_set.bias = 2.043f;//震动传感器的偏置电压默认为2.43V
    g_adc_set.refV = 3.3f;  //参考电压
    g_sys_para.firmUpdate = false;
    g_sys_para.firmPacksCount = 0;
    g_sys_para.firmSizeCurrent = 0;
}

/***************************************************************************************
  * @brief  BLE连接状态引脚中断回调函数
  * @input   
  * @return  
***************************************************************************************/
void PINT1_CallBack(pint_pin_int_t pintr, uint32_t pmatch_status)
{
	if(GPIO_PinRead(GPIO, BOARD_BT_STATUS_PORT, BOARD_BT_STATUS_PIN))
	{
		g_sys_para.BleWifiLedStatus = BLE_CONNECT;
	}else{
		g_sys_para.BleWifiLedStatus = BLE_READY;
	}
}

/***************************************************************************************
  * @brief  NB-IoT网络状态引脚中断回调函数
  * @input   
  * @return  
***************************************************************************************/
void PINT2_CallBack(pint_pin_int_t pintr, uint32_t pmatch_status)
{
	if(GPIO_PinRead(GPIO, BOARD_NB_NETSTATUS_PORT, BOARD_NB_NETSTATUS_PIN))
	{
		g_sys_para.BleWifiLedStatus = BLE_CONNECT;
	}else{
		g_sys_para.BleWifiLedStatus = BLE_READY;
	}
}

/***************************************************************************************
  * @brief   utick0回调函数
  * @input   
  * @return  
***************************************************************************************/
void UTICK0_Callback(void)
{
	extern void FLEXCOMM3_TimeTick(void);
	FLEXCOMM3_TimeTick();
}

