#ifndef __ADC_APP_H
#define __ADC_APP_H

#define ADC_READY                GPIO_PinRead(GPIO, BOARD_ADC_RDY_PORT, BOARD_ADC_RDY_PIN)


#define ADC_LEN      32768

#define AUTO_SAMPLE 0  //�������Զ�����
#define HAND_SAMPLE 1  //�ֶ�����

extern TaskHandle_t ADC_TaskHandle ;  /* ADC������ */

void ADC_SampleStart(uint8_t reason);
void ADC_AppTask(void);
void ADC_SampleStop(void);

extern float ShakeADC[];
extern char  SpeedStrADC[];
extern char  VibrateStrADC[];
extern float Temperature[64];

#endif
