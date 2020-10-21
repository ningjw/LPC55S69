#include "main.h"
#include "fsl_device_registers.h"

#define CORE1_BOOT_ADDRESS 0x20033000
	
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
/* Shared variable by both cores, before changing of this variable the cores must
   first take mailbox mutex, after changing the shared variable must return mutex */
volatile uint32_t g_msg = 1;
/* For the flow control */
volatile bool g_secondary_core_started = false;

static void InitSysPara();
static void core1_init();

void main(void)
{
	BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	
	BOARD_BootClockRUN();
	BOARD_InitPins();
	
	BOARD_InitPeripherals();
	
#ifdef CORE1_IMAGE_COPY_TO_RAM
	core1_init();
#endif
	
	CTIMER1_Init();
	memory_init();
//	FLASH_Init(&flashInstance);
	SPI_Flash_Init();
	PWR_NB_ON;
	InitSysPara();
	PQ_Init(POWERQUAD);
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

void start_secondary_core(uint32_t sec_core_boot_addr)
{
    /* Boot source for Core 1 from flash */
    SYSCON->CPUCFG |= SYSCON_CPUCFG_CPU1ENABLE_MASK;
    SYSCON->CPBOOT = SYSCON_CPBOOT_CPBOOT(sec_core_boot_addr);

    int32_t temp = SYSCON->CPUCTRL;
    temp |= 0xc0c48000;
    SYSCON->CPUCTRL = temp | SYSCON_CPUCTRL_CPU1RSTEN_MASK | SYSCON_CPUCTRL_CPU1CLKEN_MASK;
    SYSCON->CPUCTRL = (temp | SYSCON_CPUCTRL_CPU1CLKEN_MASK) & (~SYSCON_CPUCTRL_CPU1RSTEN_MASK);
}

void stop_secondary_core(void)
{
    uint32_t temp = SYSCON->CPUCTRL;
    temp |= 0xc0c48000U;

    /* hold in reset and disable clock */
    SYSCON->CPUCTRL = (temp | SYSCON_CPUCTRL_CPU1RSTEN_MASK) & (~SYSCON_CPUCTRL_CPU1CLKEN_MASK);
}


static void core1_init()
{
	/* Init Mailbox */
    MAILBOX_Init(MAILBOX);
	
    /* Enable mailbox interrupt */
    NVIC_EnableIRQ(MAILBOX_IRQn);
	
	/* Copy Secondary core application from FLASH to the target memory. */
    memcpy((void *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, CORE1_IMAGE_SIZE);
	printf("Copy CORE1 image to address: 0x%x, size: %d\r\n", CORE1_BOOT_ADDRESS, CORE1_IMAGE_SIZE);

	/* Boot Secondary core application */
	start_secondary_core(CORE1_BOOT_ADDRESS);
	
	/* Wait for start and initialization of secondary core */
    while (!g_secondary_core_started);
	printf("core1 start ok");
	
	printf("Write to the secondary core mailbox register: %d\r\n", g_msg);
    /* Write g_msg to the secondary core mailbox register - it causes interrupt on the secondary core */
    MAILBOX_SetValue(MAILBOX, kMAILBOX_CM33_Core1, g_msg);

    while (1)
    {
        __WFI();
    }
}
uint32_t value = 0;
void MAILBOX_IRQHandler(void)
{
    if (!g_secondary_core_started)
    {
		value = MAILBOX_GetValue(MAILBOX, kMAILBOX_CM33_Core0);
        if (1234 == value)
        {
            g_secondary_core_started = true;
        }
        MAILBOX_ClearValueBits(MAILBOX, kMAILBOX_CM33_Core0, 0xffffffff);
    }
    else
    {
        g_msg = MAILBOX_GetValue(MAILBOX, kMAILBOX_CM33_Core0);
        MAILBOX_ClearValueBits(MAILBOX, kMAILBOX_CM33_Core0, 0xffffffff);
        printf("Read value from the primary core mailbox register: %d\r\n", g_msg);
        g_msg++;
        printf("Write to the secondary core mailbox register: %d\r\n", g_msg);
        MAILBOX_SetValue(MAILBOX, kMAILBOX_CM33_Core1, g_msg);
    }
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
