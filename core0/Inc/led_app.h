#ifndef __LED_APP_H
#define __LED_APP_H

/* Board led color mapping */
#define LOGIC_LED_ON 0U
#define LOGIC_LED_OFF 1U

#ifndef BOARD_LED_RED_GPIO
#define BOARD_LED_RED_GPIO GPIO
#endif
#define BOARD_LED_RED_GPIO_PORT 1U
#ifndef BOARD_LED_RED_GPIO_PIN
#define BOARD_LED_RED_GPIO_PIN 6U
#endif

#ifndef BOARD_LED_BLUE_GPIO
#define BOARD_LED_BLUE_GPIO GPIO
#endif
#define BOARD_LED_BLUE_GPIO_PORT 1U
#ifndef BOARD_LED_BLUE_GPIO_PIN
#define BOARD_LED_BLUE_GPIO_PIN 4U
#endif

#define LED_RED_INIT(output)                                                                          \
    {                                                                                                 \
        IOCON_PinMuxSet(IOCON, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN, IOCON_DIGITAL_EN);    \
        GPIO_PinInit(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN,             \
                     &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}); /*!< Enable target LED1 */ \
    }
#define LED_RED_ON()                                            \
    GPIO_PortClear(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PORT, \
                   1U << BOARD_LED_RED_GPIO_PIN) /*!< Turn on target LED1 */
#define LED_RED_OFF()                                                                        \
    GPIO_PortSet(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PORT,                                \
                 1U << BOARD_LED_RED_GPIO_PIN) /*!< Turn off target LED1 \ \ \ \ \ \ \ \ \ \ \
                                                */
#define LED_RED_TOGGLE()                                         \
    GPIO_PortToggle(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PORT, \
                    1U << BOARD_LED_RED_GPIO_PIN) /*!< Toggle on target LED1 */

#define LED_BLUE_INIT(output)                                                                         \
    {                                                                                                 \
        IOCON_PinMuxSet(IOCON, BOARD_LED_BLUE_GPIO_PORT, BOARD_LED_BLUE_GPIO_PIN, IOCON_DIGITAL_EN);  \
        GPIO_PinInit(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PORT, BOARD_LED_BLUE_GPIO_PIN,          \
                     &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}); /*!< Enable target LED1 */ \
    }
#define LED_BLUE_ON()                                             \
    GPIO_PortClear(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PORT, \
                   1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Turn on target LED1 */
#define LED_BLUE_OFF()                                          \
    GPIO_PortSet(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PORT, \
                 1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Turn off target LED1 */
#define LED_BLUE_TOGGLE()                                          \
    GPIO_PortToggle(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PORT, \
                    1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Toggle on target LED1 */

#define LED_GREEN_INIT(output)                                                              \
    GPIO_PinInit(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PORT, BOARD_LED_GREEN_GPIO_PIN, \
                 &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}) /*!< Enable target LED1 */
#define LED_GREEN_ON()                                              \
    GPIO_PortClear(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PORT, \
                   1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Turn on target LED1 */
#define LED_GREEN_OFF()                                           \
    GPIO_PortSet(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PORT, \
                 1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Turn off target LED1 */
#define LED_GREEN_TOGGLE()                                           \
    GPIO_PortToggle(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PORT, \
                    1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Toggle on target LED1 */


typedef enum{
    WORK_FINE = 0, //绿色：采集器工作正常,显示绿色
    WORK_ERR,      //红色，固态：采集器错误状态,无序列号，加速度传感器没有校准
    WORK_FATAL_ERR,//红色，快速闪烁：严重错误， 传感器故障，线路故障，采集器不可以工作。
    BLE_CLOSE,     //不亮: 表示蓝牙处于关闭状态
    BLE_READY,     //绿色，闪烁: 表示采集器已通电，已启用蓝牙，但蓝牙未连接到应用程序
    BLE_CONNECT,   //绿色，固态: 表示采集器已通电, 通过蓝牙并连接到应用程序
    BLE_UPDATE,    //绿色和红色交替: 通过蓝牙正在进行固件更新。
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
