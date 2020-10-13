#include "main.h"
#include "fsl_device_registers.h"

#define CORE1_BOOT_ADDRESS (void *)0x20033000
	
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#define CORE1_IMAGE_SIZE  (uint32_t)&Image$$CORE1_REGION$$Length

#define EXCLUDE_PD (kPDRUNCFG_PD_DCDC | kPDRUNCFG_PD_FRO192M | kPDRUNCFG_PD_FRO32K)

typedef struct {  //����ṹ��
	uint16_t len;
	uint16_t spdData[1024];
}msg_t;
msg_t * my_msg_t=NULL;        //����ṹ��ָ�롣


extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;

SysPara  g_sys_para;
ADC_Set  g_adc_set;
rtc_datetime_t sysTime;
flash_config_t flashInstance;
volatile uint32_t startupDone     = 0U;

static void InitSysPara();
static void core1_init();



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
	PWR_NB_ON;
	InitSysPara();
	PQ_Init(POWERQUAD);
	printf("app start\n");

#ifdef CORE1_IMAGE_COPY_TO_RAM
	core1_init();
#endif

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

mcmgr_status_t mcmgr_start_core_internal(mcmgr_core_t coreNum, void *bootAddress)
{
    if (coreNum != kMCMGR_Core1)
    {
        return kStatus_MCMGR_Error;
    }

    SYSCON->CPUCFG |= SYSCON_CPUCFG_CPU1ENABLE_MASK;

    /* Boot source for Core 1 from RAM */
    SYSCON->CPBOOT = SYSCON_CPBOOT_CPBOOT(bootAddress);

    uint32_t temp = SYSCON->CPUCTRL;
    temp |= 0xc0c48000U;
    SYSCON->CPUCTRL = temp | SYSCON_CPUCTRL_CPU1RSTEN_MASK | SYSCON_CPUCTRL_CPU1CLKEN_MASK;
    SYSCON->CPUCTRL = (temp | SYSCON_CPUCTRL_CPU1CLKEN_MASK) & (~SYSCON_CPUCTRL_CPU1RSTEN_MASK);

    return kStatus_MCMGR_Success;
}

mcmgr_status_t mcmgr_stop_core_internal(mcmgr_core_t coreNum)
{
    if (coreNum != kMCMGR_Core1)
    {
        return kStatus_MCMGR_Error;
    }
    uint32_t temp = SYSCON->CPUCTRL;
    temp |= 0xc0c48000U;

    /* hold in reset and disable clock */
    SYSCON->CPUCTRL = (temp | SYSCON_CPUCTRL_CPU1RSTEN_MASK) & (~SYSCON_CPUCTRL_CPU1CLKEN_MASK);
    return kStatus_MCMGR_Success;
}

void delay(void)
{
    for (uint32_t i = 0; i < 0x7fffffU; i++)
    {
        __NOP();
    }
}

static void core1_init()
{
	/* Init Mailbox */
    MAILBOX_Init(MAILBOX);
	
    /* Enable mailbox interrupt */
    NVIC_EnableIRQ(MAILBOX_IRQn);
	
	/* Copy Secondary core application from FLASH to the target memory. */
    memcpy(CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, CORE1_IMAGE_SIZE);
	
	/* Boot Secondary core application */
	mcmgr_start_core_internal(kMCMGR_Core1, CORE1_BOOT_ADDRESS);
	
	delay();
	
	while (MAILBOX_GetMutex(MAILBOX) == 0){}
	
	printf("len=%d\n",my_msg_t->len);
	
	MAILBOX_SetMutex(MAILBOX);
		
		
//	mcmgr_stop_core_internal(kMCMGR_Core1);
}

void MAILBOX_IRQHandler(void)
{
	uint32_t data;
	data = MAILBOX_GetValue(MAILBOX, kMAILBOX_CM33_Core0);
	
	if(data>=0x20000000)
    {
        my_msg_t = (msg_t *)(data);
    }
    MAILBOX_ClearValueBits(MAILBOX, kMAILBOX_CM33_Core0, data);
	__DSB();
}

/***************************************************************************************
  * @brief   ��ʼ��ϵͳ����
  * @input   
  * @return  
***************************************************************************************/
static void InitSysPara()
{
	Flash_ReadPara();
	
    g_sys_para.inactiveCondition = 1;//Ĭ������û��ͨ���ǿ�ʼ��ʱ
    g_sys_para.inactiveTime = 15;    //Ĭ��15����û�л���Զ��ػ���
    g_sys_para.batAlarmValue = 10;   //��ص�������ֵ
	
    g_adc_set.SampleRate = 100000;     //ȡ��Ƶ��
    g_sys_para.sampNumber = 3000;    //12288;    //ȡ������,
    g_sys_para.inactiveCount = 0;    //
    g_sys_para.sampLedStatus = WORK_FINE;
    g_sys_para.batLedStatus = BAT_NORMAL;
    g_sys_para.BleWifiLedStatus = BLE_CLOSE;
    g_adc_set.bias = 2.043f;//�𶯴�������ƫ�õ�ѹĬ��Ϊ2.43V
    g_adc_set.refV = 3.3f;  //�ο���ѹ
}

void SystemSleep(void)
{
	printf("enter deep sleep\n");
	PWR_ADC_OFF;//�ر�ADC�ɼ���صĵ�Դ
	PWR_5V_OFF;
	PWR_NB_OFF;
	POWER_EnterDeepSleep(EXCLUDE_PD, 0x7FFF, WAKEUP_FLEXCOMM3, 1);
	printf("exit deep sleep\n");
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
	
}

int fputc(int ch, FILE* stream)
{
    while (0U == (FLEXCOMM5_PERIPHERAL->STAT & USART_STAT_TXIDLE_MASK)){}
	USART_WriteByte(FLEXCOMM5_PERIPHERAL, (uint8_t)ch);
    return ch;
}
