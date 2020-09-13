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
  * @brief   配置PWM1,输出ADC芯片采集时钟
  * @input
  * @return
***************************************************************************************/
void ADC_PwmClkConfig(uint32_t freq)
{
	CTIMER_GetPwmPeriodValue(freq, 50, CTIMER0_TICK_FREQ);
	CTIMER_SetupPwmPeriod(CTIMER0, CTIMER0_PWM_0_CHANNEL, g_pwmPeriod, g_pulsePeriod, false);
}

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void ADC_PwmClkStart(void)
{
	CTIMER_StartTimer(CTIMER0);
}

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void ADC_PwmClkStop(void)
{
    CTIMER_StopTimer(CTIMER0);
}

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void Filter_PwmClkConfig(uint32_t freq)
{
	CTIMER_GetPwmPeriodValue(freq, 50, CTIMER2_TICK_FREQ);
	CTIMER_SetupPwmPeriod(CTIMER2, CTIMER2_PWM_1_CHANNEL, g_pwmPeriod, g_pulsePeriod, false);
}

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void Filter_PwmClkStart(void)
{
	CTIMER_StartTimer(CTIMER2);
}


/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void Filter_PwmClkStop(void)
{
	CTIMER_StopTimer(CTIMER2);
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
#if 1
	SPI_ADCMasterTransfer(FLEXCOMM0_PERIPHERAL, &xfer);
	return ((spiRxBuff[0]<<16) | (spiRxBuff[1]<<8) | spiRxBuff[2]);
#else
	uint32_t spiData = 0;
	uint8_t  bitData[24] = {0};

	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[0] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];

	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[1] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[2] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[3] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[4] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[5] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[6] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[7] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[8] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[9] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[10] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[11] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[12] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[13] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[14] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[15] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[16] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[17] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[18] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[19] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[20] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[21] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[22] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,1);
	bitData[23] = GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN];
	
	//GPIO->B[BOARD_ADC_MISO_PORT][BOARD_ADC_MISO_PIN]
	
	GPIO_PinWrite(GPIO,BOARD_ADC_SPI_SCK_PORT,BOARD_ADC_SPI_SCK_PIN,0);

	for(uint8_t i=0;i<24;i++){
		spiData |= (bitData[i]<<(23-i));
	}
    return spiData;
#endif
}



