#ifndef __ADC_DRV_H
#define __ADC_DRV_H


void XBARA_Configuration(void);
void ADC_ETC_Config(void);
uint32_t LPSPI4_ReadData(void);
void PWM1_Config(void);
void PWM1_Start(void);
void PWM1_Stop(void);
void ADC_PwmClkConfig(uint32_t freq);
void ADC_PwmClkStop(void);


#endif
