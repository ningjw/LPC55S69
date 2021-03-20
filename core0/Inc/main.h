#ifndef __MAIN_H
#define __MAIN_H


#include "stdio.h"
//#include "EventRecorder.h"
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
#include "memory.h"
#include "fsl_iap.h"
#include "fsl_powerquad.h"
#include "fsl_mailbox.h"
#include "fsl_lpadc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "queue.h"
#include "timers.h"

#include "mcmgr.h"
#include "mcmgr_internal_core_api.h"

#include "cJSON.h"
#include "protocol.h"
#include "led_app.h"
#include "ble_app.h"
#include "adc_app.h"
#include "flash_app.h"
#include "battery_app.h"
#include "cat1_app.h"
#include "core1_app.h"
#include "nfc_app.h"

#include "battery_drv.h"
#include "iic_tp100_drv.h"
#include "si5351_drv.h"
#include "adc_drv.h"
#include "w25q128_drv.h"
#include "soft_iic_drv.h"
#include "ota.h"
#include "md5.h"

#define VERSION_CONTROL 3

#if VERSION_CONTROL == 1
#define BLE_VERSION
#elif VERSION_CONTROL == 2
#define WIFI_VERSION
#elif VERSION_CONTROL == 3
#define CAT1_VERSION
#endif


#define EVT_UART_OK        (1<<0) //���ڽ���'}'�������ַ�,�ж�Ϊ���ܵ�һ��������json����,���͸��¼�
#define EVT_UART_TIMTOUT   (1<<1) //���ڽ��ܳ�ʱ, �����ж��Ƿ���ܵ�һ��֡����,�����¼�֪ͨ�߳�
#define EVT_UPLOAD_SAMPLE  (1<<2) //��������,�����������ϴ�������,���͸��¼�.
#define EVT_SAMPLE_FINISH  (1<<4)
#define EVT_SAMPLE_START   (1<<5)
#define EVT_ENTER_SLEEP    (1<<6)

//#define DEBUG_USB_AUDIO
//#define DEBUG_PRINTF printf
#define DEBUG_PRINTF(...)

#define SOFT_VERSION       "V2102271643"
#define HARD_VERSION       "1.0.0"
#define PRODUCT_ID         "388752"
#define DEVICE_ID          "655093740"
#define ACCESS_KEY         "QNbnj7mS4aOTcNHnQCAEPO/2Chv9yNZOqhghd1fYRkw="
#define AUTHORIZATION      "version=2018-10-31&res=products%2F388752&et=1929767259&method=sha1&sign=FdGIbibDkBdX6kN2MyPzkehd7iE\%3D"

#define CAT1_WAIT_TICK      10000
#define PAGE_SIZE 0x200
#define FLEXCOMM_BUFF_LEN 1024

#define CORE1_START_ADDR    0x00008000     // core1������ʼ��ַ
#define CORE0_START_ADDR    0x00010000	   // core0������ʼ��ַ

#define CORE1_DATA_ADDR     0x00030000     // core1�������ݵ�ַ
#define CORE0_DATA_ADDR     0x00038000     // core0�������ݵ�ַ

#define FFT_ADC_ADDR        0x00058000     // fftԭʼ���ݱ���

#define SYS_PARA_ADDR       0x00088000     // ��������
#define SAMPLE_PARA_ADDR    (SYS_PARA_ADDR + PAGE_SIZE)

//��5��sector���ڹ���ADC��������, ÿ����������ռ��20byte, �����Ա���20480/20=1024��
#define ADC_MAX_NUM    1023
#define ADC_INFO_ADDR  0
#define ADC_DATA_ADDR  20480

#if 0
#ifdef BLE_VERSION
    #define ADC_NUM_ONE_PACK   58
	#define FIRM_ONE_PACKE_LEN 166
	#define FIRM_ONE_LEN (FIRM_ONE_PACKE_LEN - 6)
#elif defined (WIFI_VERSION) || defined(CAT1_VERSION)
	#define ADC_NUM_ONE_PACK   335
	#define FIRM_ONE_PACKE_LEN 1006
	#define FIRM_ONE_LEN (FIRM_ONE_PACKE_LEN - 6)
#endif
#endif

#define ADC_NUM_BLE_NFC           58  //һ�����ݰ��а����˶��ٸ�ADC���ݵ�
#define FIRM_TOTAL_LEN_BLE_NFC    166 //һ�����������ܳ���
#define FIRM_DATA_LEN_BLE_NFC    (FIRM_TOTAL_LEN_BLE_NFC - 6)//һ������������Ч���ݳ���

