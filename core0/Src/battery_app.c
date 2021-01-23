#include "main.h"

#define BAT_CHG_UNCOMPLETE GPIO_PinWrite(GPIO, BOARD_PWR_CHG_COMPLETE_PORT, BOARD_PWR_CHG_COMPLETE_PIN, 0)

#define READ_CHARGE_STA    GPIO_PinRead(GPIO, BOARD_BAT_CHRG_PORT, BOARD_BAT_CHRG_PIN)
#define READ_STDBY_STA     GPIO_PinRead(GPIO, BOARD_BAT_STDBY_PORT,BOARD_BAT_STDBY_PIN)

TaskHandle_t BAT_TaskHandle = NULL;  /* 电池管理任务句柄 */
uint8_t status = 0;
float remain;
uint16_t temp;
/***********************************************************************
  * @ 函数名  ： BAT_AppTask
  * @ 功能说明：
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void BAT_AppTask(void)
{
#if defined( WIFI_VERSION) || defined(BLE_VERSION)
    // Enable auto measurement of battery voltage and temperature
    LTC2942_SetADCMode(LTC2942_ADC_AUTO);
	
    // Enable analog section of the chip (in case if it disabled)
    LTC2942_SetAnalog(LTC2942_AN_ENABLED);

    // Set prescaler M value
    LTC2942_SetPrescaler(LTC2942_PSCM_64);

	LTC2942_SetALCCMode(LTC2942_ALCC_DISABLED);
	
	g_sys_para.batRegAC = LTC2942_GetAC();
	//此时电池电量检测芯片中的AC值为完全掉电后的值,需要从flash中取
	if(g_sys_para.batRegAC >= 0x7FE0 && g_sys_para.batRegAC <= 0x7FFF){
		DEBUG_PRINTF("%s: LTC2942_SetAC\r\n",__func__);
		LTC2942_SetAC(g_sys_para.batRegAC);
		//调试时增加这条语句,为了测试拔掉电池后再上电会不会成功执行到该if语句里
		//要正常看到电池亮红灯,还需要注释掉while(1)中对电池状态的检测代码
//		g_sys_para.batLedStatus = BAT_CHARGING;
	}
	
#elif defined(CAT1_VERSION)
	lpadc_conv_result_t batVoltage;
	
	POWER_DisablePD(kPDRUNCFG_PD_LDOGPADC);
	
	LPADC_DoSoftwareTrigger(ADC0_PERIPHERAL, 1U); /* 1U对应触发0*/
	
//	while (!LPADC_GetConvResult(ADC0_PERIPHERAL, &batVoltage, 0U)) {}
	
	//根据电压计算电池容量
    if(g_sys_para.batVoltage >= 3.73f) { //(3.73 - 4.2)
        remain = -308.19f * g_sys_para.batVoltage * g_sys_para.batVoltage + 2607.7f * g_sys_para.batVoltage - 5417.9f;
    } else if(g_sys_para.batVoltage >= 3.68f) { //(3.68 - 3.73)
        remain = -1666.7f * g_sys_para.batVoltage * g_sys_para.batVoltage + 12550 * g_sys_para.batVoltage - 23603;
    } else { // (3.5 - 3.68)
        remain = 55.556f * g_sys_para.batVoltage - 194.44f;
    }
#endif
    DEBUG_PRINTF("BAT_AppTask Running\r\n");
	
    while(1)
    {
		temp = TMP101_ReadTemp();
		
#if defined( WIFI_VERSION) || defined(BLE_VERSION)
        // 获取电池电压
        g_sys_para.batVoltage = LTC2942_GetVoltage() / 1000.0;
		
        // 获取温度传感器温度
//        g_sys_para.batTemp = LTC2942_GetTemperature() / 100.0;
		
		// 获取电量百分比
		g_sys_para.batRegAC = LTC2942_GetAC();
		if(g_sys_para.batRegAC > 0x7FE0 && g_sys_para.batRegAC <= 0x7FFF){//这里保证AC寄存器里的数据不为7FFF,因为7FFF在开机时会作为一个判断
			if(READ_CHARGE_STA == 0 && READ_STDBY_STA == 1) {//充电当中
				g_sys_para.batRegAC = 0x8000;
				LTC2942_SetAC(0x8000);
			}else{
				g_sys_para.batRegAC = 0x7FE0;
				LTC2942_SetAC(0x7FE0);
			}
			Flash_SavePara();//将AC寄存器的值写入flash保存下来
		}
        g_sys_para.batRemainPercent = g_sys_para.batRegAC * 100.0 / 0xFFFF;

		//当保存在flash中的电池电量与当前电池电量不相等时
		//将当前电量重新保存到flash当中
		if(g_sys_para.batRemainPercent != g_sys_para.batRemainPercentBak){
			Flash_SavePara();
		}
		
        if(READ_CHARGE_STA == 0 && READ_STDBY_STA == 1) {//充电当中
//			DEBUG_PRINTF("%s: Battery charging \r\n",__func__);
            g_sys_para.batLedStatus = BAT_CHARGING;
            BAT_CHG_UNCOMPLETE;
			if (g_sys_para.batRemainPercent == 100){//充电当中检测到电量为100%, 改为99%
				g_sys_para.batRemainPercent = 99;
			}
        } else 
#elif defined CAT1_VERSION


#endif
		if(READ_CHARGE_STA == 1 && READ_STDBY_STA == 0) { //充电完成
//			DEBUG_PRINTF("%s: Battery full \r\n",__func__);
            g_sys_para.batLedStatus = BAT_FULL;
            LTC2942_SetAC(0xFFFF);
        } else if(g_sys_para.batRemainPercent <= g_sys_para.batAlarmValue) { //电量低于报警值
            DEBUG_PRINTF("%s: Percent <= AlarmValue \r\n",__func__);
			g_sys_para.batLedStatus = BAT_ALARM;
			if(g_sys_para.batRemainPercent == 0){//放电当中,电量为0,手动改为1
				g_sys_para.batRemainPercent = 1;
			}
        } else if(g_sys_para.batRemainPercent <= 20) { //电量低于20%
			DEBUG_PRINTF("%s: Percent <= 20 \r\n",__func__);
            g_sys_para.batLedStatus = BAT_LOW20;
			if(g_sys_para.batRemainPercent == 0){//放电当中,电量为0,手动改为1
				g_sys_para.batRemainPercent = 1;
			}
        } else {
            g_sys_para.batLedStatus = BAT_NORMAL;
        }
        vTaskDelay(2000);
	}
}



