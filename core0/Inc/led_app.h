#ifndef __LED_APP_H
#define __LED_APP_H

#define ON  0
#define OFF 1

#include "main.h"

static __inline void SysLedRedOn(void)
{
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, 1);
}

static __inline void SysLedGreenOn(void)
{
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, 1);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, 0);
}

static __inline void SysLedYellowOn(void)
{
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, 1);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, 1);
}

static __inline void SysLedOff(void)
{
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, 0);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, 0);
}

typedef enum{
    SYS_IDLE = 0,    //��ɫ���ɼ�������,���޴�����.
    
    SYS_IN_SAMPLING, //��ɫ: ����������
    SYS_UPLOAD_DATA, //��ɫ: 0.2�����,�ɼ��������ϴ���������
    SYS_UPLOAD_DATA_ERR,//��ɫ: 0.2�����,�ϴ��������ݴ���
    
    SYS_UPGRADE,      //���̽�����˸, ϵͳ���ڸ�������.
    SYS_UPGRADE_ERR,  //��ƽ�����˸, ϵͳ��������ʧ��.
    
    SYS_ERROR,        //��ɫ����, ϵͳδע�ᵽOneNetƽ̨
    
    BLE_CLOSE,        //����: ��ʾ�������ڹر�״̬
    BLE_WIFI_READY,   //��ɫ����˸: ��ʾ�ɼ�����ͨ�磬������������������δ���ӵ�Ӧ�ó���
    BLE_WIFI_CONNECT,   //��ɫ����̬: ��ʾ�ɼ�����ͨ��, ͨ�����������ӵ�Ӧ�ó���
    BLE_WIFI_UPDATE,    //��ɫ�ͺ�ɫ����: ͨ���������ڽ��й̼����¡�
    BAT_FULL,      //��ɫ����ʾ��������ӵ���Դʱ�ѳ����硣
    BAT_CHARGING,  //��ɫ����̬�������ӵ���Դʱ��ָʾ������ڳ�硣
    BAT_LOW20,       //��ɫ��������˸����ʾ�������㡣ʣ������ԼΪȫ�������� 20%��
    BAT_ALARM,     //��ɫ��������˸����ʾ��ص������͡��������Ϳ�ͨ�� APP ���趨��ͱ���ֵ����������� 10%ʱ�����Զ�����
    BAT_NORMAL,    //���δ���,�ҵ�������20%
}led_sta_t;

extern TaskHandle_t LED_TaskHandle;
void LED_AppTask(void);
void LED_CheckSelf(void);

#endif
