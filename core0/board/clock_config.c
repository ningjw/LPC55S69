/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/
/*
 * How to set up clock using clock driver functions:
 *
 * 1. Setup clock sources.
 *
 * 2. Set up wait states of the flash.
 *
 * 3. Set up all dividers.
 *
 * 4. Set up all selectors to provide selected clocks.
 */

/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Clocks v7.0
processor: LPC55S69
package_id: LPC55S69JEV98
mcu_data: ksdk2_0
processor_version: 9.0.2
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

#include "fsl_power.h"
#include "fsl_clock.h"
#include "clock_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* System clock frequency. */
extern uint32_t SystemCoreClock;

/*******************************************************************************
 ************************ BOARD_InitBootClocks function ************************
 ******************************************************************************/
void BOARD_InitBootClocks(void)
{
    BOARD_BootClockRUN();
}

/*******************************************************************************
 ********************** Configuration BOARD_BootClockRUN ***********************
 ******************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!Configuration
name: BOARD_BootClockRUN
called_from_default_init: true
outputs:
- {id: CTIMER0_clock.outFreq, value: 96 MHz}
- {id: CTIMER1_clock.outFreq, value: 96 MHz}
- {id: CTIMER2_clock.outFreq, value: 96 MHz}
- {id: FXCOM0_clock.outFreq, value: 48 MHz}
- {id: FXCOM2_clock.outFreq, value: 48 MHz}
- {id: FXCOM5_clock.outFreq, value: 48 MHz}
- {id: FXCOM6_clock.outFreq, value: 48 MHz}
- {id: SYSTICK0_clock.outFreq, value: 96 MHz}
- {id: System_clock.outFreq, value: 96 MHz, locked: true, accuracy: '0.001'}
- {id: UTICK_clock.outFreq, value: 1 MHz}
- {id: WDT_clock.outFreq, value: 1 MHz}
settings:
- {id: ANALOG_CONTROL_FRO192M_CTRL_ENDI_FRO_12M_CFG, value: Disable}
- {id: ANALOG_CONTROL_FRO192M_CTRL_ENDI_FRO_96M_CFG, value: Enable}
- {id: PMC_PDRUNCFG_PDEN_XTAL32K_CFG, value: Power_up}
- {id: RTC.RTCOSC32KSEL.sel, value: RTC.XTAL32K}
- {id: RTC_EN_CFG, value: Enable}
- {id: SYSCON.ADCCLKDIV.scale, value: '8', locked: true}
- {id: SYSCON.CTIMERCLKSEL0.sel, value: ANACTRL.fro_hf_clk}
- {id: SYSCON.CTIMERCLKSEL1.sel, value: ANACTRL.fro_hf_clk}
- {id: SYSCON.CTIMERCLKSEL2.sel, value: SYSCON.MAINCLKSELB}
- {id: SYSCON.FCCLKSEL0.sel, value: SYSCON.FROHFDIV}
- {id: SYSCON.FCCLKSEL2.sel, value: SYSCON.FROHFDIV}
- {id: SYSCON.FCCLKSEL5.sel, value: SYSCON.FROHFDIV}
- {id: SYSCON.FCCLKSEL6.sel, value: SYSCON.FROHFDIV}
- {id: SYSCON.FRGCTRL1_DIV.scale, value: '256', locked: true}
- {id: SYSCON.FROHFDIV.scale, value: '2'}
- {id: SYSCON.MAINCLKSELA.sel, value: ANACTRL.fro_hf_clk}
- {id: SYSCON.PLL0CLKSEL.sel, value: SYSCON.CLK_IN_EN}
- {id: SYSCON.PLL0DIV.scale, value: '7'}
- {id: SYSCON.PLL0M_MULT.scale, value: '24', locked: true}
- {id: SYSCON.PLL0N_DIV.scale, value: '1', locked: true}
- {id: SYSCON.SYSTICKCLKSEL0.sel, value: SYSCON.SYSTICKCLKDIV0}
- {id: SYSCON.USB0CLKDIV.scale, value: '2', locked: true}
- {id: SYSCON_CLOCK_CTRL_FRO1MHZ_CLK_ENA_CFG, value: Enabled}
- {id: UTICK_EN_CFG, value: Enable}
sources:
- {id: ANACTRL.fro_hf.outFreq, value: 96 MHz}
- {id: SYSCON.XTAL32M.outFreq, value: 16 MHz, enabled: true}
- {id: SYSCON.fro_1m.outFreq, value: 1 MHz}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/*******************************************************************************
 * Variables for BOARD_BootClockRUN configuration
 ******************************************************************************/
/*******************************************************************************
 * Code for BOARD_BootClockRUN configuration
 ******************************************************************************/
