/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Peripherals v7.0
processor: LPC55S69
package_id: LPC55S69JEV98
mcu_data: ksdk2_0
processor_version: 7.0.1
functionalGroups:
- name: BOARD_InitPeripherals
  UUID: 6d318e78-2ce5-461a-85f3-e18610602d0a
  called_from_default_init: true
  selectedCore: cm33_core0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'system'
- type_id: 'system_54b53072540eeeb8f8e9343e71f28176'
- global_system_definitions: []
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'msg'
- type_id: 'msg_6e2baaf3b97dbeef01c0043275f9a0e7'
- global_messages: []
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

void DMA0_init(void) {

  /* Channel CH0 initialization */
  /* Enable the DMA 0channel in the DMA */
  DMA_EnableChannel(DMA0_DMA_BASEADDR, DMA0_CH0_DMA_CHANNEL);
  /* Create the DMA DMA0_CH0_Handlehandle */
  DMA_CreateHandle(&DMA0_CH0_Handle, DMA0_DMA_BASEADDR, DMA0_CH0_DMA_CHANNEL);
}

/***********************************************************************************************************************
 * CTIMER1 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'CTIMER1'
- type: 'ctimer'
- mode: 'Capture_Match'
- custom_name_enabled: 'false'
- type_id: 'ctimer_c8b90232d8b6318ba1dac2cf08fb5f4a'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'CTIMER1'
- config_sets:
  - fsl_ctimer:
    - ctimerConfig:
      - mode: 'kCTIMER_TimerMode'
      - clockSource: 'FunctionClock'
      - clockSourceFreq: 'BOARD_BootClockRUN'
      - timerPrescaler: '1'
    - EnableTimerInInit: 'false'
    - matchChannels: []
    - interruptCallbackConfig:
      - interrupt:
        - IRQn: 'CTIMER1_IRQn'
        - enable_priority: 'false'
        - priority: '0'
      - callback: 'kCTIMER_SingleCallback'
      - singleCallback: 'CTIMER1_Callback'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const ctimer_config_t CTIMER1_config = {
  .mode = kCTIMER_TimerMode,
  .input = kCTIMER_Capture_0,
  .prescale = 0
};

void CTIMER1_init(void) {
  /* CTIMER1 peripheral initialization */
  CTIMER_Init(CTIMER1_PERIPHERAL, &CTIMER1_config);
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
const spi_master_config_t FLEXCOMM0_config = {
  .enableLoopback = false,
  .enableMaster = true,
  .polarity = kSPI_ClockPolarityActiveHigh,
  .phase = kSPI_ClockPhaseFirstEdge,
  .direction = kSPI_MsbFirst,
  .baudRate_Bps = 12000000,
  .dataWidth = kSPI_Data8Bits,
  .sselNum = kSPI_Ssel0,
  .sselPol = kSPI_SpolActiveAllLow,
  .txWatermark = kSPI_TxFifo0,
  .rxWatermark = kSPI_RxFifo1,
  .delayConfig = {
    .preDelay = 0,
    .postDelay = 0,
    .frameDelay = 0,
    .transferDelay = 0
  }
};

void FLEXCOMM0_init(void) {
  RESET_PeripheralReset(kFC0_RST_SHIFT_RSTn);
  /* Initialization function */
  SPI_MasterInit(FLEXCOMM0_PERIPHERAL, &FLEXCOMM0_config, FLEXCOMM0_CLOCK_SOURCE);
}

/***********************************************************************************************************************
 * FLEXCOMM1 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'FLEXCOMM1'
- type: 'flexcomm_i2c'
- mode: 'I2C_Polling'
- custom_name_enabled: 'false'
- type_id: 'flexcomm_i2c_567d1a9d97c12e5d39b00259c3436dc4'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FLEXCOMM1'
- config_sets:
  - fsl_i2c:
    - i2c_mode: 'kI2C_Master'
    - clockSource: 'FXCOMFunctionClock'
    - clockSourceFreq: 'BOARD_BootClockRUN'
    - i2c_master_config:
      - enableMaster: 'true'
      - baudRate_Bps: '100000'
      - enableTimeout: 'false'
    - quick_selection: 'QS_I2C_Master'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const i2c_master_config_t FLEXCOMM1_config = {
  .enableMaster = true,
  .baudRate_Bps = 100000,
  .enableTimeout = false
};

void FLEXCOMM1_init(void) {
  RESET_PeripheralReset( kFC1_RST_SHIFT_RSTn);
  /* Initialization function */
  I2C_MasterInit(FLEXCOMM1_PERIPHERAL, &FLEXCOMM1_config, FLEXCOMM1_CLOCK_SOURCE);
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
        - enable_priority: 'false'
        - priority: '0'
        - enable_custom_name: 'false'
  - usartConfig_t:
    - usartConfig:
      - clockSource: 'FXCOMFunctionClock'
      - clockSourceFreq: 'BOARD_BootClockRUN'
      - baudRate_Bps: '9600'
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
  .baudRate_Bps = 9600,
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

void FLEXCOMM2_init(void) {
  /* Reset FLEXCOMM device */
  RESET_PeripheralReset(kFC2_RST_SHIFT_RSTn);
  USART_Init(FLEXCOMM2_PERIPHERAL, &FLEXCOMM2_config, FLEXCOMM2_CLOCK_SOURCE);
  USART_EnableInterrupts(FLEXCOMM2_PERIPHERAL, kUSART_RxErrorInterruptEnable | kUSART_RxLevelInterruptEnable);
  /* Enable interrupt FLEXCOMM2_IRQn request in the NVIC */
  EnableIRQ(FLEXCOMM2_FLEXCOMM_IRQN);
}

