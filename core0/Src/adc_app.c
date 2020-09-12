#include "main.h"

uint32_t SpeedADC[SPD_LEN];
uint32_t ShakeADC[ADC_LEN];
float Temperature[64];

#define ADC_MODE_LOW_POWER       GPIO_PinWrite(GPIO, BOARD_ADC_MODE_PORT, BOARD_ADC_MODE_PIN, 1)  //�͹���ģʽ
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(GPIO, BOARD_ADC_MODE_PORT, BOARD_ADC_MODE_PIN, 0)   //����ģʽ
#define ADC_MODE_HIGH_RESOLUTION //�߾���ģʽ(����)
#define ADC_SYNC_HIGH            GPIO_PinWrite(GPIO, BOARD_ADC_SYNC_PORT, BOARD_ADC_SYNC_PIN, 1)
#define ADC_SYNC_LOW             GPIO_PinWrite(GPIO, BOARD_ADC_SYNC_PORT, BOARD_ADC_SYNC_PIN, 0)

#define PWR_ADC_ON    GPIO_PinWrite(GPIO, BOARD_PWR_ADC_PORT, BOARD_PWR_ADC_PIN, 0)
#define PWR_ADC_OFF   GPIO_PinWrite(GPIO, BOARD_PWR_ADC_PORT, BOARD_PWR_ADC_PIN, 1)

TaskHandle_t ADC_TaskHandle = NULL;  /* ADC������ */

uint32_t timeCapt = 0;
char str[12];

uint32_t ADC_ShakeValue = 0;
uint32_t  ADC_InvalidCnt = 0;
float ADC_VoltageValue;

/***************************************************************************************
  * @brief   
  * @input
  * @return
***************************************************************************************/
void RTC_IRQHANDLER(void)
{
	//�ڲɼ�����ʱ,ÿ���1S��ȡһ���¶�����
	if (g_sys_para.tempCount < sizeof(Temperature) && g_sys_para.WorkStatus){
		Temperature[g_sys_para.tempCount++] = MXL_ReadObjTemp();
	}
	
	if(g_sys_para.inactiveCount++ >= (g_sys_para.inactiveTime + 1)*60-5) { //��ʱʱ�䵽
		GPIO_PinWrite(GPIO, BOARD_PWR_OFF_PORT, BOARD_PWR_OFF_PIN, 1);//�ػ�
	}
}

/***************************************************************************************
  * @brief  ��ʱ�����ڻ�ȡת���źŵ�����(Ƶ��)
  * @input
  * @return
***************************************************************************************/
void CTIMER1_Callback(uint32_t flags)
{
	//������벶��0���ж�
	CTIMER_ClearStatusFlags(CTIMER1, kCTIMER_Capture0Flag);
	if(g_adc_set.spdCount < SPD_LEN){
		SpeedADC[g_adc_set.spdCount++] = CTIMER_GetTimerCountValue(CTIMER1);
	}
}

/***************************************************************************************
  * @brief   start adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStart(void)
{
	g_sys_para.tempCount = 0;
    g_adc_set.spdCount = 0;
    g_adc_set.shkCount = 0;
	memset(ShakeADC,0,ADC_LEN);
	memset(SpeedADC,0,SPD_LEN);

		//�ж��Զ��ػ�����
    if(g_sys_para.inactiveCondition != 1) {
        g_sys_para.inactiveCount = 0;
    }
	
	//�жϵ����Ƿ񳬳��������
	if(g_sys_para.sampNumber > ADC_LEN){
		g_sys_para.sampNumber = ADC_LEN;
	}
	
	PWR_ADC_ON;//����ADC��صĵ�Դ
	vTaskDelay(500);//�ȴ�500ms
	
	//��������LED�Ƶ�����,��ֹͣPendSV��SysTick�ж�
    vTaskSuspend(BAT_TaskHandle);
    vTaskSuspend(LED_TaskHandle);
	NVIC_DisableIRQ(PendSV_IRQn);   
    NVIC_DisableIRQ(SysTick_IRQn);
	
	//����ADCоƬʱ��
	SI5351a_SetPDN(SI_CLK1_CONTROL,true);
	if(g_adc_set.SampleRate > 45000){
		ADC_MODE_HIGH_SPEED;//ʹ�ø���ģʽ
		//ʹ��PWM��ΪADS1271��ʱ��, �䷶ΧΪ37ns - 10000ns (10us)
		ADC_PwmClkConfig(g_adc_set.SampleRate * 256);
		si5351aSetClk0Frequency(g_adc_set.SampleRate * 256);
	}else{
		ADC_MODE_LOW_POWER;//ʹ�õ���ģʽ
		//ʹ��PWM��ΪADS1271��ʱ��, �䷶ΧΪ37ns - 10000ns (10us)
		ADC_PwmClkConfig(g_adc_set.SampleRate * 512);
		si5351aSetClk0Frequency(g_adc_set.SampleRate * 512);
	}

    /* ���PWM ����LTC1063FA��ʱ������,���Ʋ�������*/
	g_sys_para.Ltc1063Clk = 1000 * g_adc_set.SampleRate / 25;
