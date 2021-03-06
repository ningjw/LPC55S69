/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "fsl_dma.h"
#include "fsl_common.h"
#include "fsl_lpadc.h"
#include "fsl_ctimer.h"
#include "fsl_clock.h"
#include "fsl_spi.h"
#include "fsl_reset.h"
#include "fsl_usart.h"
#include "fsl_rtc.h"
#include "fsl_utick.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
/* Definitions for BOARD_InitPeripherals functional group */
/* Used DMA device. */
#define DMA0_DMA_BASEADDR DMA0
/* DMA0 interrupt vector ID (number). */
#define DMA0_IRQN DMA0_IRQn

  /* Channel CH0 definitions */
/* Selected DMA channel number. */
#define DMA0_CH0_DMA_CHANNEL 0
/* Alias for ADC0 peripheral */
#define ADC0_PERIPHERAL ADC0
/* Definition of peripheral ID */
#define CTIMER0_PERIPHERAL CTIMER0
/* Timer tick frequency in Hz (input frequency of the timer) */
#define CTIMER0_TICK_FREQ 96000000UL
/* Timer tick period in ns (input period of the timer) */
#define CTIMER0_TICK_PERIOD 10UL
/* Definition of PWM period */
#define CTIMER0_PWM_PERIOD 96
/* Definition of channel 0 ID */
#define CTIMER0_PWM0_CHANNEL kCTIMER_Match_0
/* Definition of channel 0 duty */
#define CTIMER0_PWM0_DUTY 48
/* Definition of peripheral ID */
#define CTIMER2_PERIPHERAL CTIMER2
/* Timer tick frequency in Hz (input frequency of the timer) */
#define CTIMER2_TICK_FREQ 96000000UL
/* Timer tick period in ns (input period of the timer) */
#define CTIMER2_TICK_PERIOD 10UL
/* Definition of PWM period */
#define CTIMER2_PWM_PERIOD 96
/* Definition of channel 1 ID */
#define CTIMER2_PWM1_CHANNEL kCTIMER_Match_1
/* Definition of channel 1 duty */
#define CTIMER2_PWM1_DUTY 48
/* Definition of peripheral ID */
#define CTIMER3_PERIPHERAL CTIMER3
/* Timer tick frequency in Hz (input frequency of the timer) */
#define CTIMER3_TICK_FREQ 1UL
/* Timer tick period in ns (input period of the timer) */
#define CTIMER3_TICK_PERIOD 1000000000UL
/* Definition of channel 0 ID */
#define CTIMER3_MATCH0_CHANNEL kCTIMER_Match_0
/* CTIMER3 interrupt vector ID (number). */
#define CTIMER3_TIMER_IRQN CTIMER3_IRQn
/* BOARD_InitPeripherals defines for FLEXCOMM0 */
/* Definition of peripheral ID */
#define FLEXCOMM0_PERIPHERAL ((SPI_Type *)FLEXCOMM0)
/* Definition of the clock source frequency */
#define FLEXCOMM0_CLOCK_SOURCE 48000000UL
/* Definition of peripheral ID */
#define FLEXCOMM2_PERIPHERAL ((USART_Type *)FLEXCOMM2)
/* Definition of the clock source frequency */
#define FLEXCOMM2_CLOCK_SOURCE 48000000UL
/* FLEXCOMM2 interrupt vector ID (number). */
#define FLEXCOMM2_FLEXCOMM_IRQN FLEXCOMM2_IRQn
/* FLEXCOMM2 interrupt handler identifier. */
#define FLEXCOMM2_FLEXCOMM_IRQHANDLER FLEXCOMM2_IRQHandler
/* Definition of peripheral ID */
#define FLEXCOMM5_PERIPHERAL ((USART_Type *)FLEXCOMM5)
/* Definition of the clock source frequency */
#define FLEXCOMM5_CLOCK_SOURCE 48000000UL
/* FLEXCOMM5 interrupt vector ID (number). */
#define FLEXCOMM5_FLEXCOMM_IRQN FLEXCOMM5_IRQn
/* FLEXCOMM5 interrupt handler identifier. */
#define FLEXCOMM5_FLEXCOMM_IRQHANDLER FLEXCOMM5_IRQHandler
/* BOARD_InitPeripherals defines for FLEXCOMM6 */
/* Definition of peripheral ID */
#define FLEXCOMM6_PERIPHERAL ((SPI_Type *)FLEXCOMM6)
/* Definition of the clock source frequency */
#define FLEXCOMM6_CLOCK_SOURCE 48000000UL
/* Definition of peripheral ID */
#define RTC_PERIPHERAL RTC
/* BOARD_InitPeripherals defines for UTICK0 */
/* Definition of peripheral ID */
#define UTICK0_PERIPHERAL UTICK0
/* Definition of kUTICK_Repeat operational mode */
#define UTICK0_MODE kUTICK_Repeat
/* Timer tick frequency in Hz (input frequency of the timer) */
#define UTICK0_TICK_FREQ 1000000UL
/* Timer tick period in ns (input period of the timer) */
#define UTICK0_TICK_PERIOD 1000UL
/* Definition of timer value in ticks */
#define UTICK0_TICKS 9999999UL
/* UTICK0 interrupt vector ID (number). */
#define UTICK0_IRQN UTICK0_IRQn

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
extern dma_handle_t DMA0_CH0_Handle;
extern const lpadc_config_t ADC0_config;
extern lpadc_conv_command_config_t ADC0_commandsConfig[1];
extern lpadc_conv_trigger_config_t ADC0_triggersConfig[1];
extern const ctimer_config_t CTIMER0_config;
extern const ctimer_config_t CTIMER2_config;
extern const ctimer_config_t CTIMER3_config;
extern const ctimer_match_config_t CTIMER3_Match0_config;
extern const spi_master_config_t FLEXCOMM0_config;
extern const usart_config_t FLEXCOMM2_config;
extern const usart_config_t FLEXCOMM5_config;
extern const spi_master_config_t FLEXCOMM6_config;

/***********************************************************************************************************************
 * Callback functions
 **********************************************************************************************************************/
/* UTICK0 callback function */
extern void UTICK0_Callback(void);

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/

void BOARD_InitPeripherals(void);

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void);

#if defined(__cplusplus)
}
#endif

#endif /* _PERIPHERALS_H_ */
