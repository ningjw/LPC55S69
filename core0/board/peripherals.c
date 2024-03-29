/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Peripherals v9.0
processor: LPC55S69
package_id: LPC55S69JEV98
mcu_data: ksdk2_0
processor_version: 9.0.2
functionalGroups:
- name: BOARD_InitPeripherals
  UUID: 6d318e78-2ce5-461a-85f3-e18610602d0a
  selectedCore: cm33_core0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'system'
- type_id: 'system_54b53072540eeeb8f8e9343e71f28176'
- global_system_definitions:
  - user_definitions: ''
  - user_includes: ''
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "peripherals.h"

/***********************************************************************************************************************
 * BOARD_InitPeripherals functional group
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * DMA0 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'DMA0'
- type: 'lpc_dma'
- mode: 'basic'
- custom_name_enabled: 'false'
- type_id: 'lpc_dma_c13ca997a68f2ca6c666916ba13db7d7'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'DMA0'
- config_sets:
  - fsl_dma:
    - dma_table:
      - 0: []
    - dma_channels:
      - 0:
        - apiMode: 'trans'
        - dma_channel:
          - channel_prefix_id: 'CH0'
          - DMA_source: 'kDma0RequestHashCrypt'
          - init_channel_priority: 'false'
          - dma_priority: 'kDMA_ChannelPriority0'
          - enable_custom_name: 'false'
        - peripheral_request: 'false'
        - init_trigger_config: 'false'
        - trigger_config:
          - type: 'kDMA_NoTrigger'
          - burst: 'kDMA_SingleTransfer'
          - wrap: 'kDMA_NoWrap'
        - trans_config:
          - init_callback: 'false'
          - callback_function: ''
          - callback_user_data: ''
        - tcd_config: []
        - allocateTCD: 'noncache'
        - initTCD: 'noTCDInit'
    - dma_interrupt_trans:
      - IRQn: 'DMA0_IRQn'
      - enable_priority: 'false'
      - priority: '0'
    - quick_selection: 'default'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

  /* Channel CH0 global variables */
dma_handle_t DMA0_CH0_Handle;

static void DMA0_init(void) {

  /* Channel CH0 initialization */
  /* Enable the DMA 0channel in the DMA */
  DMA_EnableChannel(DMA0_DMA_BASEADDR, DMA0_CH0_DMA_CHANNEL);
  /* Create the DMA DMA0_CH0_Handlehandle */
  DMA_CreateHandle(&DMA0_CH0_Handle, DMA0_DMA_BASEADDR, DMA0_CH0_DMA_CHANNEL);
}

/***********************************************************************************************************************
 * CTIMER0 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'CTIMER0'
- type: 'ctimer'
- mode: 'PWM'
- custom_name_enabled: 'false'
- type_id: 'ctimer_c8b90232d8b6318ba1dac2cf08fb5f4a'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'CTIMER0'
- config_sets:
  - fsl_ctimer:
    - ctimerConfig:
      - mode: 'kCTIMER_TimerMode'
      - clockSource: 'FunctionClock'
      - clockSourceFreq: 'BOARD_BootClockRUN'
      - timerPrescaler: '1'
    - EnableTimerInInit: 'false'
    - pwmConfig:
      - pwmPeriodValueStr: '96'
      - enableInterrupt: 'false'
      - pwmChannels:
        - 0:
          - pwmChannelPrefixId: 'PWM0'
          - pwmChannel: 'kCTIMER_Match_0'
          - pwmDutyValueStr: '48'
          - enableInterrupt: 'false'
    - interruptCallbackConfig:
      - interrupt:
        - IRQn: 'CTIMER0_IRQn'
        - enable_priority: 'false'
        - priority: '0'
      - callback: 'kCTIMER_NoCallback'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const ctimer_config_t CTIMER0_config = {
  .mode = kCTIMER_TimerMode,
  .input = kCTIMER_Capture_0,
  .prescale = 0
};

static void CTIMER0_init(void) {
  /* CTIMER0 peripheral initialization */
  CTIMER_Init(CTIMER0_PERIPHERAL, &CTIMER0_config);
  /* PWM channel 0 of CTIMER0 peripheral initialization */
  CTIMER_SetupPwmPeriod(CTIMER0_PERIPHERAL, CTIMER0_PWM0_CHANNEL, CTIMER0_PWM_PERIOD, CTIMER0_PWM0_DUTY, false);
}

