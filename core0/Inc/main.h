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
#include "nbiot_app.h"
#include "core1_app.h"
#include "nfc_app.h"

#include "battery_drv.h"
#include "iic_tp100_drv.h"
#include "si5351_drv.h"
#include "adc_drv.h"
#include "w25q128_drv.h"
#include "soft_iic_drv.h"


#define VERSION_CONTROL 2

#if VERSION_CONTROL == 1
#define BLE_VERSION
#elif VERSION_CONTROL == 2
#define WIFI_VERSION 
#elif VERSION_CONTROL == 3
#define CAT1_VERSION
#endif

#define DEBUG_USB_AUDIO
#define DEBUG_PRINTF printf
//#define DEBUG_PRINTF(...)  

#define SOFT_VERSION       "21010200"
#define HARD_VERSION       "1.0.0"

#define CORE1_START_ADDR    0x00008000     // core1代码起始地址
#define CORE0_START_ADDR    0x00010000	   // core0代码起始地址

#define CORE1_DATA_ADDR     0x00030000     // core1升级数据地址
#define CORE0_DATA_ADDR     0x00038000     // core0升级数据地址

#define FFT_ADC_ADDR        0x00058000     // fft原始数据保存

#define PARA_ADDR           0x00088000     // 参数保存

#define PAGE_SIZE 0x200

//有5个sector用于管理ADC采样数据, 每个采样数据占用20byte, 共可以保存20480/20=1024个
#define ADC_MAX_NUM    1023
#define ADC_INFO_ADDR  0
#define ADC_DATA_ADDR  20480

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

#define ADC_MODE_LOW_POWER       GPIO_PinWrite(GPIO, BOARD_ADC_MODE_PORT, BOARD_ADC_MODE_PIN, 1)  //低功耗模式
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(GPIO, BOARD_ADC_MODE_PORT, BOARD_ADC_MODE_PIN, 0)   //高速模式
#define ADC_MODE_HIGH_RESOLUTION //高精度模式(浮空)
#define ADC_SYNC_HIGH            GPIO_PinWrite(GPIO, BOARD_ADC_SYNC_PORT, BOARD_ADC_SYNC_PIN, 1)
#define ADC_SYNC_LOW             GPIO_PinWrite(GPIO, BOARD_ADC_SYNC_PORT, BOARD_ADC_SYNC_PIN, 0)


#define SET_COMMOND_MODE()       GPIO_PinWrite(GPIO, BOARD_BT_MODE_PORT, BOARD_BT_MODE_PIN, 1);
#define SET_THROUGHPUT_MODE()    GPIO_PinWrite(GPIO, BOARD_BT_MODE_PORT, BOARD_BT_MODE_PIN, 0);

#define BLE_RTS_LEVEL()          GPIO_PinRead(GPIO, BOARD_BT_RTS_PORT, BOARD_BT_RTS_PIN)

#define PWR_ADC_ON               GPIO_PinWrite(GPIO, BOARD_PWR_ADC_PORT, BOARD_PWR_ADC_PIN, 0)
#define PWR_ADC_OFF              GPIO_PinWrite(GPIO, BOARD_PWR_ADC_PORT, BOARD_PWR_ADC_PIN, 1)

#define PWR_NB_ON                GPIO_PinWrite(GPIO, BOARD_PWR_NB_PORT, BOARD_PWR_NB_PIN, 0)
#define PWR_NB_OFF               GPIO_PinWrite(GPIO, BOARD_PWR_NB_PORT, BOARD_PWR_NB_PIN, 1)

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
	uint32_t bakup;//让
}AdcInfoTotal;

typedef struct{
	uint32_t AdcDataAddr;//ADC数据地址
	uint32_t AdcDataLen; //ADC数据长度
	char  AdcDataTime[12];//ADC数据采集时间
}AdcInfo;

