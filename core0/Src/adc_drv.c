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
	CTIMER_SetupPwmPeriod(CTIMER0, CTIMER2_PWM_1_CHANNEL, g_pwmPeriod, g_pulsePeriod, false);
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
  * @brief
  * @input
  * @return
***************************************************************************************/
uint32_t ADS1271_ReadData(void)
{
    uint32_t spiData = 0;
	for( uint8_t i; i<3; i++){
		while((FLEXCOMM0_PERIPHERAL->FIFOSTAT & SPI_FIFOSTAT_TXEMPTY_MASK)==0);	//等待发送fifo为空,从而可以发送数据	
		FLEXCOMM0_PERIPHERAL->FIFOWR = 0xFFFFFFFF;		 //发送数据
		while((FLEXCOMM0_PERIPHERAL->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK)==0);//等待接收fifo不为空,从而可以读取数据
		spiData = (spiData<<8) | FLEXCOMM0_PERIPHERAL->FIFORD;			//读取数据
	}
	return spiData;					//返回接收到的数据 		    
}