/***********************************************************************************************************************
 * CTIMER2 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'CTIMER2'
- type: 'ctimer'
- mode: 'PWM'
- custom_name_enabled: 'false'
- type_id: 'ctimer_c8b90232d8b6318ba1dac2cf08fb5f4a'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'CTIMER2'
- config_sets:
  - fsl_ctimer:
    - ctimerConfig:
      - mode: 'kCTIMER_TimerMode'
      - clockSource: 'FunctionClock'
      - clockSourceFreq: 'BOARD_BootClockRUN'
      - timerPrescaler: '1'
    - EnableTimerInInit: 'false'
    - pwmConfig:
      - pwmPeriodValueStr: '96'
      - enableInterrupt: 'false'
      - pwmChannels:
        - 0:
          - pwmChannelPrefixId: 'PWM1'
          - pwmChannel: 'kCTIMER_Match_1'
          - pwmDutyValueStr: '48'
          - enableInterrupt: 'false'
    - interruptCallbackConfig:
      - interrupt:
        - IRQn: 'CTIMER1_IRQn'
        - enable_priority: 'false'
        - priority: '0'
      - callback: 'kCTIMER_NoCallback'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const ctimer_config_t CTIMER2_config = {
  .mode = kCTIMER_TimerMode,
  .input = kCTIMER_Capture_0,
  .prescale = 0
};

static void CTIMER2_init(void) {
  /* CTIMER2 peripheral initialization */
  CTIMER_Init(CTIMER2_PERIPHERAL, &CTIMER2_config);
  /* PWM channel 1 of CTIMER2 peripheral initialization */
  CTIMER_SetupPwmPeriod(CTIMER2_PERIPHERAL, CTIMER2_PWM1_CHANNEL, CTIMER2_PWM_PERIOD, CTIMER2_PWM1_DUTY, false);
}

/***********************************************************************************************************************
 * FLEXCOMM0 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'FLEXCOMM0'
- type: 'flexcomm_spi'
- mode: 'SPI_Polling'
- custom_name_enabled: 'false'
- type_id: 'flexcomm_spi_481dadba00035f986f31ed9ac95af181'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FLEXCOMM0'
- config_sets:
  - fsl_spi:
    - spi_mode: 'kSPI_Master'
    - clockSource: 'FXCOMFunctionClock'
    - clockSourceFreq: 'BOARD_BootClockRUN'
    - spi_master_config:
      - enableLoopback: 'false'
      - enableMaster: 'true'
      - polarity: 'kSPI_ClockPolarityActiveHigh'
      - phase: 'kSPI_ClockPhaseFirstEdge'
      - direction: 'kSPI_MsbFirst'
      - baudRate_Bps: '24000000'
      - dataWidth: 'kSPI_Data8Bits'
      - sselNum: 'kSPI_Ssel0'
      - sselPol_set: ''
      - txWatermark: 'kSPI_TxFifo0'
      - rxWatermark: 'kSPI_RxFifo1'
      - delayConfig:
        - preDelay: '0'
        - postDelay: '0'
        - frameDelay: '0'
        - transferDelay: '0'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const spi_master_config_t FLEXCOMM0_config = {
  .enableLoopback = false,
  .enableMaster = true,
  .polarity = kSPI_ClockPolarityActiveHigh,
  .phase = kSPI_ClockPhaseFirstEdge,
  .direction = kSPI_MsbFirst,
  .baudRate_Bps = 24000000UL,
  .dataWidth = kSPI_Data8Bits,
  .sselNum = kSPI_Ssel0,
  .sselPol = kSPI_SpolActiveAllLow,
  .txWatermark = kSPI_TxFifo0,
  .rxWatermark = kSPI_RxFifo1,
  .delayConfig = {
    .preDelay = 0U,
    .postDelay = 0U,
    .frameDelay = 0U,
    .transferDelay = 0U
  }
};

static void FLEXCOMM0_init(void) {
  RESET_PeripheralReset(kFC0_RST_SHIFT_RSTn);
  /* Initialization function */
  SPI_MasterInit(FLEXCOMM0_PERIPHERAL, &FLEXCOMM0_config, FLEXCOMM0_CLOCK_SOURCE);
}

