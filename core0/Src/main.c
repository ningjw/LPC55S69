#include "main.h"
#include "fsl_device_registers.h"

#define EXCLUDE_PD (kPDRUNCFG_PD_DCDC | kPDRUNCFG_PD_FRO192M | kPDRUNCFG_PD_FRO32K)

SysPara        g_sys_para;
SysFlashPara   g_sys_flash_para;
SysSamplePara  g_sample_para;
rtc_datetime_t sysTime;
flash_config_t flashInstance;
uint8_t g_commTxBuf[FLEXCOMM_BUFF_LEN] = {0};//ble/wifi/nfc/cat1 ���õĴ��ڷ��ͻ�����

static void InitSysPara();

void main(void)
{
	BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	ctimer_config_t config;
	
	BOARD_BootClockRUN();
	BOARD_InitPins();
	BOARD_InitPeripherals();
	
	memory_init();
	SPI_Flash_Init();
	InitSysPara();
	DEBUG_PRINTF("app start, version = %s\n",SOFT_VERSION);
	RTC_GetDatetime(RTC, &sysTime);
	DEBUG_PRINTF("%d-%02d-%02d %02d:%02d:%02d\r\n",
				sysTime.year,sysTime.month,sysTime.day,
				sysTime.hour,sysTime.minute,sysTime.second);
    
	/* ����LED_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )LED_AppTask,"LED_Task",512,NULL, 1,&LED_TaskHandle);
    
	/* ����ADC_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )ADC_AppTask, "ADC_Task",1024,NULL, 4,&ADC_TaskHandle);
	
	/* ����Battery_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )BAT_AppTask,"BAT_Task",512,NULL, 2,&BAT_TaskHandle);
	
#ifdef CAT1_VERSION
	/* ����CAT1_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )CAT1_AppTask,"CAT1_Task",1536,NULL, 3,&CAT1_TaskHandle);

    /* ����NFC_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
//    xTaskCreate((TaskFunction_t )NFC_AppTask,"NFC_Task",512,NULL, 3,&NFC_TaskHandle);
#endif
 
#if defined(BLE_VERSION) || defined(WIFI_VERSION)
    /* ����BLE_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )BLE_WIFI_AppTask,"BLE_WIFI_Task",1024,NULL, 3,&BLE_WIFI_TaskHandle);
	
	/* ����ADC_Task���� ��������Ϊ����ں��������֡�ջ��С���������������ȼ������ƿ� */ 
    xTaskCreate((TaskFunction_t )CORE1_AppTask, "CORE1_Task",512, NULL, 4,&CORE1_TaskHandle);
#endif
	
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
	Flash_ReadPara();
	if(g_sys_flash_para.firstPoweron != 0xAA)
    {
        g_sys_flash_para.firstPoweron = 0xAA;
        g_sys_flash_para.autoPwrOffCondition = 1;//Ĭ������û��ͨ���ǿ�ʼ��ʱ
        g_sys_flash_para.autoPwrOffIdleTime = 15;    //Ĭ��15����û�л���Զ��ػ���
        g_sys_flash_para.batAlarmValue = 10;   //��ص�������ֵ
        g_sys_flash_para.bias = 2.043f;        //�𶯴�������ƫ�õ�ѹĬ��Ϊ2.43V
        g_sys_flash_para.refV = 3.3f;          //�ο���ѹ
        
        g_sample_para.Averages = 1;
		strcpy(g_sample_para.SpeedUnits, "RPM");
		strcpy(g_sample_para.ProcessUnits, "C");
		strcpy(g_sample_para.EU, "mm/s");
		g_sample_para.Senstivity = 50;
		g_sample_para.Zerodrift = 0;
		g_sample_para.EUType = 4;
		g_sample_para.WindowsType = 2;
		g_sample_para.StartFrequency = 0;
		g_sample_para.EndFrequency = 1000;
		g_sample_para.SampleRate = 2560;     //ȡ��Ƶ��
        g_sample_para.Lines = 1600;          //����
		g_sample_para.Averages = 1;
		g_sample_para.AverageOverlap = 0;
		g_sample_para.AverageType = 0;
		g_sample_para.EnvFilterLow = 500;
		g_sample_para.EnvFilterHigh = 10000;
		g_sample_para.IncludeMeasurements = 1;
        g_sample_para.sampNumber = 2.56 * g_sample_para.Lines * g_sample_para.Averages * (1 - g_sample_para.AverageOverlap)
                                + 2.56 * g_sample_para.Lines * g_sample_para.AverageOverlap;
        g_sample_para.sampleInterval = 5;      //����ʱ����5���Ӳ���һ��.
        g_sys_flash_para.reportVersion = true;
        SPI_Flash_Erase_Sector(0);
        Flash_SavePara();
    }
    g_sys_para.sysIdleCount = 0;    //
    g_sys_para.sampLedStatus = WORK_FINE;
    g_sys_para.batLedStatus = BAT_NORMAL;
    g_sys_para.BleWifiLedStatus = BLE_CLOSE;
}

