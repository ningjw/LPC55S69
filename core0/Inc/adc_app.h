#ifndef __ADC_APP_H
#define __ADC_APP_H

#define ADC_READY                GPIO_PinRead(BOARD_ADC_RDY_GPIO, BOARD_ADC_RDY_PIN)

#define NOTIFY_TMR1            (1<<1)
#define NOTIFY_ADC             (1<<2)
#define NOTIFY_ADS1271         (1<<3)
#define NOTIFY_FINISH          (1<<4)
#define NOTIFY_SAMPLE_FINISHED (1<<5)
#define ADC_LEN      100000

extern TaskHandle_t ADC_TaskHandle ;  /* ADCÈÎÎñ¾ä±ú */

void ADC_SampleStart(void);
void ADC_AppTask(void);
void ADC_SampleStop(void);

extern uint32_t ShakeADC[];
extern uint32_t SpeedADC[];
extern char  SpeedStrADC[];
extern char  VibrateStrADC[];


#endif
