/* --------------------------------------------------------------------------
 *
 *
 *---------------------------------------------------------------------------*/
#include "main.h"


volatile uint32_t g_pwmPeriod   = 0U;
volatile uint32_t g_pulsePeriod = 0U;

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
	/* PWM channel 0 of CTIMER0 peripheral initialization: ADCÊ±ÖÓ*/
	CTIMER_GetPwmPeriodValue(adcFreq, 50, CTIMER0_TICK_FREQ);
	CTIMER_SetupPwmPeriod(CTIMER0_PERIPHERAL, CTIMER0_PWM0_CHANNEL, g_pwmPeriod, g_pulsePeriod, false);
	CTIMER_StartTimer(CTIMER0_PERIPHERAL);
	
	/* PWM channel 1 of CTIMER2 peripheral initialization: ÂË²¨Æ÷Ê±ÖÓ*/
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



