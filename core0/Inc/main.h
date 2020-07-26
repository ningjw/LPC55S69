#ifndef __MAIN_H
#define __MAIN_H

#define SOFT_VERSION       "0.16"
#define HARD_VERSION       "1.1"

#define BLE_VERSION
//#define WIFI_VERSION 
#ifdef BLE_VERSION
    #define ADC_NUM_ONE_PACK   58
	#define FIRM_ONE_PACKE_LEN 166 
	#define FIRM_ONE_LEN (FIRM_ONE_PACKE_LEN - 6)
#elif defined WIFI_VERSION
	#define ADC_NUM_ONE_PACK   335
	#define FIRM_ONE_PACKE_LEN 1006
	#define FIRM_ONE_LEN (FIRM_ONE_PACKE_LEN - 6)
#endif

#define ULONG_MAX     0xFFFFFFFF
#define EVT_OK       (1 << 0)
#define EVT_TIMTOUT  (1 << 1)


#include "stdio.h"
#include "EventRecorder.h"
#include "pin_mux.h"
#include "peripherals.h"
#include "clock_config.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_usart.h"
#include "fsl_utick.h"
#include "fsl_i2c.h"
#include "fsl_spi.h"
#include "fsl_power.h"
#include "fsl_iocon.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "queue.h"
#include "timers.h"

#include "cJSON.h"
#include "protocol.h"
#include "led_app.h"
#include "ble_app.h"
#include "battery_drv.h"
#include "iic_temp_drv.h"
#include "si5351_drv.h"

typedef struct{
	uint32_t totalAdcInfo;
	uint32_t addrOfNewInfo;
	uint32_t addrOfNewData;
	uint32_t freeOfKb;
	uint32_t bakup;//��
}AdcInfoTotal;

typedef struct{
	uint32_t AdcDataAddr;//ADC���ݵ�ַ
	uint32_t AdcDataLen; //ADC���ݳ���
	char  AdcDataTime[12];//ADC���ݲɼ�ʱ��
}AdcInfo;

//�ýṹ�嶨������Ҫ���浽EEPROM�еĲ���
typedef struct{
    uint8_t  firmUpdate;     //�̼�����
    uint32_t firmSizeTotal;  //�̼��ܴ�С
    uint32_t firmCrc16;      //�̼�У����
    uint32_t firmPacksTotal; //�̼��ܰ���
//	uint32_t batEnergyInFlash;
	
    uint32_t firmPacksCount; //��ǰ���ܵĹ̼�����
    uint32_t firmSizeCurrent;//��ǰ���ܵ��Ĺ̼���С
    uint32_t firmCurrentAddr;//��һ��������Ҫ����ĵ�ַ
	uint32_t firmByteCount;  //��ǰ���ܵ����ֽ���
	
	uint32_t inactiveCount;  //�������ûʱ��
    uint8_t  inactiveTime;   //�������ûʱ��
    uint8_t  batAlarmValue;  //��ص�������ֵ
    uint8_t  inactiveCondition;//�������ô�������

    uint8_t  batLedStatus; //���״̬
    uint8_t  BleWifiLedStatus; //����״̬
    uint8_t  sampLedStatus;//����״̬
    bool     ads1271IsOk;  //ADCоƬ�Ƿ����
    bool     emmcIsOk;     //eMMC�ļ�ϵͳ�Ƿ����
	float    batVoltage;   //��ص�ѹ
    float    batTemp;      //����¶�
    float    objTemp;      //�����¶�
	float    envTemp;      //�����¶�
    uint32_t    batRemainPercent;//���ٷֱ�
	uint32_t batRegAC;//��ع���оƬAC�Ĵ���ֵ
	uint32_t batRemainPercentBak;//������flash�еĵ�ص����ٷֱ�
	
    uint32_t sampNumber;  //ȡ��ʱ��
    uint32_t Ltc1063Clk;  //ȡ��ʱ��Ƶ��
    char*    sampJson;     //�Ѿ������json��ʽ�����ݵ��׵�ַ
	
	uint32_t sampPacksCnt; //������
    
	uint32_t spdPacks;     //ת���ź���Ҫ�ֶ��ٸ����������
	uint32_t shkPacks;     //���ź���Ҫ�ֶ��ٸ����������
	
    float    voltageADS1271;
	uint32_t periodSpdSignal;//ת���ź�����(us)
    
	uint32_t tempCount;  //��ǰ��¼���¶ȸ���
	bool     WorkStatus; //����ָʾ��ǰ�Ƿ����ڲɼ�.
	
    
    char     fileName[20];
    char     earliestFile[20];
}SysPara;


typedef struct{
    char  IDPath[128];
    char  NamePath[128];
	char  SpeedUnits[8];
	char  ProcessUnits[8];//�¶ȵ�λ
	char  DetectionType;//�ֶ����0,��ʱ���1
	float Senstivity;//������
    float Zerodrift;//���ƫ��
    int   EUType;
    char  EU[8];//��λ
	int   WindowsType;//
    char  WindowName[20];//����������
	int   StartFrequency;//��ʼƵ��
    int   EndFrequency;//��ֹƵ��
    int   SampleRate;//����Ƶ��
    int   Lines;//����
	int   Averages;//ƽ������
    float AverageOverlap;//�ص���
    int   AverageType;//�ص���ʽ
    int   EnvFilterLow;
    int   EnvFilterHigh;//�����˲�Ƶ�� 
	int   IncludeMeasurements;//
	
    float Speed;//ƽ��ת��
    float Process;//ƽ���¶�
    float ProcessMin;//��Сֵ
    float ProcessMax;//���ֵ
    
    int   StorageReson;//�ɼ���ʽ
    char  MeasurementComment[128];
    char  DAUID[20];
    char  Content[4];//����
	
	float    bias;       //�𶯴�����ƫ�õ�ѹ
    float    refV;       //1052�Ĳο���ѹֵ
	uint32_t sampPacks;	   //�ܹ��ɼ���������,��Ҫ�ֶ��ٸ�������Android
	uint32_t spdCount;   //ת���źŲɼ����ĸ���
    uint32_t shkCount;   //���źŲɼ����ĸ���
}ADC_Set;


extern SysPara g_sys_para;
extern ADC_Set g_adc_set;

#endif