#define ADC_NUM_WIFI_CAT1         335//һ�����ݰ��а����˶��ٸ�ADC���ݵ�
#define FIRM_TOTAL_LEN_WIFI_CAT1  1006//һ�����������ܳ���
#define FIRM_DATA_LEN_WIFI_CAT1   (FIRM_TOTAL_LEN_WIFI_CAT1 - 6)//һ������������Ч���ݳ���

#define ULONG_MAX     0xFFFFFFFF

#define ADC_MODE_LOW_POWER       GPIO_PinWrite(GPIO, BOARD_ADC_MODE_PORT, BOARD_ADC_MODE_PIN, 1)  //�͹���ģʽ
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(GPIO, BOARD_ADC_MODE_PORT, BOARD_ADC_MODE_PIN, 0)   //����ģʽ
#define ADC_MODE_HIGH_RESOLUTION //�߾���ģʽ(����)
#define ADC_SYNC_HIGH            GPIO_PinWrite(GPIO, BOARD_ADC_SYNC_PORT, BOARD_ADC_SYNC_PIN, 1)
#define ADC_SYNC_LOW             GPIO_PinWrite(GPIO, BOARD_ADC_SYNC_PORT, BOARD_ADC_SYNC_PIN, 0)


#define SET_COMMOND_MODE()       GPIO_PinWrite(GPIO, BOARD_BT_MODE_PORT, BOARD_BT_MODE_PIN, 1);
#define SET_THROUGHPUT_MODE()    GPIO_PinWrite(GPIO, BOARD_BT_MODE_PORT, BOARD_BT_MODE_PIN, 0);

#define BLE_RTS_LEVEL()          GPIO_PinRead(GPIO, BOARD_BT_RTS_PORT, BOARD_BT_RTS_PIN)

#define PWR_3V3A_ON               GPIO_PinWrite(GPIO, BOARD_PWR_3V3A_PORT, BOARD_PWR_3V3A_PIN, 1)
#define PWR_3V3A_OFF              GPIO_PinWrite(GPIO, BOARD_PWR_3V3A_PORT, BOARD_PWR_3V3A_PIN, 0)

#define PWR_CAT1_ON              GPIO_PinWrite(GPIO, BOARD_PWR_CAT1_PORT, BOARD_PWR_CAT1_PIN, 1)
#define PWR_CAT1_OFF             GPIO_PinWrite(GPIO, BOARD_PWR_CAT1_PORT, BOARD_PWR_CAT1_PIN, 0)

#define PWR_WIFI_ON              GPIO_PinWrite(GPIO, BOARD_BT_PWR_PORT, BOARD_BT_PWR_PIN, 1)
#define PWR_WIFI_OFF             GPIO_PinWrite(GPIO, BOARD_BT_PWR_PORT, BOARD_BT_PWR_PIN, 0)

#define PWR_5V_ON                GPIO_PinWrite(GPIO, BOARD_PWR_5V_PORT, BOARD_PWR_5V_PIN, 1)
#define PWR_5V_OFF               GPIO_PinWrite(GPIO, BOARD_PWR_5V_PORT, BOARD_PWR_5V_PIN, 0)

#define READ_MCU_CTS             GPIO_PinRead(GPIO, BOARD_WIFI_CTS_PORT, BOARD_WIFI_CTS_PIN)

#define WIFI_NRELOAD_LOW         GPIO_PinWrite(GPIO, BOARD_WIFI_nReload_PORT, BOARD_WIFI_nReload_PIN, 0)
#define WIFI_NRELOAD_HIGH        GPIO_PinWrite(GPIO, BOARD_WIFI_nReload_PORT, BOARD_WIFI_nReload_PIN, 1)

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
	uint32_t sysIdleCount;  //����ͳ�Ƶ�ǰ�ʱ��

    uint8_t  batLedStatus;     //���״̬
    uint8_t  BleWifiLedStatus; //����״̬
    uint8_t  sampLedStatus;    //����״̬
    bool     WorkStatus;       //����ָʾ��ǰ�Ƿ����ڲɼ�.

	float    batVoltage;   //��ص�ѹ
    float    batTemp;      //����¶�
    float    objTemp;      //�����¶�
	float    envTemp;      //�����¶�
    uint32_t batRemainPercent;//���ٷֱ�
	uint32_t batRegAC;        //��ع���оƬAC�Ĵ���ֵ

	uint32_t sampPacksCnt; //������

	uint32_t spdPacksByWifiCat1;     //ת���ź���Ҫ�ֶ��ٸ����������
	uint32_t shkPacksByWifiCat1;     //���ź���Ҫ�ֶ��ٸ����������
    uint32_t spdPacksByBleNfc;       //ת���ź���Ҫ�ֶ��ٸ����������
	uint32_t shkPacksByBleNfc;       //���ź���Ҫ�ֶ��ٸ����������
	uint32_t tempCount;   //��ǰ��¼���¶ȸ���

	uint8_t  Cat1LinkStatus;//����ָʾcat1�Ƿ��Ѿ������Ϸ�����

    uint32_t sampPacksByBleNfc;	 //�ܹ��ɼ���������,��Ҫ�ֶ��ٸ����ϴ�
    uint32_t sampPacksByWifiCat1;//�ܹ��ɼ���������,��Ҫ�ֶ��ٸ����ϴ�
    uint32_t spdStartSid;//ת���źŴ��ĸ�sid��ʼ.
    char     CSQ[8];
}SysPara;