#ifdef HDV_1_0
	g_sys_para.Ltc1063Clk = 1000000;
#endif
	SI5351a_SetPDN(SI_CLK0_CONTROL,true);
	si5351aSetClk1Frequency(g_sys_para.Ltc1063Clk);
	
	//��ʼ�ɼ�����ǰ��ȡһ���¶�
	Temperature[g_sys_para.tempCount++] = MXL_ReadObjTemp();
	//����Ϊtrue��,����PIT�ж��вɼ��¶�����
	g_sys_para.WorkStatus = true;
	
	/* ���벶�񣬼���ת���ź����� */
    CTIMER_StartTimer(CTIMER1);
	
	//����ǰ500������
	ADC_InvalidCnt = 0;
	while (1) { //wait ads1271 ready
        while(ADC_READY == 1){};//�ȴ�ADC_READYΪ�͵�ƽ
		ADC_ShakeValue = ADS1271_ReadData();
		ADC_InvalidCnt++;
		if(ADC_InvalidCnt > 100) break;
    }
	
	while(ADC_READY == 0){};//�ȴ�ADC_READYΪ�ߵ�ƽ
	while(1) { //wait ads1271 ready
        while(ADC_READY == 1){};//�ȴ�ADC_READYΪ�͵�ƽ
		__disable_irq();//�ر��ж�
		ShakeADC[g_adc_set.shkCount++] = ADS1271_ReadData();
		__enable_irq();//�����ж�
		if(g_adc_set.shkCount >= g_sys_para.sampNumber){
			g_adc_set.shkCount = g_sys_para.sampNumber;
			SpeedADC[0] = SpeedADC[1];//�ɼ��ĵ�һ�����ݿ��ܲ���һ������������,���Ե�һ�����ݶ���.
			if(g_sys_para.sampNumber == 0){//Android�����жϲɼ������,��ֵΪ0
				g_adc_set.spdCount = 0;
			}
			break;
		}
    }

	ADC_SampleStop();
}


/***************************************************************************************
  * @brief   stop adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStop(void)
{
	/* Stop channel 0. */
//    PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
	/* Stop get temperature*/
	g_sys_para.WorkStatus = false;
	ADC_PwmClkStop();
    /* Stop the timer */
	CTIMER_StopTimer(CTIMER1);
	
    vTaskResume(BAT_TaskHandle);
    vTaskResume(LED_TaskHandle);
	
	//�����ɼ����ȡһ���¶�
	Temperature[g_sys_para.tempCount++] = MXL_ReadObjTemp();
	
	//�ر�ʱ�����
	SI5351a_SetPDN(SI_CLK0_CONTROL,false);
	SI5351a_SetPDN(SI_CLK1_CONTROL,false);
	
	//�رյ�Դ
	PWR_ADC_OFF;
	
    /* ����ADC��������¼�  */
    xTaskNotify(ADC_TaskHandle, NOTIFY_FINISH, eSetBits);
}


