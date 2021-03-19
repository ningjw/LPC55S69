#ifndef __ADC_APP_H
#define __ADC_APP_H

#define ADC_READY                GPIO_PinRead(GPIO, BOARD_ADC_RDY_PORT, BOARD_ADC_RDY_PIN)


#define ADC_LEN      32768

#define AUTO_SAMPLE 0  //周期性自动采样
#define HAND_SAMPLE 1  //手动采样

extern TaskHandle_t ADC_TaskHandle ;  /* ADC任务句柄 */

void ADC_SampleStart(uint8_t reason);
void ADC_AppTask(void);
void ADC_SampleStop(void);

extern float ShakeADC[];
extern char  SpeedStrADC[];
extern char  VibrateStrADC[];
extern float Temperature[64];

#endif