typedef enum{
    NO_VERSION,
    BOOT_NEW_VERSION,
    REPORT_VERSION,
}update_status_t;

typedef struct{
    uint32_t firmCore0Update;//Core0�̼�����,0:��ʾ��ǰ�޸���; 1:��ʾ�Ѿ����̼������ز����ֵ�flash,��������׼����; 2:��ʾ��Ҫ�ϴ��汾��
	uint32_t firmCore1Update;//Core1�̼�����
    uint32_t firmCore0Size;  //Core0�̼��ܴ�С
	uint32_t firmCore1Size;  //Core1�̼��ܴ�С

    uint32_t firmCrc16;      //�̼�У����
    uint32_t firmPacksTotal; //�̼��ܰ���
	uint32_t firmCoreIndex;  //�ж�������core0,��������core1
    uint32_t firmPacksCount; //��ǰ���ܵĹ̼�����
    uint32_t firmCurrentAddr;//��һ��������Ҫ����ĵ�ַ
    uint32_t firmSizeCurrent;

    uint32_t batRemainPercentBak;//������flash�еĵ�ص����ٷֱ�
    uint8_t  batAlarmValue;  //��ص�������ֵ

    uint8_t  autoPwrOffIdleTime;   //���ٷ��Ӳ����,�Զ��ػ�
    uint8_t  autoPwrOffCondition;//�������ô����Զ��ػ�������

    float    bias;       //�𶯴�����ƫ�õ�ѹ
    float    refV;       //1052�Ĳο���ѹֵ

    uint8_t  firstPoweron;   //�״ο���
    uint8_t  WifiBleInitFlag;//����ָʾ����/wifiģ���Ƿ��Ѿ���ʼ����
	uint8_t  SelfRegisterFlag   ;//����ָʾcat1�Ƿ��Ѿ���ʼ����
    char     SN[24];
    char     IMEI[24];
    char     ICCID[24];
    char     device_id[20];
    char     key[40];
    char     firmUpdateTargetV[20];//��Ҫ�����İ汾
    char     firmUpdateToken[48];//������Ҫ��token
    char     firmUpdateMD5[48];//������MD5
    char     reportVersion;//�Ƿ񽫵�ǰ�汾�ϱ�������
}SysFlashPara;

typedef struct{
	char  DetectType;//�ֶ����0,��ʱ���1
    char  IDPath[128];
    char  NamePath[128];
	char  SpeedUnits[8];
	char  ProcessUnits[8];//�¶ȵ�λ
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

    char  MeasurementComment[128];
    char  DAUID[20];

    uint32_t shkCount;   //���źŲɼ����ĸ���
	uint32_t spdCount;   //ת���źŲɼ����ĸ���.

    uint8_t  sampleReason;//�ɼ���ʽ
    uint32_t sampleInterval;//cat1�汾��������
    uint32_t sampNumber;  //ȡ��ʱ��
    uint32_t Ltc1063Clk;  //ȡ��ʱ��Ƶ��
	float    shkRMS;      //���źŵ�ʱ����ֵ
}SysSamplePara;//�ýṹ���ܳ��Ȳ������ױ䶯

extern SysPara        g_sys_para;
extern SysFlashPara   g_sys_flash_para;
extern SysSamplePara  g_sample_para;
extern rtc_datetime_t sysTime;
extern flash_config_t flashInstance;

void SystemSleep();
void delay_us(uint32_t nus);
#endif
