#include "main.h"

#define BAT_CHG_UNCOMPLETE GPIO_PinWrite(GPIO, BOARD_PWR_CHG_COMPLETE_PORT, BOARD_PWR_CHG_COMPLETE_PIN, 0)

#define READ_CHARGE_STA    GPIO_PinRead(GPIO, BOARD_BAT_CHRG_PORT, BOARD_BAT_CHRG_PIN)
#define READ_STDBY_STA     GPIO_PinRead(GPIO, BOARD_BAT_STDBY_PORT,BOARD_BAT_STDBY_PIN)

#define DEMO_LPADC_USER_CHANNEL 13U
#define DEMO_LPADC_USER_CMDID   1U /* CMD1 */
#define DEMO_LPADC_VREF_SOURCE  kLPADC_ReferenceVoltageAlt2

TaskHandle_t BAT_TaskHandle = NULL;  /* ��ع��������� */
uint8_t status = 0;
lpadc_conv_result_t         mLpadcResult;
/***********************************************************************
  * @ ������  �� BAT_AppTask
  * @ ����˵����
  * @ ����    �� ��
  * @ ����ֵ  �� ��
  **********************************************************************/
void BAT_AppTask(void)
{
#if defined( WIFI_VERSION) || defined(BLE_VERSION)
    // Enable auto measurement of battery voltage and temperature
    LTC2942_SetADCMode(LTC2942_ADC_AUTO);
	
    // Enable analog section of the chip (in case if it disabled)
    LTC2942_SetAnalog(LTC2942_AN_ENABLED);
	
    // Set prescaler M value
    LTC2942_SetPrescaler(LTC2942_PSCM_64);

	LTC2942_SetALCCMode(LTC2942_ALCC_DISABLED);
	
	g_sys_para.batRegAC = LTC2942_GetAC();
	//��ʱ��ص������оƬ�е�ACֵΪ��ȫ������ֵ,��Ҫ��flash��ȡ
	if(g_sys_para.batRegAC >= 0x7FE0 && g_sys_para.batRegAC <= 0x7FFF){
		DEBUG_PRINTF("%s: LTC2942_SetAC\r\n",__func__);
		LTC2942_SetAC(g_sys_para.batRegAC);
		//����ʱ�����������,Ϊ�˲��԰ε���غ����ϵ�᲻��ɹ�ִ�е���if�����
		//Ҫ����������������,����Ҫע�͵�while(1)�жԵ��״̬�ļ�����
//		g_sys_para.batLedStatus = BAT_CHARGING;
	}
#else
	lpadc_config_t mLpadcConfigStruct;
    lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;
    lpadc_conv_command_config_t mLpadcCommandConfigStruct;

	CLOCK_SetClkDiv(kCLOCK_DivAdcAsyncClk, 16U, true);
    CLOCK_AttachClk(kMAIN_CLK_to_ADC_CLK);
    /* Disable LDOGPADC power down */
    POWER_DisablePD(kPDRUNCFG_PD_LDOGPADC);
	
	LPADC_GetDefaultConfig(&mLpadcConfigStruct);
    mLpadcConfigStruct.enableAnalogPreliminary = true;
	mLpadcConfigStruct.referenceVoltageSource = DEMO_LPADC_VREF_SOURCE;
	mLpadcConfigStruct.conversionAverageMode = kLPADC_ConversionAverage128;
	LPADC_Init(ADC0, &mLpadcConfigStruct);
	
	LPADC_SetOffsetValue(ADC0, 10, 10);
	LPADC_DoAutoCalibration(ADC0);
	
	/* Set conversion CMD configuration. */
    LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
    mLpadcCommandConfigStruct.channelNumber = DEMO_LPADC_USER_CHANNEL;
    LPADC_SetConvCommandConfig(ADC0, DEMO_LPADC_USER_CMDID, &mLpadcCommandConfigStruct);

    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
    mLpadcTriggerConfigStruct.targetCommandId       = DEMO_LPADC_USER_CMDID;
    mLpadcTriggerConfigStruct.enableHardwareTrigger = false;
    LPADC_SetConvTriggerConfig(ADC0, 0U, &mLpadcTriggerConfigStruct); /* Configurate the trigger0. */

#endif
    DEBUG_PRINTF("BAT_AppTask Running\r\n");
	
    while(1)
    {
#if defined( WIFI_VERSION) || defined(BLE_VERSION)
        // ��ȡ��ص�ѹ
        g_sys_para.batVoltage = LTC2942_GetVoltage() / 1000.0;
		
        // ��ȡ�¶ȴ������¶�
//        g_sys_para.batTemp = LTC2942_GetTemperature() / 100.0;
		
		// ��ȡ�����ٷֱ�
		g_sys_para.batRegAC = LTC2942_GetAC();
		if(g_sys_para.batRegAC > 0x7FE0 && g_sys_para.batRegAC <= 0x7FFF){//���ﱣ֤AC�Ĵ���������ݲ�Ϊ7FFF,��Ϊ7FFF�ڿ���ʱ����Ϊһ���ж�
			if(READ_CHARGE_STA == 0 && READ_STDBY_STA == 1) {//��統��
				g_sys_para.batRegAC = 0x8000;
				LTC2942_SetAC(0x8000);
			}else{
				g_sys_para.batRegAC = 0x7FE0;
				LTC2942_SetAC(0x7FE0);
			}
			Flash_SavePara();//��AC�Ĵ�����ֵд��flash��������
		}
        g_sys_para.batRemainPercent = g_sys_para.batRegAC * 100.0 / 0xFFFF;

		//��������flash�еĵ�ص����뵱ǰ��ص��������ʱ
		//����ǰ�������±��浽flash����
		if(g_sys_para.batRemainPercent != g_sys_para.batRemainPercentBak){
			Flash_SavePara();
		}
		
        if(READ_CHARGE_STA == 0 && READ_STDBY_STA == 1) {//��統��
//			DEBUG_PRINTF("%s: Battery charging \r\n",__func__);
            g_sys_para.batLedStatus = BAT_CHARGING;
            BAT_CHG_UNCOMPLETE;
			if (g_sys_para.batRemainPercent == 100){//��統�м�⵽����Ϊ100%, ��Ϊ99%
				g_sys_para.batRemainPercent = 99;
			}
        } else if(READ_CHARGE_STA == 1 && READ_STDBY_STA == 0) { //������
//			DEBUG_PRINTF("%s: Battery full \r\n",__func__);
            g_sys_para.batLedStatus = BAT_FULL;
            LTC2942_SetAC(0xFFFF);
        } else 
#elif defined CAT1_VERSION
		LPADC_DoSoftwareTrigger(ADC0, 1U); /* 1U is trigger0 mask. */
		while (!LPADC_GetConvResult(ADC0, &mLpadcResult, 0U)) {
			vTaskDelay(1000);
		}
		g_sys_para.batVoltage = (67793.0 - 3.5028 * mLpadcResult.convValue) / 10000.0;
		//���ݵ�ѹ����������
		if(g_sys_para.batVoltage >= 3.73f) { //(3.73 - 4.2)
			g_sys_para.batRemainPercent = -308.19f * g_sys_para.batVoltage * g_sys_para.batVoltage + 2607.7f * g_sys_para.batVoltage - 5417.9f;
		} else if(g_sys_para.batVoltage >= 3.68f) { //(3.68 - 3.73)
			g_sys_para.batRemainPercent = -1666.7f * g_sys_para.batVoltage * g_sys_para.batVoltage + 12550 * g_sys_para.batVoltage - 23603;
		} else { // (3.5 - 3.68)
			g_sys_para.batRemainPercent = 55.556f * g_sys_para.batVoltage - 194.44f;
		}
#endif
		if(g_sys_para.batRemainPercent <= g_sys_flash_para.batAlarmValue) { //�������ڱ���ֵ
            DEBUG_PRINTF("%s: Percent <= AlarmValue \r\n",__func__);
			g_sys_para.batLedStatus = BAT_ALARM;
			if(g_sys_para.batRemainPercent == 0){//�ŵ統��,����Ϊ0,�ֶ���Ϊ1
				g_sys_para.batRemainPercent = 1;
			}
        } else if(g_sys_para.batRemainPercent <= 20) { //��������20%
			DEBUG_PRINTF("%s: Percent <= 20 \r\n",__func__);
            g_sys_para.batLedStatus = BAT_LOW20;
			if(g_sys_para.batRemainPercent == 0){//�ŵ統��,����Ϊ0,�ֶ���Ϊ1
				g_sys_para.batRemainPercent = 1;
			}
        } else {
            g_sys_para.batLedStatus = BAT_NORMAL;
        }
        vTaskDelay(2000);
	}
}