void BOARD_BootClockRUN(void)
{
#ifndef SDK_SECONDARY_CORE
    /*!< Set up the clock sources */
    /*!< Configure FRO192M */
    POWER_DisablePD(kPDRUNCFG_PD_FRO192M);               /*!< Ensure FRO is on  */
    CLOCK_SetupFROClocking(12000000U);                   /*!< Set up FRO to the 12 MHz, just for sure */
    CLOCK_AttachClk(kFRO12M_to_MAIN_CLK);                /*!< Switch to FRO 12MHz first to ensure we can change the clock setting */

    /*!< Configure fro_1m */
    SYSCON->CLOCK_CTRL |=  SYSCON_CLOCK_CTRL_FRO1MHZ_CLK_ENA_MASK;                 /*!< Ensure fro_1m is on */

    CLOCK_SetupFROClocking(96000000U);                   /* Enable FRO HF(96MHz) output */

    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FRO1MHZ_UTICK_ENA_MASK;               /* The FRO 1 MHz clock to UTICK is enabled. */

    POWER_SetVoltageForFreq(96000000U);                  /*!< Set voltage for the one of the fastest clock outputs: System clock output */
    CLOCK_SetFLASHAccessCyclesForFreq(96000000U);          /*!< Set FLASH wait states for core */

    /*!< Set up dividers */
    CLOCK_SetClkDiv(kCLOCK_DivSystickClk0, 0U, true);               /*!< Reset SYSTICKCLKDIV0 divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivSystickClk0, 1U, false);              /*!< Set SYSTICKCLKDIV0 divider to value 1 */
    #if FSL_CLOCK_DRIVER_VERSION >= MAKE_VERSION(2, 3, 4)
      CLOCK_SetClkDiv(kCLOCK_DivFlexFrg0, 0U, false);         /*!< Set DIV to value 0xFF and MULT to value 0U in related FLEXFRGCTRL register */
    #else
      CLOCK_SetClkDiv(kCLOCK_DivFlexFrg0, 256U, false);         /*!< Set DIV to value 0xFF and MULT to value 0U in related FLEXFRGCTRL register */
    #endif
    #if FSL_CLOCK_DRIVER_VERSION >= MAKE_VERSION(2, 3, 4)
      CLOCK_SetClkDiv(kCLOCK_DivFlexFrg2, 0U, false);         /*!< Set DIV to value 0xFF and MULT to value 0U in related FLEXFRGCTRL register */
    #else
      CLOCK_SetClkDiv(kCLOCK_DivFlexFrg2, 256U, false);         /*!< Set DIV to value 0xFF and MULT to value 0U in related FLEXFRGCTRL register */
    #endif
    #if FSL_CLOCK_DRIVER_VERSION >= MAKE_VERSION(2, 3, 4)
      CLOCK_SetClkDiv(kCLOCK_DivFlexFrg5, 0U, false);         /*!< Set DIV to value 0xFF and MULT to value 0U in related FLEXFRGCTRL register */
    #else
      CLOCK_SetClkDiv(kCLOCK_DivFlexFrg5, 256U, false);         /*!< Set DIV to value 0xFF and MULT to value 0U in related FLEXFRGCTRL register */
    #endif
    #if FSL_CLOCK_DRIVER_VERSION >= MAKE_VERSION(2, 3, 4)
      CLOCK_SetClkDiv(kCLOCK_DivFlexFrg6, 0U, false);         /*!< Set DIV to value 0xFF and MULT to value 0U in related FLEXFRGCTRL register */
    #else
      CLOCK_SetClkDiv(kCLOCK_DivFlexFrg6, 256U, false);         /*!< Set DIV to value 0xFF and MULT to value 0U in related FLEXFRGCTRL register */
    #endif
    CLOCK_SetClkDiv(kCLOCK_DivAhbClk, 1U, false);         /*!< Set AHBCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivFrohfClk, 0U, true);               /*!< Reset FROHFDIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivFrohfClk, 2U, false);         /*!< Set FROHFDIV divider to value 2 */
    CLOCK_SetClkDiv(kCLOCK_DivWdtClk, 0U, true);               /*!< Reset WDTCLKDIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivWdtClk, 1U, false);         /*!< Set WDTCLKDIV divider to value 1 */

    /*!< Set up clock selectors - Attach clocks to the peripheries */
    CLOCK_AttachClk(kFRO_HF_to_MAIN_CLK);                 /*!< Switch MAIN_CLK to FRO_HF */
    CLOCK_AttachClk(kFRO_HF_DIV_to_FLEXCOMM0);                 /*!< Switch FLEXCOMM0 to FRO_HF_DIV */
    CLOCK_AttachClk(kFRO_HF_DIV_to_FLEXCOMM2);                 /*!< Switch FLEXCOMM2 to FRO_HF_DIV */
    CLOCK_AttachClk(kFRO_HF_DIV_to_FLEXCOMM5);                 /*!< Switch FLEXCOMM5 to FRO_HF_DIV */
    CLOCK_AttachClk(kFRO_HF_DIV_to_FLEXCOMM6);                 /*!< Switch FLEXCOMM6 to FRO_HF_DIV */
    CLOCK_AttachClk(kSYSTICK_DIV0_to_SYSTICK0);                 /*!< Switch SYSTICK0 to SYSTICK_DIV0 */
    CLOCK_AttachClk(kFRO_HF_to_CTIMER0);                 /*!< Switch CTIMER0 to FRO_HF */
    CLOCK_AttachClk(kFRO_HF_to_CTIMER1);                 /*!< Switch CTIMER1 to FRO_HF */
    CLOCK_AttachClk(kMAIN_CLK_to_CTIMER2);                 /*!< Switch CTIMER2 to MAIN_CLK */

    ANACTRL->FRO192M_CTRL &= ~ANACTRL_FRO192M_CTRL_ENA_12MHZCLK_MASK;    /* Disable FRO 12 MHz output */

    /*!< Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_CORE_CLOCK;
#endif
}

