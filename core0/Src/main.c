#include "main.h"
#include "fsl_device_registers.h"

#define EXCLUDE_PD (kPDRUNCFG_PD_DCDC | kPDRUNCFG_PD_FRO192M | kPDRUNCFG_PD_FRO32K)

SysPara  g_sys_para;
ADC_Set  g_adc_set;
rtc_datetime_t sysTime;
flash_config_t flashInstance;

static void InitSysPara();

void main(void)
{
	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
	ctimer_config_t config;
	
	BOARD_BootClockRUN();
	BOARD_InitPins();
	BOARD_InitPeripherals();
	
	memory_init();
	SPI_Flash_Init();
	
	InitSysPara();
	PQ_Init(POWERQUAD);
	printf("app start\n");

	/* 创建LED_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )LED_AppTask,"LED_Task",256,NULL, 1,&LED_TaskHandle);
    
	/* 创建ADC_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )ADC_AppTask, "ADC_Task",1024,NULL, 4,&ADC_TaskHandle);
#ifdef NB_VERSION
	PWR_NB_ON;
	
	/* 创建NB_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )NB_AppTask,"NB_Task",512,NULL, 3,&NB_TaskHandle);

    /* 创建NB_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )NFC_AppTask,"NFC_Task",512,NULL, 3,&NFC_TaskHandle);
#endif
	
#if defined(BLE_VERSION) || defined(WIFI_VERSION)
    /* 创建BLE_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )BLE_WIFI_AppTask,"BLE_WIFI_Task",1024,NULL, 3,&BLE_WIFI_TaskHandle);
	/* 创建Battery_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )BAT_AppTask,"BAT_Task",512,NULL, 2,&BAT_TaskHandle);
	/* 创建ADC_Task任务 参数依次为：入口函数、名字、栈大小、函数参数、优先级、控制块 */ 
    xTaskCreate((TaskFunction_t )CORE1_AppTask, "CORE1_Task",512, NULL, 4,&CORE1_TaskHandle);

#endif
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
	Flash_ReadPara();
	
    g_sys_para.inactiveCondition = 1;//默认蓝牙没有通信是开始计时
    g_sys_para.inactiveTime = 15;    //默认15分钟没有活动后，自动关机。
    g_sys_para.batAlarmValue = 10;   //电池电量报警值
	
    g_adc_set.SampleRate = 100000;     //取样频率
    g_sys_para.sampNumber = 3000;    //12288;    //取样点数,
    g_sys_para.inactiveCount = 0;    //
    g_sys_para.sampLedStatus = WORK_FINE;
    g_sys_para.batLedStatus = BAT_NORMAL;
    g_sys_para.BleWifiLedStatus = BLE_CLOSE;
    g_adc_set.bias = 2.043f;//震动传感器的偏置电压默认为2.43V
    g_adc_set.refV = 3.3f;  //参考电压
}

void SystemSleep(void)
{
	printf("enter deep sleep\n");
	PWR_ADC_OFF;//关闭ADC采集相关的电源
	PWR_5V_OFF;
	PWR_NB_OFF;
#ifdef NB_VERSION
	POWER_EnterDeepSleep(EXCLUDE_PD, 0x7FFF, WAKEUP_CTIMER3, 1);
#else
	POWER_EnterDeepSleep(EXCLUDE_PD, 0x7FFF, WAKEUP_FLEXCOMM3, 1);
#endif
	printf("exit deep sleep\n");
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

	//在采集数据时,每间隔1S获取一次温度数据
	if (g_sys_para.tempCount < sizeof(Temperature) && g_sys_para.WorkStatus){
		Temperature[g_sys_para.tempCount++] = TMP101_ReadTemp();
	}
#ifndef NB_VERSION
	if(g_sys_para.inactiveCount++ >= (g_sys_para.inactiveTime + 1)*60-5) { //定时时间到
		GPIO_PinWrite(GPIO, BOARD_PWR_OFF_PORT, BOARD_PWR_OFF_PIN, 1);//关机
	}
#endif
}

int fputc(int ch, FILE* stream)
{
	return ch;
    while (0U == (FLEXCOMM5_PERIPHERAL->STAT & USART_STAT_TXIDLE_MASK)){}
	USART_WriteByte(FLEXCOMM5_PERIPHERAL, (uint8_t)ch);
    return ch;
}
