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
    SYS_IDLE = 0,    //绿色：采集器空闲,且无错误发生.
    
    SYS_IN_SAMPLING, //黄色: 正常采样中
    SYS_UPLOAD_DATA, //黄色: 0.2秒快闪,采集器正在上传采样数据
    SYS_UPLOAD_DATA_ERR,//红色: 0.2秒快闪,上传采样数据错误
    
    SYS_UPGRADE,      //黄绿交替闪烁, 系统正在更新升级.
    SYS_UPGRADE_ERR,  //红黄交替闪烁, 系统在线升级失败.
    
    SYS_ERROR,        //红色常亮, 系统未注册到OneNet平台
    
    BLE_CLOSE,        //不亮: 表示蓝牙处于关闭状态
    BLE_WIFI_READY,   //绿色，闪烁: 表示采集器已通电，已启用蓝牙，但蓝牙未连接到应用程序
    BLE_WIFI_CONNECT,   //绿色，固态: 表示采集器已通电, 通过蓝牙并连接到应用程序
    BLE_WIFI_UPDATE,    //绿色和红色交替: 通过蓝牙正在进行固件更新。
    BAT_FULL,      //绿色：表示电池在连接到电源时已充满电。
    BAT_CHARGING,  //红色，固态：当连接到电源时，指示电池正在充电。
    BAT_LOW20,       //红色，缓慢闪烁：表示电量不足。剩余寿命约为全部寿命的 20%。
    BAT_ALARM,     //红色，快速闪烁：表示电池电量过低。电量过低可通过 APP 来设定最低报警值。如电量低于 10%时，可自动报警
    BAT_NORMAL,    //电池未充电,且电量大于20%
}led_sta_t;

extern TaskHandle_t LED_TaskHandle;
void LED_AppTask(void);
void LED_CheckSelf(void);

#endif