/***********************************************************************************************************************
 * FLEXCOMM2 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'FLEXCOMM2'
- type: 'flexcomm_usart'
- mode: 'interrupts'
- custom_name_enabled: 'false'
- type_id: 'flexcomm_usart_c0a0c6d3d3ef57701b439b00070052a8'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FLEXCOMM2'
- config_sets:
  - interruptsCfg:
    - interrupts: 'kUSART_RxErrorInterruptEnable kUSART_RxLevelInterruptEnable'
    - interrupt_vectors:
      - enable_rx_tx_irq: 'true'
      - interrupt_rx_tx:
        - IRQn: 'FLEXCOMM2_IRQn'
        - enable_interrrupt: 'enabled'
        - enable_priority: 'false'
        - priority: '0'
        - enable_custom_name: 'false'
  - usartConfig_t:
    - usartConfig:
      - clockSource: 'FXCOMFunctionClock'
      - clockSourceFreq: 'BOARD_BootClockRUN'
      - baudRate_Bps: '115200'
      - syncMode: 'kUSART_SyncModeDisabled'
      - parityMode: 'kUSART_ParityDisabled'
      - stopBitCount: 'kUSART_OneStopBit'
      - bitCountPerChar: 'kUSART_8BitsPerChar'
      - loopback: 'false'
      - txWatermark: 'kUSART_TxFifo0'
      - rxWatermark: 'kUSART_RxFifo1'
      - enableRx: 'true'
      - enableTx: 'true'
      - clockPolarity: 'kUSART_RxSampleOnFallingEdge'
      - enableContinuousSCLK: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const usart_config_t FLEXCOMM2_config = {
  .baudRate_Bps = 115200UL,
  .syncMode = kUSART_SyncModeDisabled,
  .parityMode = kUSART_ParityDisabled,
  .stopBitCount = kUSART_OneStopBit,
  .bitCountPerChar = kUSART_8BitsPerChar,
  .loopback = false,
  .txWatermark = kUSART_TxFifo0,
  .rxWatermark = kUSART_RxFifo1,
  .enableRx = true,
  .enableTx = true,
  .clockPolarity = kUSART_RxSampleOnFallingEdge,
  .enableContinuousSCLK = false
};

static void FLEXCOMM2_init(void) {
  /* Reset FLEXCOMM device */
  RESET_PeripheralReset(kFC2_RST_SHIFT_RSTn);
  USART_Init(FLEXCOMM2_PERIPHERAL, &FLEXCOMM2_config, FLEXCOMM2_CLOCK_SOURCE);
  USART_EnableInterrupts(FLEXCOMM2_PERIPHERAL, kUSART_RxErrorInterruptEnable | kUSART_RxLevelInterruptEnable);
  /* Enable interrupt FLEXCOMM2_IRQn request in the NVIC. */
  EnableIRQ(FLEXCOMM2_FLEXCOMM_IRQN);
}

