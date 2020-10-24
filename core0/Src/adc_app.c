#include "main.h"

uint32_t SpeedADC[SPD_LEN];
uint32_t ShakeADC[ADC_LEN];
float Temperature[64];



TaskHandle_t ADC_TaskHandle = NULL;  /* ADC任务句柄 */

uint32_t timeCapt = 0;
char str[12];

uint32_t ADC_ShakeValue = 0;
uint32_t  ADC_InvalidCnt = 0;
float ADC_VoltageValue;

/***************************************************************************************
  * @brief   
  * @input
  * @return
***************************************************************************************/
void RTC_IRQHANDLER(void)
{
	//在采集数据时,每间隔1S获取一次温度数据
	if (g_sys_para.tempCount < sizeof(Temperature) && g_sys_para.WorkStatus){
		Temperature[g_sys_para.tempCount++] = TMP101_ReadTemp();
	}
	
	if(g_sys_para.inactiveCount++ >= (g_sys_para.inactiveTime + 1)*60-5) { //定时时间到
		GPIO_PinWrite(GPIO, BOARD_PWR_OFF_PORT, BOARD_PWR_OFF_PIN, 1);//关机
	}
}



/***************************************************************************************
  * @brief   start adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStart(void)
{
	g_sys_para.tempCount = 0;
    g_adc_set.spdCount = 0;
    g_adc_set.shkCount = 0;
	memset(ShakeADC,0,ADC_LEN);
	memset(SpeedADC,0,SPD_LEN);
	PWR_ADC_ON;//开启ADC相关的电源
	PWR_5V_ON;//开启5V的滤波器电源
		//判断自动关机条件
    if(g_sys_para.inactiveCondition != 1) {
        g_sys_para.inactiveCount = 0;
    }
	
	//判断点数是否超出数组界限
	if(g_sys_para.sampNumber > ADC_LEN){
		g_sys_para.sampNumber = ADC_LEN;
	}
	
	/* 输出PWM 用于LTC1063FA的时钟输入,控制采样带宽*/
	g_sys_para.Ltc1063Clk = 1000 * g_adc_set.SampleRate / 25;
	SI5351a_SetPDN(SI_CLK0_CONTROL,true);
	si5351aSetFilterClk1(g_sys_para.Ltc1063Clk);
	
	//配置ADC芯片时钟
	SI5351a_SetPDN(SI_CLK1_CONTROL,true);
	if(g_adc_set.SampleRate > 45000){
		ADC_MODE_HIGH_SPEED;//使用高速模式
		//使用PWM作为ADS1271的时钟, 其范围为37ns - 10000ns (10us)
		si5351aSetAdcClk0(g_adc_set.SampleRate * 256);
		ADC_PwmClkStart(g_adc_set.SampleRate * 256, g_sys_para.Ltc1063Clk);
	}else{
		ADC_MODE_LOW_POWER;//使用低速模式
		//使用PWM作为ADS1271的时钟, 其范围为37ns - 10000ns (10us)
		si5351aSetAdcClk0(g_adc_set.SampleRate * 512);
		ADC_PwmClkStart(g_adc_set.SampleRate * 512, g_sys_para.Ltc1063Clk);
	}

    
	
	
	
	vTaskDelay(100);//等待500ms
	
	//开始采集数据前获取一次温度
	Temperature[g_sys_para.tempCount++] = TMP101_ReadTemp();
	//设置为true后,会在PIT中断中采集温度数据
	g_sys_para.WorkStatus = true;
	
	//丢弃前部分数据
	ADC_InvalidCnt = 0;
	while (1) { //wait ads1271 ready
        while(ADC_READY == 1){};//等待ADC_READY为低电平
		ADC_ShakeValue = ADS1271_ReadData();
		ADC_InvalidCnt++;
		if(ADC_InvalidCnt > 100) break;
    }
	__disable_irq();//关闭中断
	while(ADC_READY == 0){};//等待ADC_READY为高电平
	while(1) { //wait ads1271 ready
        while(ADC_READY == 1){};//等待ADC_READY为低电平
#if 1
		ShakeADC[g_adc_set.shkCount++] = ADS1271_ReadData();
#else
		ShakeADC[g_adc_set.shkCount++] = 0xC00000;
#endif
		if(g_adc_set.shkCount >= g_sys_para.sampNumber){
			g_adc_set.shkCount = g_sys_para.sampNumber;
			SpeedADC[0] = SpeedADC[1];//采集的第一个数据可能不是一个完整的周期,所以第一个数据丢弃.
			if(g_sys_para.sampNumber == 0){//Android发送中断采集命令后,该值为0
				g_adc_set.spdCount = 0;
			}
			break;
		}
    }
	__enable_irq();//开启中断
	
	//结束采集后获取一次温度
	Temperature[g_sys_para.tempCount++] = TMP101_ReadTemp();
	ADC_SampleStop();
}


