#include "main.h"
#include "fsl_powerquad.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "math.h"

float ShakeADC[ADC_LEN];
float Temperature[64];

TaskHandle_t ADC_TaskHandle = NULL;  /* ADC������ */

uint32_t timeCapt = 0;
char str[12];

uint32_t ADC_ShakeValue = 0;

float ADC_VoltageValue;

static float GetEnegryWindowCorrected(int windowType) {
    float NBF = 1;
    switch (windowType) {
        case 0://���δ�0
            NBF = 1;                                                 
            break;
        case 1:   //���Ǵ�1
            NBF = 1.33;
            break;
        case 2:   //������2
            NBF = 1.6339;                                            
            break;
        case 3:    //������3
            NBF = 1.59;                                             
            break;
        case 4:    //����������4
            NBF = 1.65;//����
            break;
        case 5:    //Kaiser-Bessel(3.0)                      ������
            NBF = 1.86;                                            
            break;
        case 6:    //top flat  ƽ����
            NBF = 2.26;                                           
            break;
        case 7:    //possion(3.0)
            NBF = 1.65;
            break;
        case 8:    //possion(4.0)
            NBF = 2.08;
            break;
        case 9:    //cauchy(4.0)
            NBF = 1.76;
            break;
        case 10:    //cauchy(5.0)
            NBF = 2.06;
            break;
        case 11:    //Gaussian(3.0)
            NBF = 1.64;
            break;
        case 12:    //Kaiser-Bessel(3.5)
            NBF = 1.93;
            break;

    }
    return NBF;
}


//���δ�0�����Ǵ�1��������2��������3������������4������5������
static float GetRMS(float data[],int len, int windowType) 
{  
    float rms = 0.000;
    float sum = 0.000;
    float EnegryCorrected = GetEnegryWindowCorrected(windowType);
    for (int i = 0; i < len; i++) {
        sum = sum + data[i] * data[i];
    }
    float average = sum / len;
    rms = EnegryCorrected * sqrt(average);
    rms = (float) round(rms * 1000) / 1000.0f;   //����3λС��
    return rms;
}


/***************************************************************************************
  * @brief   start adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStart(void)
{
	g_sys_para.tempCount = 0;
    g_adc_set.shkCount = 0;
	memset(ShakeADC,0,ADC_LEN);
	PWR_ADC_ON;//����ADC��صĵ�Դ
	PWR_5V_ON;//����5V���˲�����Դ
		//�ж��Զ��ػ�����
    if(g_sys_para.inactiveCondition != 1) {
        g_sys_para.inactiveCount = 0;
    }
	
	//�жϵ����Ƿ񳬳��������
	if(g_sys_para.sampNumber > ADC_LEN){
		g_sys_para.sampNumber = ADC_LEN;
	}
	
	/* ���PWM ����LTC1063FA��ʱ������,���Ʋ�������*/
	g_sys_para.Ltc1063Clk = 1000 * g_adc_set.SampleRate / 25;
	SI5351a_SetPDN(SI_CLK0_CONTROL,true);
	si5351aSetFilterClk1(g_sys_para.Ltc1063Clk);
	
	//����ADCоƬʱ��
	SI5351a_SetPDN(SI_CLK1_CONTROL,true);
	if(g_adc_set.SampleRate > 45000){
		ADC_MODE_HIGH_SPEED;//ʹ�ø���ģʽ
		//ʹ��PWM��ΪADS1271��ʱ��, �䷶ΧΪ37ns - 10000ns (10us)
		si5351aSetAdcClk0(g_adc_set.SampleRate * 256);
		ADC_PwmClkStart(g_adc_set.SampleRate * 256, g_sys_para.Ltc1063Clk);
	}else{
		ADC_MODE_LOW_POWER;//ʹ�õ���ģʽ
		//ʹ��PWM��ΪADS1271��ʱ��, �䷶ΧΪ37ns - 10000ns (10us)
		si5351aSetAdcClk0(g_adc_set.SampleRate * 512);
		ADC_PwmClkStart(g_adc_set.SampleRate * 512, g_sys_para.Ltc1063Clk);
	}

	vTaskDelay(100);//�ȴ�500ms
	
	//��ʼ�ɼ�����ǰ��ȡһ���¶�
	Temperature[g_sys_para.tempCount++] = TMP101_ReadTemp();
	//����Ϊtrue��,����PIT�ж��вɼ��¶�����
	g_sys_para.WorkStatus = true;
	
	//����ǰ��������
	int ADC_InvalidCnt = 0;
	while (1) { //wait ads1271 ready
        while(ADC_READY == 1){};//�ȴ�ADC_READYΪ�͵�ƽ
		ADC_ShakeValue = ADS1271_ReadData();
		ADC_InvalidCnt++;
		if(ADC_InvalidCnt > 100) break;
    }
	
	start_spd_caputer();
	__disable_irq();//�ر��ж�
	while(ADC_READY == 0){};//�ȴ�ADC_READYΪ�ߵ�ƽ
	while(1) { //wait ads1271 ready
        while(ADC_READY == 1){};//�ȴ�ADC_READYΪ�͵�ƽ
		ShakeADC[g_adc_set.shkCount++] = ADS1271_ReadData();
		if(g_adc_set.shkCount >= g_sys_para.sampNumber){
			g_adc_set.shkCount = g_sys_para.sampNumber;
			break;
		}
    }
	__enable_irq();//�����ж�
	stop_spd_caputer();
	if(g_sys_para.sampNumber == 0){//Android�����жϲɼ������,��ֵΪ0
		spd_msg->len = 0;
	}
	//�����ɼ����ȡһ���¶�
	Temperature[g_sys_para.tempCount++] = TMP101_ReadTemp();
	ADC_SampleStop();
}