/***********************************************************************************************************************
 * FLEXCOMM5 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'FLEXCOMM5'
- type: 'flexcomm_usart'
- mode: 'interrupts'
- custom_name_enabled: 'false'
- type_id: 'flexcomm_usart_c0a0c6d3d3ef57701b439b00070052a8'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FLEXCOMM5'
- config_sets:
  - interruptsCfg:
    - interrupts: ''
    - interrupt_vectors:
      - enable_rx_tx_irq: 'false'
      - interrupt_rx_tx:
        - IRQn: 'FLEXCOMM5_IRQn'
        - enable_interrrupt: 'enabled'
        - enable_priority: 'false'
        - priority: '0'
        - enable_custom_name: 'false'
  - usartConfig_t:
    - usartConfig:
      - clockSource: 'FXCOMFunctionClock'
      - clockSourceFreq: 'BOARD_BootClockRUN'
      - baudRate_Bps: '115200'
      - syncMode: 'kUSART_SyncModeDisabled'
      - parityMode: 'kUSART_ParityDisabled'
      - stopBitCount: 'kUSART_OneStopBit'
      - bitCountPerChar: 'kUSART_8BitsPerChar'
      - loopback: 'false'
      - txWatermark: 'kUSART_TxFifo0'
      - rxWatermark: 'kUSART_RxFifo1'
      - enableRx: 'false'
      - enableTx: 'true'
      - clockPolarity: 'kUSART_RxSampleOnFallingEdge'
      - enableContinuousSCLK: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const usart_config_t FLEXCOMM5_config = {
  .baudRate_Bps = 115200UL,
  .syncMode = kUSART_SyncModeDisabled,
  .parityMode = kUSART_ParityDisabled,
  .stopBitCount = kUSART_OneStopBit,
  .bitCountPerChar = kUSART_8BitsPerChar,
  .loopback = false,
  .txWatermark = kUSART_TxFifo0,
  .rxWatermark = kUSART_RxFifo1,
  .enableRx = false,
  .enableTx = true,
  .clockPolarity = kUSART_RxSampleOnFallingEdge,
  .enableContinuousSCLK = false
};

static void FLEXCOMM5_init(void) {
  /* Reset FLEXCOMM device */
  RESET_PeripheralReset(kFC5_RST_SHIFT_RSTn);
  USART_Init(FLEXCOMM5_PERIPHERAL, &FLEXCOMM5_config, FLEXCOMM5_CLOCK_SOURCE);
  USART_EnableInterrupts(FLEXCOMM5_PERIPHERAL, 0);
}

/***********************************************************************************************************************
 * FLEXCOMM6 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'FLEXCOMM6'
- type: 'flexcomm_spi'
- mode: 'SPI_Polling'
- custom_name_enabled: 'false'
- type_id: 'flexcomm_spi_481dadba00035f986f31ed9ac95af181'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FLEXCOMM6'
- config_sets:
  - fsl_spi:
    - spi_mode: 'kSPI_Master'
    - clockSource: 'FXCOMFunctionClock'
    - clockSourceFreq: 'BOARD_BootClockRUN'
    - spi_master_config:
      - enableLoopback: 'false'
      - enableMaster: 'true'
      - polarity: 'kSPI_ClockPolarityActiveHigh'
      - phase: 'kSPI_ClockPhaseFirstEdge'
      - direction: 'kSPI_MsbFirst'
      - baudRate_Bps: '12000000'
      - dataWidth: 'kSPI_Data8Bits'
      - sselNum: 'kSPI_Ssel0'
      - sselPol_set: ''
      - txWatermark: 'kSPI_TxFifo0'
      - rxWatermark: 'kSPI_RxFifo1'
      - delayConfig:
        - preDelay: '0'
        - postDelay: '0'
        - frameDelay: '0'
        - transferDelay: '0'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const spi_master_config_t FLEXCOMM6_config = {
  .enableLoopback = false,
  .enableMaster = true,
  .polarity = kSPI_ClockPolarityActiveHigh,
  .phase = kSPI_ClockPhaseFirstEdge,
  .direction = kSPI_MsbFirst,
  .baudRate_Bps = 12000000UL,
  .dataWidth = kSPI_Data8Bits,
  .sselNum = kSPI_Ssel0,
  .sselPol = kSPI_SpolActiveAllLow,
  .txWatermark = kSPI_TxFifo0,
  .rxWatermark = kSPI_RxFifo1,
  .delayConfig = {
    .preDelay = 0U,
    .postDelay = 0U,
    .frameDelay = 0U,
    .transferDelay = 0U
  }
};

static void FLEXCOMM6_init(void) {
  RESET_PeripheralReset(kFC6_RST_SHIFT_RSTn);
  /* Initialization function */
  SPI_MasterInit(FLEXCOMM6_PERIPHERAL, &FLEXCOMM6_config, FLEXCOMM6_CLOCK_SOURCE);
}