/***************************************************************************************
  * @brief   stop adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStop(void)
{
	/* Stop get temperature*/
	g_sys_para.WorkStatus = false;
	
	//关闭时钟输出
	SI5351a_SetPDN(SI_CLK0_CONTROL,false);
	SI5351a_SetPDN(SI_CLK1_CONTROL,false);
	ADC_PwmClkStop();
	//关闭电源
	PWR_ADC_OFF;
	PWR_5V_OFF;//开启5V的滤波器电源
	
    /* 触发ADC采样完成事件  */
    xTaskNotify(ADC_TaskHandle, NOTIFY_FINISH, eSetBits);
}


/***********************************************************************
  * @ 函数名  ： ADC采集任务
  * @ 功能说明：
  * @ 参数    ： 无
  * @ 返回值  ： 无
  **********************************************************************/
void ADC_AppTask(void)
{
    uint32_t r_event;
    BaseType_t xReturn = pdTRUE;

	/*以下为开机自检代码*/
	ADC_MODE_LOW_POWER;
	si5351aSetAdcClk0(1000000);//给ADS1271提供时钟
	si5351aSetFilterClk1(1000000);//设置滤波器时钟
	PWR_5V_ON;
	PWR_ADC_ON;
#if 1
    /* 等待ADS1271 ready,并读取电压值,如果没有成功获取电压值, 则闪灯提示 */
    while (ADC_READY == 1){};  //wait ads1271 ready
    if(ADS1271_ReadData() == 0) {
        g_sys_para.sampLedStatus = WORK_FATAL_ERR;
    }
#else
	while (1) { //wait ads1271 ready
        while(ADC_READY == 1){};//等待ADC_READY为低电平
		ADC_ShakeValue = ADS1271_ReadData();
		ADC_VoltageValue = ADC_ShakeValue * 2.048 / 0x800000;
    }
#endif
	SI5351a_SetPDN(SI_CLK0_CONTROL, false);
	SI5351a_SetPDN(SI_CLK1_CONTROL, false);
	PWR_ADC_OFF;//关闭ADC采集相关的电源
	PWR_5V_OFF;
    printf("ADC Task Create and Running\r\n");
	TMP101_Init();
    while(1)
    {
        /*等待ADC完成采样事件*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &r_event, portMAX_DELAY);

        /* 判断是否成功等待到事件 */
        if ( pdTRUE == xReturn ) {
            /* 完成采样事件*/
            if(r_event & NOTIFY_FINISH) {
				/* ---------------将震动信号转换-----------------------*/
#if 0
				__disable_irq();
				float tempValue = 0;
                for(uint32_t i = 0; i < g_adc_set.shkCount; i++) {
					if(ShakeADC[i] < 0x800000){
						tempValue = ShakeADC[i] * g_adc_set.bias * 1.0f / 0x800000;
					}else{
						tempValue = ((ShakeADC[i] - 0x800000) * g_adc_set.bias * 1.0f / 0x800000) - g_adc_set.bias;
					}
					printf("%01.5f,",tempValue);
                }
				__enable_irq();
#endif
				//计算发送震动信号需要多少个包,蓝牙数据一次发送182个Byte的数据, 而一个采样点需要3Byte表示, 则一次传送58个采样点
				g_sys_para.shkPacks = (g_adc_set.shkCount / ADC_NUM_ONE_PACK) +  (g_adc_set.shkCount%ADC_NUM_ONE_PACK?1:0);
				//计算发送转速信号需要多少个包
				g_sys_para.spdPacks = (g_adc_set.spdCount / ADC_NUM_ONE_PACK) +  (g_adc_set.spdCount%ADC_NUM_ONE_PACK?1:0);
                
				g_adc_set.spdStartSid = g_sys_para.shkPacks + 3;
				//计算将一次采集数据全部发送到Android需要多少个包
#ifdef BLE_VERSION
				g_adc_set.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 3;
#elif defined WIFI_VERSION
				g_adc_set.sampPacks = g_sys_para.spdPacks + g_sys_para.shkPacks + 1;
#endif
                /* ------------------统计平均温度,最小温度,最大温度--------------------*/
			    float sum = 0;
				int min_i = 0;
				int max_i = 0;
				for(int i=0;i<g_sys_para.tempCount;i++){
					sum += Temperature[g_sys_para.tempCount];
					min_i = Temperature[i] < Temperature[min_i] ? i : min_i;
					max_i = Temperature[i] > Temperature[max_i] ? i : max_i;
				}
				g_adc_set.Process = sum / g_sys_para.tempCount;
				g_adc_set.ProcessMax = Temperature[max_i];
				g_adc_set.ProcessMin = Temperature[min_i];
				
				W25Q128_AddAdcData();
                /* 发送任务通知，并解锁阻塞在该任务通知下的任务 */
                xTaskNotifyGive( BLE_TaskHandle);
            }
        }
    }
}



