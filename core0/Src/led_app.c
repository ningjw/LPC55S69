#include "main.h"

#define ON  0
#define OFF 1

static uint32_t sys_led_cnt = 0;
static uint32_t bat_led_cnt = 0;
static uint32_t ble_led_cnt = 0;

TaskHandle_t LED_TaskHandle = NULL;  /* LEDÈÎÎñ¾ä±ú */
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
		
        if(flag_led_chk) { //µ±Ç°ledµÆÕýÔÚ×Ô¼ì
            vTaskDelay(200);
        }
        else
        {
//            if((SNVS_HP_GetStatusFlags(SNVS) & SNVS_HPSR_BTN_MASK) == 0x01){
//                g_sys_para.inactiveCount = 0;
//            }
            
            //ÏµÍ³×´Ì¬Ö¸Ê¾µÆ
            switch(g_sys_para.sampLedStatus)
            {
            case WORK_FINE://ºìµÆÃð,ÂÌµÆÁÁ
                GPIO_PinWrite(GPIO, BOARD_LED_SYS_RED_PORT,  BOARD_LED_SYS_RED_PIN, OFF);
                GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, ON);
                break;
            case WORK_ERR://ÂÌµÆÃð,ºìµÆ0.6ÃëÉÁË¸
                GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, OFF);
                if(sys_led_cnt++ % 3 == 0) {
					GPIO_PortToggle(GPIO, BOARD_LED_SYS_RED_PORT,1U << BOARD_LED_SYS_RED_PIN);
                }
                break;
            case WORK_FATAL_ERR://ÂÌµÆÃð,ºìµÆ0.2ÃëÉÁË¸
                GPIO_PinWrite(GPIO, BOARD_LED_SYS_GREEN_PORT, BOARD_LED_SYS_GREEN_PIN, OFF);
				GPIO_PortToggle(GPIO, BOARD_LED_SYS_RED_PORT,1U << BOARD_LED_SYS_RED_PIN);
                break;
            default:
                break;
            }

            //µç³Ø×´Ì¬Ö¸Ê¾µÆ
            switch(g_sys_para.batLedStatus)
            {
            case BAT_FULL://ºìµÆÃð,ÂÌµÆÁÁ
                GPIO_PinWrite(GPIO,BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, OFF);
                GPIO_PinWrite(GPIO,BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, ON);
                break;
            case BAT_CHARGING://ºìµÆÁÁ,ÂÌµÆÃð
                GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,   BOARD_LED_BAT_RED_PIN,  ON);
                GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);
                break;
            case BAT_LOW20://ÂÌµÆÃð,ºìµÆ0.6ÃëÉÁË¸
                GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);
                if(bat_led_cnt++ % 3 == 0) {
					GPIO_PortToggle(GPIO, BOARD_LED_BAT_RED_PORT, 1 << BOARD_LED_BAT_RED_PIN);
                }
                break;
            case BAT_ALARM://ÂÌµÆÃð,ºìµÆ0.2ÃëÉÁË¸
                GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);
				GPIO_PortToggle(GPIO, BOARD_LED_BAT_RED_PORT, 1 << BOARD_LED_BAT_RED_PIN);
                break;
            case BAT_NORMAL://µç³ØÎ´³äµç,ÇÒµçÁ¿´óÓÚ20%
                GPIO_PinWrite(GPIO, BOARD_LED_BAT_RED_PORT,  BOARD_LED_BAT_RED_PIN, OFF);
                GPIO_PinWrite(GPIO, BOARD_LED_BAT_GREEN_PORT, BOARD_LED_BAT_GREEN_PIN, OFF);
                break;
            default:
                break;
            }

            //À¶ÑÀ×´Ì¬Ö¸Ê¾µÆ
            switch(g_sys_para.BleWifiLedStatus)
            {
            case BLE_CLOSE://ºìµÆÃð,ÂÌµÆÃð
                GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
                GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, OFF);
                break;
            case BLE_READY://ºìµÆÃð,ÂÌµÆ0.6ÃëÉÁ
                GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
                if(ble_led_cnt++ % 3 == 0) {
					GPIO_PortToggle(GPIO, BOARD_LED_BLE_GREEN_PORT, 1 << BOARD_LED_BLE_GREEN_PIN);
                }
                break;
            case BLE_CONNECT:  //ºìµÆÃð,ÂÌµÆÁÁ
                GPIO_PinWrite(GPIO, BOARD_LED_BLE_RED_PORT,  BOARD_LED_BLE_RED_PIN, OFF);
                GPIO_PinWrite(GPIO, BOARD_LED_BLE_GREEN_PORT, BOARD_LED_BLE_GREEN_PIN, ON);
                break;
            case BLE_UPDATE://ºìÂÌµÆ½»ÌæÉÁË¸
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
  * @brief   Ö¸Ê¾µÆ×Ô¼ì,ÏÈÏ¨ÃðËùÓÐµÄµÆ200ms,È»ºó½«ËùÓÐledµÆÉèÖÃÎªºìÉ«200ms,
             ÔÙ½«ËùÓÐµÄledµÆÉèÖÃÎªÂÌÉ«200ms,×îºóÏ¨Ãð.
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
