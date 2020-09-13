/* --------------------------------------------------------------------------
 *
 *
 *---------------------------------------------------------------------------*/
#include "main.h"

volatile uint32_t g_pwmPeriod   = 0U;
volatile uint32_t g_pulsePeriod = 0U;
extern void CTIMER1_Callback(uint32_t flags);
ctimer_callback_t CTIMER1_callback[] = {CTIMER1_Callback};
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

/***************************************************************************************
  * @brief   用于捕获转速信号周期
  * @input   
  * @return  
***************************************************************************************/
void CTIMER1_Init(void)
{
	ctimer_config_t config;
	CTIMER_GetDefaultConfig(&config);
    config.input = kCTIMER_Capture_0;
    CTIMER_Init(CTIMER1, &config);
    CTIMER_RegisterCallBack(CTIMER1, CTIMER1_callback, kCTIMER_SingleCallback);
    CTIMER_SetupCapture(CTIMER1, kCTIMER_Capture_0, kCTIMER_Capture_BothEdge, true);
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



