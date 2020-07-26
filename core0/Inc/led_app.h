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
    WORK_FINE = 0, //��ɫ���ɼ�����������,��ʾ��ɫ
    WORK_ERR,      //��ɫ����̬���ɼ�������״̬,�����кţ����ٶȴ�����û��У׼
    WORK_FATAL_ERR,//��ɫ��������˸�����ش��� ���������ϣ���·���ϣ��ɼ��������Թ�����
    BLE_CLOSE,     //����: ��ʾ�������ڹر�״̬
    BLE_READY,     //��ɫ����˸: ��ʾ�ɼ�����ͨ�磬������������������δ���ӵ�Ӧ�ó���
    BLE_CONNECT,   //��ɫ����̬: ��ʾ�ɼ�����ͨ��, ͨ�����������ӵ�Ӧ�ó���
    BLE_UPDATE,    //��ɫ�ͺ�ɫ����: ͨ���������ڽ��й̼����¡�
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