//该结构体定义了需要保存到EEPROM中的参数
typedef struct{
    uint32_t firmCore0Update;//Core0固件更新
	uint32_t firmCore1Update;//Core1固件更新
    uint32_t firmCore0Size;  //Core0固件总大小
	uint32_t firmCore1Size;  //Core1固件总大小
	
    uint32_t firmCrc16;      //固件校验码
    uint32_t firmPacksTotal; //固件总包数
	uint32_t firmCoreIndex;  //判断是升级core0,还是升级core1
	
    uint32_t firmPacksCount; //当前接受的固件包数
    uint32_t firmSizeCurrent;//当前接受到的固件大小
    uint32_t firmCurrentAddr;//下一次数据需要保存的地址
	uint32_t firmByteCount;  //当前接受到的字节数
	
	uint32_t inactiveCount;  //用于设置活动时间
    uint8_t  inactiveTime;   //用于设置活动时间
    uint8_t  batAlarmValue;  //电池电量报警值
    uint8_t  inactiveCondition;//用于设置触发条件

    uint8_t  batLedStatus; //电池状态
    uint8_t  BleWifiLedStatus; //蓝牙状态
	uint8_t  NbNetStatus;  //NB-IoT网络状态指示灯
    uint8_t  sampLedStatus;//采样状态
    bool     ads1271IsOk;  //ADC芯片是否完好
    bool     emmcIsOk;     //eMMC文件系统是否完好
	float    batVoltage;   //电池电压
    float    batTemp;      //电池温度
    float    objTemp;      //物体温度
	float    envTemp;      //环境温度
    uint32_t batRemainPercent;//充电百分比
	uint32_t batRegAC;     //电池管理芯片AC寄存器值
	uint32_t batRemainPercentBak;//保存在flash中的电池电量百分比
	
    uint32_t sampNumber;  //取样时间
    uint32_t Ltc1063Clk;  //取样时钟频率
    char*    sampJson;     //已经打包成json格式的数据的首地址
	
	uint32_t sampPacksCnt; //计数器
    
	uint32_t spdPacks;     //转速信号需要分多少个包发送完成
	uint32_t shkPacks;     //震动信号需要分多少个包发送完成
	float    shkRMS;       //震动信号的时域总值
    float    voltageADS1271;
	uint32_t periodSpdSignal;//转速信号周期(us)
    
	uint32_t tempCount;  //当前记录的温度个数
	bool     WorkStatus; //用于指示当前是否正在采集.
	uint8_t  WifiBleInitFlag;
    char     fileName[20];
    char     earliestFile[20];
}SysPara;


typedef struct{
	char  DetectType;//手动检测0,定时检测1
    char  IDPath[128];
    char  NamePath[128];
	char  SpeedUnits[8];
	char  ProcessUnits[8];//温度单位
	float Senstivity;//灵敏度
    float Zerodrift;//零点偏移
    int   EUType;
    char  EU[8];//单位
	int   WindowsType;//
    char  WindowName[20];//窗函数名称
	int   StartFrequency;//起始频率
    int   EndFrequency;//截止频率
    int   SampleRate;//采样频率
    int   Lines;//线数
	int   Averages;//平均次数
    float AverageOverlap;//重叠率
    int   AverageType;//重叠方式
    int   EnvFilterLow;
    int   EnvFilterHigh;//包络滤波频段 
	int   IncludeMeasurements;//
	
    float Speed;//平均转速
    float Process;//平均温度
    float ProcessMin;//最小值
    float ProcessMax;//最大值
    
    int   StorageReson;//采集方式
    char  MeasurementComment[128];
    char  DAUID[20];
    char  Content[4];//保留
	
	float    bias;       //震动传感器偏置电压
    float    refV;       //1052的参考电压值
	uint32_t sampPacks;	   //总共采集道的数据,需要分多少个包发给Android
    uint32_t shkCount;   //震动信号采集到的个数
	uint32_t spdStartSid;//转速信号从那个sid开始.
}ADC_Set;


extern SysPara g_sys_para;
extern ADC_Set g_adc_set;
extern rtc_datetime_t sysTime;
extern flash_config_t flashInstance;

void SystemSleep();

#endif
