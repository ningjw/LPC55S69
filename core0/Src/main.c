#include "main.h"

SysPara  g_sys_para;
ADC_Set  g_adc_set;
rtc_datetime_t sysTime;
flash_config_t flashInstance;
static void InitSysPara();


void main(void)
{
	BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	BOARD_BootClockRUN();
	BOARD_InitPins();

	BOARD_InitPeripherals();
	CTIMER1_Init();
	memory_init();
	FLASH_Init(&flashInstance);
	SPI_Flash_Init();
	InitSysPara();
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
    xTaskCreate((TaskFunction_t )NB_AppTask,"NB_Task",1024,NULL, 3,&NB_TaskHandle);

    /* ����ADC_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )ADC_AppTask, "ADC_Task",1024,NULL, 4,&ADC_TaskHandle);
	
    vTaskStartScheduler();   /* �������񣬿������� */

    while(1);
}

/***************************************************************************************
  * @brief   ��ʼ��ϵͳ����
  * @input   
  * @return  
***************************************************************************************/
static void InitSysPara()
{
    g_sys_para.inactiveCondition = 1;//Ĭ������û��ͨ���ǿ�ʼ��ʱ
    g_sys_para.inactiveTime = 15;    //Ĭ��15����û�л���Զ��ػ���
    g_sys_para.batAlarmValue = 10;   //��ص�������ֵ
	
    g_adc_set.SampleRate = 5120;     //ȡ��Ƶ��
    g_sys_para.sampNumber = 6144;    //12288;    //ȡ������,
    g_sys_para.inactiveCount = 0;    //
    g_sys_para.sampLedStatus = WORK_FINE;
    g_sys_para.batLedStatus = BAT_NORMAL;
    g_sys_para.BleWifiLedStatus = BLE_CLOSE;
    g_adc_set.bias = 2.043f;//�𶯴�������ƫ�õ�ѹĬ��Ϊ2.43V
    g_adc_set.refV = 3.3f;  //�ο���ѹ
    g_sys_para.firmUpdate = false;
    g_sys_para.firmPacksCount = 0;
    g_sys_para.firmSizeCurrent = 0;
}

/***************************************************************************************
  * @brief  BLE����״̬�����жϻص�����
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
  * @brief  NB-IoT����״̬�����жϻص�����
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
  * @brief   utick0�ص�����
  * @input   
  * @return  
***************************************************************************************/
void UTICK0_Callback(void)
{
	extern void FLEXCOMM3_TimeTick(void);
	FLEXCOMM3_TimeTick();
}

