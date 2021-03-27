#ifndef __ADC_DRV_H
#define __ADC_DRV_H


uint32_t ADS1271_ReadData(void);

void ADC_PwmClkStart(uint32_t adcFreq, uint32_t ltcFreq);
void ADC_PwmClkStop(void);
void LPC55S69_AdcInit(void);
void LPC55S69_BatAdcUpdate(void);
#endif
