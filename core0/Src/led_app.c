#include "main.h"

#define ON  0
#define OFF 1

static uint32_t sys_led_cnt = 0;
static uint32_t bat_led_cnt = 0;
static uint32_t ble_led_cnt = 0;

TaskHandle_t LED_TaskHandle = NULL;  /* LED������ */
static bool flag_led_chk;


/***************************************************************************************
  * @brief
  * @input
  * @return
***************************************************************************************/
void LED_AppTask(void)
{
    while(1)
    {
		RTC_GetDatetime(RTC, &sysTime);
		
        if(flag_led_chk) { //��ǰled�������Լ�
            vTaskDelay(200);
        }
        else
        {
//            if((SNVS_HP_GetStatusFlags(SNVS) & SNVS_HPSR_BTN_MASK) == 0x01){
//                g_sys_para.inactiveCount = 0;
//            }
            
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
            case BLE_READY://�����,�̵�0.6����
                GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
                if(ble_led_cnt++ % 3 == 0) {
					GPIO_PortToggle(GPIO, BOARD_LED_BLE_GREEN_PORT, 1 << BOARD_LED_BLE_GREEN_PIN);
                }
                break;
            case BLE_CONNECT:  //�����,�̵���
                GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
                GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, ON);
                break;
            case BLE_UPDATE://���̵ƽ�����˸
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

    GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);

    GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, OFF);

    vTaskDelay(100);

    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, ON);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, OFF);

    GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, ON);
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);

    GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, ON);
    GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, OFF);

    vTaskDelay(200);

    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, OFF);

    GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);

    GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, OFF);

    vTaskDelay(200);

    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, ON);

    GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, ON);

    GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, ON);

    vTaskDelay(200);

    GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, OFF);

    GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);

    GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
    GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, OFF);

    vTaskDelay(100);
    flag_led_chk = false;
}