void SystemSleep(void)
{
	DEBUG_PRINTF("enter deep sleep\n");
	PWR_3V3A_OFF;//�ر�ADC�ɼ���صĵ�Դ
	PWR_5V_OFF;
	PWR_CAT1_OFF;
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT,  BOARD_LED_SYS_GREEN_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_ADC_FORMAT_PORT,  BOARD_ADC_FORMAT_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_ADC_SYNC_PORT,  BOARD_ADC_SYNC_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_ADC_MODE_PORT,  BOARD_ADC_MODE_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_PWR_SDA_PORT,  BOARD_PWR_SDA_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_TMP_SCL_PORT,  BOARD_TMP_SCL_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_TMP_SDA_PORT,  BOARD_TMP_SDA_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_FLASH_MOSI_PORT,  BOARD_FLASH_MOSI_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_FLASH_SCK_PORT,  BOARD_FLASH_SCK_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_FLASH_WP_PORT,  BOARD_FLASH_WP_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_ADC_SPI_SCK_PORT,  BOARD_ADC_SPI_SCK_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_FLASH_CS_PORT,  BOARD_FLASH_CS_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_FLT_CLK_PORT,  BOARD_FLT_CLK_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_ADC_CLK_PORT,  BOARD_ADC_CLK_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_NFC_RSTPD_PORT,  BOARD_NFC_RSTPD_PIN, 0);
	
	GPIO_PinWrite(GPIO,0,18,0);//2:NB_reload: PIO0-18
	GPIO_PinWrite(GPIO,0,19,0);//4:PIO0_19
	GPIO_PinWrite(GPIO,1,28,0);//6:NB_RST: PIO1_28
	GPIO_PinWrite(GPIO,1,15,0);//8:PIO1_15
	GPIO_PinWrite(GPIO,1,19,0);//9:NB_EN: PIO1_19
	GPIO_PinWrite(GPIO,0,27,0);//10:NB_RXD:PIO0_27
	GPIO_PinWrite(GPIO,0,17,0);//11:PIO0_17
	GPIO_PinWrite(GPIO,1,27,0);//12:PIO1_27
	GPIO_PinWrite(GPIO,0,29,0);//13:NB_ST: PIO0_29
	GPIO_PinWrite(GPIO,0,26,0);//14:NB_TXD: PIO0_26
	GPIO_PinWrite(GPIO,0,2,0);//15:PIO0_2
	GPIO_PinWrite(GPIO,1,26,0);//16:PIO1_26
	void BOARD_InitPinsInput(void);
	BOARD_InitPinsInput();
#ifdef CAT1_VERSION
	POWER_EnterDeepSleep(EXCLUDE_PD, 0x7FFF, WAKEUP_CTIMER3, 1);
#else
	POWER_EnterDeepSleep(EXCLUDE_PD, 0x7FFF, WAKEUP_FLEXCOMM3, 1);
#endif
	DEBUG_PRINTF("exit deep sleep\n");
	BOARD_InitPins();
}

#ifndef CAT1_VERSION
/***************************************************************************************
  * @brief  BLE/wifi����״̬�����жϻص�����
  * @input   
  * @return  
***************************************************************************************/
void PINT1_CallBack(pint_pin_int_t pintr, uint32_t pmatch_status)
{
	//nLink�͵�ƽ��ʾWIFI������
	if(GPIO_PinRead(GPIO, BOARD_BT_STATUS_PORT, BOARD_BT_STATUS_PIN) == 0)
	{
		g_sys_para.BleWifiLedStatus = BLE_WIFI_CONNECT;
	}else{
		g_sys_para.BleWifiLedStatus = BLE_WIFI_READY;
	}
}

/***************************************************************************************
  * @brief  NB-IoT����״̬�����жϻص�����
  * @input   
  * @return  
***************************************************************************************/
void PINT2_CallBack(pint_pin_int_t pintr, uint32_t pmatch_status)
{
	#ifdef CAT1_VERSION
	if(GPIO_PinRead(GPIO, BOARD_NB_NETSTATUS_PORT, BOARD_NB_NETSTATUS_PIN))
	{
		g_sys_para.BleWifiLedStatus = BLE_WIFI_CONNECT;
	}else{
		g_sys_para.BleWifiLedStatus = BLE_WIFI_READY;
	}
	#endif
}
#endif
/***************************************************************************************
  * @brief   utick0�ص�����
  * @input   
  * @return  
***************************************************************************************/
void UTICK0_Callback(void)
{
	uint32_t static sleep_time_cnt = 0;
	//�ڲɼ�����ʱ,ÿ���1S��ȡһ���¶�����
	if (g_sys_para.tempCount < sizeof(Temperature) && g_sys_para.WorkStatus){
		Temperature[g_sys_para.tempCount++] = TMP101_ReadTemp();
	}else if(sleep_time_cnt++ > 5){
//		SystemSleep();
	}
	
#ifndef CAT1_VERSION
	if(g_sys_para.sysIdleCount++ >= (g_sys_para.autoPwrOffIdleTime + 1)*60-5) { //��ʱʱ�䵽
		GPIO_PinWrite(GPIO, BOARD_PWR_OFF_PORT, BOARD_PWR_OFF_PIN, 1);//�ػ�
	}
#endif
}

void delay_us(uint32_t nus)
{
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;				//LOAD��ֵ	    	 
	ticks=nus*1; 						//��Ҫ�Ľ����� 
	told=SysTick->VAL;        				//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
		}  
	};
}

#ifndef CAT1_VERSION
int fputc(int ch, FILE* stream)
{
    while (0U == (FLEXCOMM5_PERIPHERAL->STAT & USART_STAT_TXIDLE_MASK)){}
	USART_WriteByte(FLEXCOMM5_PERIPHERAL, (uint8_t)ch);
    return ch;
}
#endif