/***********************************************************************
  * @ ������  �� ADC�ɼ�����
  * @ ����˵����
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void ADC_AppTask(void)
{
    uint32_t r_event;
    BaseType_t xReturn = pdTRUE;

	/*����Ϊ�����Լ����*/
	ADC_MODE_LOW_POWER;
//	ADC_PwmClkConfig(1000000);
	si5351aSetClk0Frequency(12000000);//��ADS1271�ṩʱ��
	si5351aSetClk1Frequency(1000000);//�����˲���ʱ��
	g_sys_para.Ltc1063Clk = 1000 * g_adc_set.SampleRate / 25;
#if 0
    /* �ȴ�ADS1271 ready,����ȡ��ѹֵ,���û�гɹ���ȡ��ѹֵ, ��������ʾ */
    while (ADC_READY == 1){};  //wait ads1271 ready
    if(ADS1271_ReadData() == 0) {
        g_sys_para.sampLedStatus = WORK_FATAL_ERR;
    }
#else
	while (1) { //wait ads1271 ready
        while(ADC_READY == 1){};//�ȴ�ADC_READYΪ�͵�ƽ
		ADC_ShakeValue = ADS1271_ReadData();
		ADC_VoltageValue = ADC_ShakeValue * 2.048 / 0x800000;
    }
#endif
	SI5351a_SetPDN(SI_CLK0_CONTROL, false);
	SI5351a_SetPDN(SI_CLK1_CONTROL, false);
	PWR_ADC_OFF;//�ر�ADC�ɼ���صĵ�Դ
    printf("ADC Task Create and Running\r\n");
    while(1)
    {
        /*�ȴ�ADC��ɲ����¼�*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &r_event, portMAX_DELAY);

        /* �ж��Ƿ�ɹ��ȴ����¼� */
        if ( pdTRUE == xReturn ) {
            /* ��ɲ����¼�*/
            if(r_event & NOTIFY_FINISH) {
				/* ---------------�����ź�ת��-----------------------*/
//				printf("�������� %d �����ź�\r\n", g_adc_set.shkCount);
//				float tempValue = 0;
//                for(uint32_t i = 0; i < g_adc_set.shkCount; i++) {
//                    tempValue = ShakeADC[i] * g_adc_set.bias * 1.0f / 0x800000;
//					printf("%01.5f,",tempValue);
//                }
				
				//���㷢�����ź���Ҫ���ٸ���,��������һ�η���182��Byte������, ��һ����������Ҫ3Byte��ʾ, ��һ�δ���58��������
				g_sys_para.shkPacks = (g_adc_set.shkCount / ADC_NUM_ONE_PACK) +  (g_adc_set.shkCount%ADC_NUM_ONE_PACK?1:0);
				//���㷢��ת���ź���Ҫ���ٸ���
				g_sys_para.spdPacks = (g_adc_set.spdCount / ADC_NUM_ONE_PACK) +  (g_adc_set.spdCount%ADC_NUM_ONE_PACK?1:0);
                
				//���㽫һ�βɼ�����ȫ�����͵�Android��Ҫ���ٸ���
#ifdef BLE_VERSION
				g_adc_set.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 3;
#elif defined WIFI_VERSION
				g_adc_set.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 1;
#endif
                /* ------------------ͳ��ƽ���¶�,��С�¶�,����¶�--------------------*/
			    float sum = 0;
				int min_i = 0;
				int max_i = 0;
				for(int i=0;i<g_sys_para.tempCount;i++){
					sum += Temperature[g_sys_para.tempCount];
					min_i = Temperature[i] < Temperature[min_i] ? i : min_i;
					max_i = Temperature[i] > Temperature[max_i] ? i : max_i;
				}
				g_adc_set.Process = sum / g_sys_para.tempCount;
				g_adc_set.ProcessMax = Temperature[max_i];
				g_adc_set.ProcessMin = Temperature[min_i];
				
				W25Q128_AddAdcData();
                /* ��������֪ͨ�������������ڸ�����֪ͨ�µ����� */
                xTaskNotifyGive( BLE_TaskHandle);
            }
        }
    }
}



