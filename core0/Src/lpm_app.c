#include "main.h"

static void BOARD_BootClockFRO12M(void)
{
    /*!< Set up the clock sources */
    /*!< Configure FRO192M */
    POWER_DisablePD(kPDRUNCFG_PD_FRO192M); /*!< Ensure FRO is on  */
    CLOCK_SetupFROClocking(12000000U);     /*!< Set up FRO to the 12 MHz, just for sure */
    CLOCK_AttachClk(kFRO12M_to_MAIN_CLK);  /*!< Switch to FRO 12MHz first to ensure we can change the clock setting */

    CLOCK_SetupFROClocking(96000000U); /* Enable FRO HF(96MHz) output */

    POWER_SetVoltageForFreq(
        12000000U); /*!< Set voltage for the one of the fastest clock outputs: System clock output */
    CLOCK_SetFLASHAccessCyclesForFreq(12000000U); /*!< Set FLASH wait states for core */
	
    /*!< Set up dividers */
    CLOCK_SetClkDiv(kCLOCK_DivAhbClk, 1U, false); /*!< Set AHBCLKDIV divider to value 1 */

    /*!< Set up clock selectors - Attach clocks to the peripheries */
    CLOCK_AttachClk(kFRO12M_to_MAIN_CLK); /*!< Switch MAIN_CLK to FRO12M */
	
	SystemCoreClock = 12000000U;
}

static void BOARD_BootClockFROHF96M(void)
{
    /*!< Set up the clock sources */
    /*!< Configure FRO192M */
    POWER_DisablePD(kPDRUNCFG_PD_FRO192M); /*!< Ensure FRO is on  */
    CLOCK_SetupFROClocking(12000000U);     /*!< Set up FRO to the 12 MHz, just for sure */
    CLOCK_AttachClk(kFRO12M_to_MAIN_CLK);  /*!< Switch to FRO 12MHz first to ensure we can change the clock setting */

    CLOCK_SetupFROClocking(96000000U); /* Enable FRO HF(96MHz) output */

    POWER_SetVoltageForFreq(
        96000000U); /*!< Set voltage for the one of the fastest clock outputs: System clock output */
    CLOCK_SetFLASHAccessCyclesForFreq(96000000U); /*!< Set FLASH wait states for core */

    /*!< Set up dividers */
    CLOCK_SetClkDiv(kCLOCK_DivAhbClk, 1U, false); /*!< Set AHBCLKDIV divider to value 1 */

    /*!< Set up clock selectors - Attach clocks to the peripheries */
    CLOCK_AttachClk(kFRO_HF_to_MAIN_CLK); /*!< Switch MAIN_CLK to FRO_HF */

    /*< Set SystemCoreClock variable. */
    SystemCoreClock = 96000000U;
}


static void BOARD_InitPinsOnSleep(void)
{
    gpio_pin_config_t pin_config_low = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U
    };
	for(uint8_t port = 0; port<2; port++)
	{
		for(uint8_t pin = 0; pin<32; pin++)
		{
			//仿真器接口 TMP101NA的SCL/SDA引脚  以及W25Q128JVPIQTR的CS引脚
			if((port == 0) && (pin>=11 && pin <= 15)){
				continue;
			}
			
			if((port == 1) && (pin == 15 || pin==16 || pin==17 || pin==21 || pin==22)){
				continue;
			}
			/* Initialize GPIO functionality on pin i  */
			GPIO_PinInit(GPIO, port, pin, &pin_config_low);
			IOCON->PIO[port][pin] = ((IOCON->PIO[port][pin] &
							  /* Mask bits to zero which are setting */
							  (~(IOCON_PIO_FUNC_MASK | IOCON_PIO_DIGIMODE_MASK)))
								
							 /* Selects pin function.*/
							 | IOCON_PIO_FUNC(0x00u));
			__nop();
		}
	}
}


void SystemSleep(void)
{
	DEBUG_PRINTF("enter deep sleep\n");
	
	RTC_GetDatetime(RTC, &sysTime);
	DEBUG_PRINTF("Date&Time: %d-%02d-%02d %02d:%02d:%02d\r\n",
	            sysTime.year,sysTime.month,sysTime.day,
	            sysTime.hour,sysTime.minute,sysTime.second);
	
	/* Enable RTC alarm interrupt */
    RTC_EnableInterrupts(RTC, kRTC_AlarmInterruptEnable);
	
	/* Enable at the NVIC */
    EnableIRQ(RTC_IRQn);
	
	/* Set alarm time in seconds */
    RTC->MATCH = RTC->COUNT + 60 * g_sample_para.sampleInterval;
	
	/* Get alarm time */
	rtc_datetime_t alarmDate;
    RTC_GetAlarm(RTC, &alarmDate);
	DEBUG_PRINTF("System will wakeup at: %d-%02d-%02d %02d:%02d:%02d\r\n",
	            alarmDate.year,alarmDate.month,alarmDate.day,
	            alarmDate.hour,alarmDate.minute,alarmDate.second);
	
	BOARD_InitPinsOnSleep();
	BOARD_BootClockFRO12M();//深度睡眠模式下,系统时钟需要切换为内部的12M时钟, 否则无法唤醒
#ifdef CAT1_VERSION
	//配置模块通过RTC唤醒
	POWER_EnterDeepSleep(kPDRUNCFG_PD_FRO32K , 0, WAKEUP_RTC_LITE_ALARM_WAKEUP, 0x1U);
#else
	POWER_EnterDeepSleep(EXCLUDE_PD, 0x7FFF, WAKEUP_FLEXCOMM3, 1);
#endif
//    NVIC_SystemReset();
	BOARD_BootClockFROHF96M();
	BOARD_InitPins();
	DEBUG_PRINTF("exit deep sleep\n");
    //系统唤醒后需要进行一次采样
    xTaskNotify(ADC_TaskHandle, EVT_SAMPLE_START, eSetBits);
}


void RTC_IRQHandler(void)
{
	RTC_ClearStatusFlags(RTC, RTC_CTRL_ALARM1HZ_MASK);
	DEBUG_PRINTF("RTC_IRQHandler\n");
}