/***************************************************************************************
  * @brief   stop adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStop(void)
{
	/* Stop get temperature*/
	g_sys_para.WorkStatus = false;
	
	//�ر�ʱ�����
	SI5351a_SetPDN(SI_CLK0_CONTROL,false);
	SI5351a_SetPDN(SI_CLK1_CONTROL,false);
	ADC_PwmClkStop();
	//�رյ�Դ
	PWR_ADC_OFF;
	PWR_5V_OFF;//����5V���˲�����Դ
	
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
	arm_rfft_instance_q31 instance;
	/*����Ϊ�����Լ����*/
	ADC_MODE_LOW_POWER;
	si5351aSetAdcClk0(1000000);//��ADS1271�ṩʱ��
	si5351aSetFilterClk1(1000000);//�����˲���ʱ��
	PWR_5V_ON;
	PWR_ADC_ON;
#if 1
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
	PWR_5V_OFF;
    printf("ADC Task Create and Running\r\n");
	TMP101_Init();
    while(1)
    {
        /*�ȴ�ADC��ɲ����¼�*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &r_event, portMAX_DELAY);

        /* �ж��Ƿ�ɹ��ȴ����¼� */
        if ( pdTRUE == xReturn ) {
            /* ��ɲ����¼�*/
            if(r_event & NOTIFY_FINISH) {
				/* ---------------�����ź�ת��-----------------------*/
#if 0
				float tempValue = 0;
                for(uint32_t i = 0; i < g_adc_set.shkCount; i++) {
					if((uint32_t)ShakeADC[i] < 0x800000){
						ShakeADC[i] = ShakeADC[i] * g_adc_set.bias * 1.0f / 0x800000;
					}else{
						ShakeADC[i] = ((ShakeADC[i] - 0x800000) * g_adc_set.bias * 1.0f / 0x800000) - g_adc_set.bias;
					}
					printf("%01.5f,",ShakeADC[i]);
                }
				
				g_sys_para.shkRMS = GetRMS(ShakeADC, g_adc_set.shkCount, g_adc_set.WindowsType);
#endif
				//���㷢�����ź���Ҫ���ٸ���,��������һ�η���182��Byte������, ��һ����������Ҫ3Byte��ʾ, ��һ�δ���58��������
				g_sys_para.shkPacks = (g_adc_set.shkCount / ADC_NUM_ONE_PACK) +  (g_adc_set.shkCount%ADC_NUM_ONE_PACK?1:0);
				
				//���㷢��ת���ź���Ҫ���ٸ���
				g_sys_para.spdPacks = (spd_msg->len / ADC_NUM_ONE_PACK) +  (spd_msg->len%ADC_NUM_ONE_PACK?1:0);
				
				g_adc_set.spdStartSid = g_sys_para.shkPacks + 3;
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
                xTaskNotifyGive( BLE_WIFI_TaskHandle);
            }
        }
    }
}



