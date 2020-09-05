#include "main.h"

SysPara  g_sys_para;
ADC_Set  g_adc_set;
rtc_datetime_t sysTime;
flash_config_t flashInstance;

void main(void)
{
	BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	BOARD_BootClockRUN();
	BOARD_InitPins();

	BOARD_InitPeripherals();
	CTIMER1_Init();
	memory_init();
	FLASH_Init(&flashInstance);
	/* ��ʼ��EventRecorder������*/
	EventRecorderInitialize(EventRecordAll, 1U);
	EventRecorderStart();
	printf("app start\n");
	
	/* ����LED_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )LED_AppTask,"LED_Task",256,NULL, 1,&LED_TaskHandle);
    
    /* ����Battery_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )BAT_AppTask,"BAT_Task",1024,NULL, 2,&BAT_TaskHandle);

    /* ����BLE_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )BLE_AppTask,"BLE_Task",1024,NULL, 3,&BLE_TaskHandle);

    /* ����NB_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
//    xTaskCreate((TaskFunction_t )NB_AppTask,"NB_Task",1024,NULL, 3,&NB_TaskHandle);

    /* ����ADC_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
//    xTaskCreate((TaskFunction_t )ADC_AppTask, "ADC_Task",1024,NULL, 4,&ADC_TaskHandle);

    vTaskStartScheduler();   /* �������񣬿������� */

    while(1);
}

/***************************************************************************************
  * @brief  �����жϻص�����
  * @input   
  * @return  
***************************************************************************************/
void PINT0_CallBack(pint_pin_int_t pintr, uint32_t pmatch_status)
{
	
}

/***************************************************************************************
  * @brief   utick0�ص�����
  * @input   
  * @return  
***************************************************************************************/
void UTICK0_Callback(void)
{
	
}