/***********************************************************************************************************************
 * FLEXCOMM3 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'FLEXCOMM3'
- type: 'flexcomm_usart'
- mode: 'interrupts'
- custom_name_enabled: 'false'
- type_id: 'flexcomm_usart_c0a0c6d3d3ef57701b439b00070052a8'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FLEXCOMM3'
- config_sets:
  - interruptsCfg:
    - interrupts: 'kUSART_RxErrorInterruptEnable kUSART_RxLevelInterruptEnable'
    - interrupt_vectors:
      - enable_rx_tx_irq: 'true'
      - interrupt_rx_tx:
        - IRQn: 'FLEXCOMM3_IRQn'
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
const usart_config_t FLEXCOMM3_config = {
  .baudRate_Bps = 115200,
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

void FLEXCOMM3_init(void) {
  /* Reset FLEXCOMM device */
  RESET_PeripheralReset(kFC3_RST_SHIFT_RSTn);
  USART_Init(FLEXCOMM3_PERIPHERAL, &FLEXCOMM3_config, FLEXCOMM3_CLOCK_SOURCE);
  USART_EnableInterrupts(FLEXCOMM3_PERIPHERAL, kUSART_RxErrorInterruptEnable | kUSART_RxLevelInterruptEnable);
  /* Enable interrupt FLEXCOMM3_IRQn request in the NVIC */
  EnableIRQ(FLEXCOMM3_FLEXCOMM_IRQN);
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
  .baudRate_Bps = 12000000,
  .dataWidth = kSPI_Data8Bits,
  .sselNum = kSPI_Ssel0,
  .sselPol = kSPI_SpolActiveAllLow,
  .txWatermark = kSPI_TxFifo0,
  .rxWatermark = kSPI_RxFifo1,
  .delayConfig = {
    .preDelay = 0,
    .postDelay = 0,
    .frameDelay = 0,
    .transferDelay = 0
  }
};

void FLEXCOMM6_init(void) {
  RESET_PeripheralReset(kFC6_RST_SHIFT_RSTn);
  /* Initialization function */
  SPI_MasterInit(FLEXCOMM6_PERIPHERAL, &FLEXCOMM6_config, FLEXCOMM6_CLOCK_SOURCE);
}

/***********************************************************************************************************************
 * PINT initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'PINT'
- type: 'pint'
- mode: 'interrupt_mode'
- custom_name_enabled: 'false'
- type_id: 'pint_cf4a806bb2a6c1ffced58ae2ed7b43af'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'PINT'
- config_sets:
  - general:
    - interrupt_array:
      - 0:
        - interrupt_id: 'INT_1'
        - interrupt_selection: 'PINT.1'
        - interrupt_type: 'kPINT_PinIntEnableBothEdges'
        - callback_function: 'PINT1_CallBack'
        - enable_callback: 'true'
        - interrupt:
          - IRQn: 'PIN_INT1_IRQn'
          - enable_priority: 'false'
          - priority: '0'
      - 1:
        - interrupt_id: 'INT_2'
        - interrupt_selection: 'PINT.2'
        - interrupt_type: 'kPINT_PinIntEnableNone'
        - callback_function: 'PINT2_CallBack'
        - enable_callback: 'false'
        - interrupt:
          - IRQn: 'PIN_INT0_IRQn'
          - enable_priority: 'false'
          - priority: '0'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

void PINT_init(void) {
  /* PINT initiation  */
  PINT_Init(PINT_PERIPHERAL);
  /* PINT PINT.1 configuration */
  PINT_PinInterruptConfig(PINT_PERIPHERAL, PINT_INT_1, kPINT_PinIntEnableBothEdges, PINT1_CallBack);
  /* PINT PINT.2 configuration */
  PINT_PinInterruptConfig(PINT_PERIPHERAL, PINT_INT_2, kPINT_PinIntEnableNone, PINT2_CallBack);
  /* Enable PINT PINT.1 callback */
  PINT_EnableCallbackByIndex(PINT_PERIPHERAL, kPINT_PinInt1);
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
          - enable_priority: 'false'
          - priority: '0'
          - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

void RTC_init(void) {
  /* RTC initialization */
  RTC_Init(RTC_PERIPHERAL);
  /* Start RTC timer */
  RTC_StartTimer(RTC_PERIPHERAL);
}

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
      - utick_mode_t: 'kUTICK_Repeat'
      - startTimer: 'true'
      - timerValueStr: '1000000'
      - callbackEnable: 'true'
      - utick_callback_t: 'UTICK0_Callback'
    - interrupt:
      - IRQn: 'UTICK0_IRQn'
      - enable_priority: 'false'
      - priority: '0'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

void UTICK0_init(void) {
  /* UTICK0 peripheral initialization */
  UTICK_Init(UTICK0_PERIPHERAL);
  /* Configuration of UTICK0 peripheral initialization */
  UTICK_SetTick(UTICK0_PERIPHERAL, UTICK0_MODE, UTICK0_TICKS, UTICK0_Callback);
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
  CTIMER1_init();
  FLEXCOMM0_init();
  FLEXCOMM1_init();
  FLEXCOMM2_init();
  FLEXCOMM3_init();
  FLEXCOMM6_init();
  PINT_init();
  RTC_init();
  UTICK0_init();
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
  BOARD_InitPeripherals();
}
