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


#define EVT_UART_OK        (1<<0) //串口接受'}'等特殊字符,判断为接受到一包完整的json数据,发送该事件
#define EVT_UART_TIMTOUT   (1<<1) //串口接受超时, 用于判断是否接受到一个帧数据,发送事件通知线程
#define EVT_UPLOAD_SAMPLE  (1<<2) //采样结束,将采样数据上传服务器,发送该事件.
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

#define CORE1_START_ADDR    0x00008000     // core1代码起始地址
#define CORE0_START_ADDR    0x00010000	   // core0代码起始地址

#define CORE1_DATA_ADDR     0x00030000     // core1升级数据地址
#define CORE0_DATA_ADDR     0x00038000     // core0升级数据地址

#define FFT_ADC_ADDR        0x00058000     // fft原始数据保存

#define SYS_PARA_ADDR       0x00088000     // 参数保存
#define SAMPLE_PARA_ADDR    (SYS_PARA_ADDR + PAGE_SIZE)

//有5个sector用于管理ADC采样数据, 每个采样数据占用20byte, 共可以保存20480/20=1024个
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

#define ADC_NUM_BLE_NFC           58  //一个数据包中包含了多少个ADC数据点
#define FIRM_TOTAL_LEN_BLE_NFC    166 //一个升级包的总长度
#define FIRM_DATA_LEN_BLE_NFC    (FIRM_TOTAL_LEN_BLE_NFC - 6)//一个升级包中有效数据长度

#define ADC_NUM_WIFI_CAT1         335//一个数据包中包含了多少个ADC数据点
#define FIRM_TOTAL_LEN_WIFI_CAT1  1006//一个升级包的总长度
#define FIRM_DATA_LEN_WIFI_CAT1   (FIRM_TOTAL_LEN_WIFI_CAT1 - 6)//一个升级包中有效数据长度

#define ULONG_MAX     0xFFFFFFFF

#define ADC_MODE_LOW_POWER       GPIO_PinWrite(GPIO, BOARD_ADC_MODE_PORT, BOARD_ADC_MODE_PIN, 1)  //低功耗模式
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(GPIO, BOARD_ADC_MODE_PORT, BOARD_ADC_MODE_PIN, 0)   //高速模式
#define ADC_MODE_HIGH_RESOLUTION //高精度模式(浮空)
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
	uint32_t bakup;//让
}AdcInfoTotal;

typedef struct{
	uint32_t AdcDataAddr;//ADC数据地址
	uint32_t AdcDataLen; //ADC数据长度
	char  AdcDataTime[12];//ADC数据采集时间
}AdcInfo;

//该结构体定义了需要保存到EEPROM中的参数
typedef struct{
	uint32_t sysIdleCount;  //用于统计当前活动时间

    uint8_t  batLedStatus;     //电池状态
    uint8_t  BleWifiLedStatus; //蓝牙状态
    uint8_t  sampLedStatus;    //采样状态
    bool     WorkStatus;       //用于指示当前是否正在采集.

	float    batVoltage;   //电池电压
    float    batTemp;      //电池温度
    float    objTemp;      //物体温度
	float    envTemp;      //环境温度
    uint32_t batRemainPercent;//充电百分比
	uint32_t batRegAC;        //电池管理芯片AC寄存器值

	uint32_t sampPacksCnt; //计数器

	uint32_t spdPacksByWifiCat1;     //转速信号需要分多少个包发送完成
	uint32_t shkPacksByWifiCat1;     //震动信号需要分多少个包发送完成
    uint32_t spdPacksByBleNfc;       //转速信号需要分多少个包发送完成
	uint32_t shkPacksByBleNfc;       //震动信号需要分多少个包发送完成
	uint32_t tempCount;   //当前记录的温度个数

	uint8_t  Cat1LinkStatus;//用于指示cat1是否已经连接上服务器

    uint32_t sampPacksByBleNfc;	 //总共采集道的数据,需要分多少个包上传
    uint32_t sampPacksByWifiCat1;//总共采集道的数据,需要分多少个包上传
    uint32_t spdStartSid;//转速信号从哪个sid开始.
    char     CSQ[8];
}SysPara;

typedef enum{
    NO_VERSION,
    BOOT_NEW_VERSION,
    REPORT_VERSION,
}update_status_t;

typedef struct{
    uint32_t firmCore0Update;//Core0固件更新,0:表示当前无更新; 1:表示已经将固件包下载并保持到flash,做好升级准备了; 2:表示需要上传版本号
	uint32_t firmCore1Update;//Core1固件更新
    uint32_t firmCore0Size;  //Core0固件总大小
	uint32_t firmCore1Size;  //Core1固件总大小

    uint32_t firmCrc16;      //固件校验码
    uint32_t firmPacksTotal; //固件总包数
	uint32_t firmCoreIndex;  //判断是升级core0,还是升级core1
    uint32_t firmPacksCount; //当前接受的固件包数
    uint32_t firmCurrentAddr;//下一次数据需要保存的地址
    uint32_t firmSizeCurrent;

    uint32_t batRemainPercentBak;//保存在flash中的电池电量百分比
    uint8_t  batAlarmValue;  //电池电量报警值

    uint8_t  autoPwrOffIdleTime;   //多少分钟不活动后,自动关机
    uint8_t  autoPwrOffCondition;//用于设置触发自动关机的条件

    float    bias;       //震动传感器偏置电压
    float    refV;       //1052的参考电压值

    uint8_t  firstPoweron;   //首次开机
    uint8_t  WifiBleInitFlag;//用于指示蓝牙/wifi模块是否已经初始化过
	uint8_t  SelfRegisterFlag   ;//用于指示cat1是否已经初始化过
    char     SN[24];
    char     IMEI[24];
    char     ICCID[24];
    char     device_id[20];
    char     key[40];
    char     firmUpdateTargetV[20];//需要升级的版本
    char     firmUpdateToken[48];//升级需要的token
    char     firmUpdateMD5[48];//升级包MD5
    char     reportVersion;//是否将当前版本上报服务器
}SysFlashPara;

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

    char  MeasurementComment[128];
    char  DAUID[20];

    uint32_t shkCount;   //震动信号采集到的个数
	uint32_t spdCount;   //转速信号采集到的个数.

    uint8_t  sampleReason;//采集方式
    uint32_t sampleInterval;//cat1版本采样周期
    uint32_t sampNumber;  //取样时间
    uint32_t Ltc1063Clk;  //取样时钟频率
	float    shkRMS;      //震动信号的时域总值
}SysSamplePara;//该结构体总长度不能轻易变动

extern SysPara        g_sys_para;
extern SysFlashPara   g_sys_flash_para;
extern SysSamplePara  g_sample_para;
extern rtc_datetime_t sysTime;
extern flash_config_t flashInstance;

void SystemSleep();
void delay_us(uint32_t nus);
#endif
