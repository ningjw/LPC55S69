#include "main.h"

#define ON  0
#define OFF 1

static uint32_t sys_led_cnt = 0;
static uint32_t bat_led_cnt = 0;
static uint32_t ble_led_cnt = 0;

TaskHandle_t LED_TaskHandle = NULL;  /* LED������ */
static bool flag_led_chk;

float T = 0;
/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void LED_AppTask(void)
{
#ifdef DEBUG_USB_AUDIO
	extern void USB_AudioInit(void);
	USB_AudioInit();
#endif
	TMP101_Init();
    while(1)
    {
		RTC_GetDatetime(RTC, &sysTime);
		T = TMP101_ReadTemp();
        if(flag_led_chk) { //��ǰled�������Լ�
            vTaskDelay(200);
        }
        else
        {
            //ϵͳ״ָ̬ʾ��
            switch(g_sys_para.sampLedStatus)
            {
            case WORK_FINE://�����,�̵���
                GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, OFF);
                GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, ON);
                break;
            case WORK_ERR://�̵���,���0.6����˸
                GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, OFF);
                if(sys_led_cnt++ % 3 == 0) {
					GPIO_PortToggle(GPIO, BOARD_LED_SYS_RED_PORT,1U << BOARD_LED_SYS_RED_PIN);
                }
                break;
            case WORK_FATAL_ERR://�̵���,���0.2����˸
                GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, OFF);
				GPIO_PortToggle(GPIO, BOARD_LED_SYS_RED_PORT,1U << BOARD_LED_SYS_RED_PIN);
                break;
            default:
                break;
            }
#ifndef CAT1_VERSION
            //���״ָ̬ʾ��
            switch(g_sys_para.batLedStatus)
            {
            case BAT_FULL://�����,�̵���
                GPIO_PinWrite(GPIO,BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, OFF);
                GPIO_PinWrite(GPIO,BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, ON);
                break;
            case BAT_CHARGING://�����,�̵���
                GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,   BOARD_LED_BAT_RED_PIN,  ON);
                GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);
                break;
            case BAT_LOW20://�̵���,���0.6����˸
                GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);
                if(bat_led_cnt++ % 3 == 0) {
					GPIO_PortToggle(GPIO, BOARD_LED_BAT_RED_PORT, 1 << BOARD_LED_BAT_RED_PIN);
                }
                break;
            case BAT_ALARM://�̵���,���0.2����˸
                GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);
				GPIO_PortToggle(GPIO, BOARD_LED_BAT_RED_PORT, 1 << BOARD_LED_BAT_RED_PIN);
                break;
            case BAT_NORMAL://���δ���,�ҵ�������20%
                GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, OFF);
                GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);
                break;
            default:
                break;
            }

            //����״ָ̬ʾ��
            switch(g_sys_para.BleWifiLedStatus)
            {
            case BLE_CLOSE://�����,�̵���
                GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
                GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, OFF);
                break;
            case BLE_WIFI_READY://�����,�̵�0.6����
                GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
                if(ble_led_cnt++ % 3 == 0) {
					GPIO_PortToggle(GPIO, BOARD_LED_BLE_GREEN_PORT, 1 << BOARD_LED_BLE_GREEN_PIN);
                }
                break;
            case BLE_WIFI_CONNECT:  //�����,�̵���
                GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
                GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, ON);
                break;
            case BLE_WIFI_UPDATE://���̵ƽ�����˸
//				GPIO_PinWrite(BOARD_LED_BLE_RED_GPIO,  BOARD_LED_BLE_RED_PIN, OFF);
//                BOARD_LED_BLE_GREEN_GPIO->DR ^= (1 << BOARD_LED_BLE_GREEN_PIN);
                if(ble_led_cnt++ % 2 == 0) {
                    GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
                    GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, ON);
                } else {
                    GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, ON);
                    GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, OFF);
                }
                break;
            default:
                break;
            }
#endif
        }

        vTaskDelay(300);
    }
}

/***************************************************************************************
  * @brief   ָʾ���Լ�,��Ϩ�����еĵ�200ms,Ȼ������led������Ϊ��ɫ200ms,
             �ٽ����е�led������Ϊ��ɫ200ms,���Ϩ��.
  * @input
  * @return
***************************************************************************************/
void LED_CheckSelf(void)
{
    flag_led_chk = true;
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, OFF);
#ifndef CAT1_VERSION
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);

    GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, OFF);
#endif
    vTaskDelay(100);

    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, ON);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, OFF);
#ifndef CAT1_VERSION
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, ON);
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);

    GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, ON);
    GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, OFF);
#endif
    vTaskDelay(200);

    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, OFF);
#ifndef CAT1_VERSION
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);

    GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, OFF);
#endif
    vTaskDelay(200);

    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, ON);
#ifndef CAT1_VERSION
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, ON);

    GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, ON);
#endif
    vTaskDelay(200);

    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, OFF);
#ifndef CAT1_VERSION
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);

    GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, OFF);
#endif
    vTaskDelay(100);
    flag_led_chk = false;
}
