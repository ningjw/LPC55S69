/* --------------------------------------------------------------------------
 *
 *
 *---------------------------------------------------------------------------*/
#include "main.h"


volatile uint32_t g_pwmPeriod   = 0U;
volatile uint32_t g_pulsePeriod = 0U;

#define DEMO_LPADC_USER_CHANNEL 13U
#define DEMO_LPADC_USER_CMDID   1U /* CMD1 */
#define DEMO_LPADC_VREF_SOURCE  kLPADC_ReferenceVoltageAlt2

TaskHandle_t BAT_TaskHandle = NULL;  /* 电池管理任务句柄 */
uint8_t status = 0;
lpadc_conv_result_t         mLpadcResult;

/*******************************************************************************
 * Code
 ******************************************************************************/
status_t CTIMER_GetPwmPeriodValue(uint32_t pwmFreqHz, uint8_t dutyCyclePercent, uint32_t timerClock_Hz)
{
    /* Calculate PWM period match value */
    g_pwmPeriod = (timerClock_Hz / pwmFreqHz) - 1;

    /* Calculate pulse width match value */
    if (dutyCyclePercent == 0)
    {
        g_pulsePeriod = g_pwmPeriod + 1;
    }
    else
    {
        g_pulsePeriod = (g_pwmPeriod * (100 - dutyCyclePercent)) / 100;
    }
    return kStatus_Success;
}


void ADC_PwmClkStart(uint32_t adcFreq, uint32_t ltcFreq)
{
	/* PWM channel 0 of CTIMER0 peripheral initialization: ADC时钟*/
	CTIMER_GetPwmPeriodValue(adcFreq, 50, CTIMER0_TICK_FREQ);
	CTIMER_SetupPwmPeriod(CTIMER0_PERIPHERAL, CTIMER0_PWM0_CHANNEL, g_pwmPeriod, g_pulsePeriod, false);
	CTIMER_StartTimer(CTIMER0_PERIPHERAL);
	
	/* PWM channel 1 of CTIMER2 peripheral initialization: 滤波器时钟*/
	CTIMER_GetPwmPeriodValue(ltcFreq, 50, CTIMER2_TICK_FREQ);
	CTIMER_SetupPwmPeriod(CTIMER2_PERIPHERAL, CTIMER2_PWM1_CHANNEL, g_pwmPeriod, g_pulsePeriod, false);
	CTIMER_StartTimer(CTIMER2_PERIPHERAL);
}


void ADC_PwmClkStop(void)
{
	CTIMER_StopTimer(CTIMER0_PERIPHERAL);
	CTIMER_StopTimer(CTIMER2_PERIPHERAL);
}

uint8_t spiTxBuff[4] = {0xFF};
uint8_t spiRxBuff[4] = {0};
spi_transfer_t xfer = {
	.txData = spiTxBuff,
	.rxData = spiRxBuff,
	.dataSize = 3,
	.configFlags = kSPI_FrameAssert,
};
/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
uint32_t ADS1271_ReadData(void)
{
	SPI_ADCMasterTransfer(FLEXCOMM0_PERIPHERAL, &xfer);
	return ((spiRxBuff[0]<<16) | (spiRxBuff[1]<<8) | spiRxBuff[2]);
}


void LPC55S69_AdcInit(void)
{
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
}

void LPC55S69_BatAdcUpdate(void)
{
    uint8_t retry = 0;
	LPADC_DoSoftwareTrigger(ADC0, 1U); /* 1U is trigger0 mask. */
	
	while (!LPADC_GetConvResult(ADC0, &mLpadcResult, 0U)) {
        if(retry++ > 10){
            return;
        }
		vTaskDelay(1);
	}
	g_sys_para.batVoltage = (67793.0 - 3.5028 * mLpadcResult.convValue) / 10000.0;
	//根据电压计算电池容量
	if(g_sys_para.batVoltage >= 3.73f) { //(3.73 - 4.2)
		g_sys_para.batRemainPercent = -308.19f * g_sys_para.batVoltage * g_sys_para.batVoltage + 2607.7f * g_sys_para.batVoltage - 5417.9f;
	} else if(g_sys_para.batVoltage >= 3.68f) { //(3.68 - 3.73)
		g_sys_para.batRemainPercent = -1666.7f * g_sys_para.batVoltage * g_sys_para.batVoltage + 12550 * g_sys_para.batVoltage - 23603;
	} else { // (3.5 - 3.68)
		g_sys_para.batRemainPercent = 55.556f * g_sys_para.batVoltage - 194.44f;
	}
}