/***********************************************************************************************************************
 * RTC initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'RTC'
- type: 'lpc_rtc'
- mode: 'general'
- custom_name_enabled: 'false'
- type_id: 'lpc_rtc_607bd7331c2c81c0037fe4624be881b6'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'RTC'
- config_sets:
  - fsl_rtc:
    - rtc_config:
      - setDateTime: 'false'
      - setAlarmTime: 'false'
      - setWakeup: 'false'
      - alarm_wake_up_enable: 'false'
      - wake_up_enable: 'false'
      - start: 'true'
    - rtc_interrupt:
      - interrupt_vectors:
        - enable_irq: 'false'
        - interrupt:
          - IRQn: 'RTC_IRQn'
          - enable_interrrupt: 'enabled'
          - enable_priority: 'false'
          - priority: '0'
          - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

static void RTC_init(void) {
  /* RTC initialization */
  RTC_Init(RTC_PERIPHERAL);
  /* Start RTC timer */
  RTC_StartTimer(RTC_PERIPHERAL);
}

/***********************************************************************************************************************
 * NVIC initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'NVIC'
- type: 'nvic'
- mode: 'general'
- custom_name_enabled: 'false'
- type_id: 'nvic_57b5eef3774cc60acaede6f5b8bddc67'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'NVIC'
- config_sets:
  - nvic:
    - interrupt_table:
      - 0: []
      - 1: []
      - 2: []
    - interrupts: []
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/* Empty initialization function (commented out)
static void NVIC_init(void) {
} */

/***********************************************************************************************************************
 * UTICK0 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'UTICK0'
- type: 'utick'
- mode: 'general_config'
- custom_name_enabled: 'false'
- type_id: 'utick_58a3a3f691b03a130cd9419552f8327d'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'UTICK0'
- config_sets:
  - fsl_utick:
    - clockSettingUTICK:
      - clockSource: 'FunctionClock'
      - clockSourceFreq: 'BOARD_BootClockRUN'
    - timerSettingUTICK:
      - utick_mode_t: 'kUTICK_Onetime'
      - startTimer: 'true'
      - timerValueStr: '1000000'
      - callbackEnable: 'false'
    - interrupt:
      - IRQn: 'UTICK0_IRQn'
      - enable_priority: 'false'
      - priority: '0'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

static void UTICK0_init(void) {
  /* UTICK0 peripheral initialization */
  UTICK_Init(UTICK0_PERIPHERAL);
  /* Configuration of UTICK0 peripheral initialization */
  UTICK_SetTick(UTICK0_PERIPHERAL, UTICK0_MODE, UTICK0_TICKS, NULL);
}

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
  /* Global initialization */
  DMA_Init(DMA0_DMA_BASEADDR);

  /* Initialize components */
  DMA0_init();
  CTIMER0_init();
  CTIMER2_init();
  FLEXCOMM0_init();
  FLEXCOMM2_init();
  FLEXCOMM5_init();
  FLEXCOMM6_init();
  RTC_init();
  UTICK0_init();
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
}
