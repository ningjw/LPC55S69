#include "main.h"

uint32_t SpeedADC[ADC_LEN];
uint32_t ShakeADC[ADC_LEN];
float Temperature[64];

#define ADC_MODE_LOW_POWER       GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 1)  //低功耗模式
#define ADC_MODE_HIGH_SPEED      GPIO_PinWrite(BOARD_ADC_MODE_GPIO, BOARD_ADC_MODE_PIN, 0)   //高速模式
#define ADC_MODE_HIGH_RESOLUTION //高精度模式(浮空)
#define ADC_SYNC_HIGH            GPIO_PinWrite(BOARD_ADC_SYNC_GPIO, BOARD_ADC_SYNC_PIN, 1)
#define ADC_SYNC_LOW             GPIO_PinWrite(BOARD_ADC_SYNC_GPIO, BOARD_ADC_SYNC_PIN, 0)

#define PWR_ON    GPIO_PinWrite(BOARD_PWR_EN_GPIO, BOARD_PWR_EN_PIN, 0)
#define PWR_OFF   GPIO_PinWrite(BOARD_PWR_EN_GPIO, BOARD_PWR_EN_PIN, 1)


TaskHandle_t ADC_TaskHandle = NULL;  /* ADC任务句柄 */

uint32_t timeCapt = 0;
char str[12];

uint32_t ADC_ShakeValue = 0;
uint32_t  ADC_InvalidCnt = 0;

//extern lpspi_transfer_t spi_tranxfer;
extern void QuadTimer1_init(void);
/***************************************************************************************
  * @brief   kPIT_Chnl_0用于触发ADC采样 ；kPIT_Chnl_1 用于定时采样; kPIT_Chnl_2用于定时关机1分钟中断
  * @input
  * @return
***************************************************************************************/
void PIT_IRQHandler(void)
{
    if( PIT_GetStatusFlags(PIT, kPIT_Chnl_2) == true) {
        /* 清除中断标志位.*/
        PIT_ClearStatusFlags(PIT, kPIT_Chnl_2, kPIT_TimerFlag);
		
		//在采集数据时,每间隔1S获取一次温度数据
		if (g_sys_para.tempCount < sizeof(Temperature) && g_sys_para.WorkStatus){
			Temperature[g_sys_para.tempCount++] = MXL_ReadObjTemp();
		}
		
		SNVS_LP_SRTC_GetDatetime(SNVS_LP_PERIPHERAL, &sysTime);
		
        if(g_sys_para.inactiveCount++ >= (g_sys_para.inactiveTime + 1)*60-5) { //定时时间到
            GPIO_PinWrite(BOARD_SYS_PWR_OFF_GPIO, BOARD_SYS_PWR_OFF_PIN, 1);
            //SNVS->LPSR |= SNVS_LPCR_DP_EN(1);
            //SNVS->LPSR |= SNVS_LPCR_TOP(1);
        }
    }

    __DSB();
}

/***************************************************************************************
  * @brief  定时器用于获取转速信号的周期(频率)
  * @input
  * @return
***************************************************************************************/
void TMR1_IRQHandler(void)
{
    /* 清除中断标志 */
    QTMR_ClearStatusFlags(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_EdgeFlag);
	if(g_adc_set.spdCount < ADC_LEN){
		SpeedADC[g_adc_set.spdCount++] = QUADTIMER1_PERIPHERAL->CHANNEL[QUADTIMER1_CHANNEL_0_CHANNEL].CAPT;//读取寄存器值
	}
}


/***************************************************************************************
  * @brief   用于检测ADC_RDY引脚下降沿中断引脚
  * @input
  * @return
***************************************************************************************/
void GPIO2_Combined_0_15_IRQHandler(void)
{
	/* 清除中断标志位 */
	GPIO_PortClearInterruptFlags(BOARD_ADC_RDY_PORT,1U << BOARD_ADC_RDY_PIN);
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
	memset(SpeedADC,0,ADC_LEN);

		//判断自动关机条件
    if(g_sys_para.inactiveCondition != 1) {
        g_sys_para.inactiveCount = 0;
    }
	
	//判断点数是否超出数组界限
	if(g_sys_para.sampNumber > ADC_LEN){
		g_sys_para.sampNumber = ADC_LEN;
	}
	
	PWR_ON;//开启ADC相关的电源
	vTaskDelay(500);//等待500ms
	
	//挂起电池与LED灯的任务,并停止PendSV与SysTick中断
    vTaskSuspend(BAT_TaskHandle);
    vTaskSuspend(LED_TaskHandle);
	vTaskSuspend(LPM_TaskHandle);
	NVIC_DisableIRQ(PendSV_IRQn);   
    NVIC_DisableIRQ(SysTick_IRQn);
	
	//配置ADC芯片时钟
	SI5351a_SetPDN(SI_CLK1_CONTROL,true);
	if(g_adc_set.SampleRate > 45000){
		ADC_MODE_HIGH_SPEED;//使用高速模式
		//使用PWM作为ADS1271的时钟, 其范围为37ns - 10000ns (10us)
		ADC_PwmClkConfig(g_adc_set.SampleRate * 256);
		si5351aSetClk0Frequency(g_adc_set.SampleRate * 256);
	}else{
		ADC_MODE_LOW_POWER;//使用低速模式
		//使用PWM作为ADS1271的时钟, 其范围为37ns - 10000ns (10us)
		ADC_PwmClkConfig(g_adc_set.SampleRate * 512);
		si5351aSetClk0Frequency(g_adc_set.SampleRate * 512);
	}

    /* 输出PWM 用于LTC1063FA的时钟输入,控制采样带宽*/
	g_sys_para.Ltc1063Clk = 1000 * g_adc_set.SampleRate / 25;
#ifdef HDV_1_0
	g_sys_para.Ltc1063Clk = 1000000;
#endif
    QTMR_SetupPwm(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, g_sys_para.Ltc1063Clk, 50U, false, CLOCK_GetFreq(kCLOCK_IpgClk));
    QTMR_StartTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
	SI5351a_SetPDN(SI_CLK0_CONTROL,true);
	si5351aSetClk1Frequency(g_sys_para.Ltc1063Clk);
	
	//设置为true后,会在PIT中断中采集温度数据
	g_sys_para.WorkStatus = true;
	PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_2);
	PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_2);
	
	//开始采集数据前获取一次温度
	Temperature[g_sys_para.tempCount++] = MXL_ReadObjTemp();
	
	/* 输入捕获，计算转速信号周期 */
	QuadTimer1_init();
    QTMR_StartTimer(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
	
	//丢弃前500个数据
	ADC_InvalidCnt = 0;
	while (1) { //wait ads1271 ready
        while(ADC_READY == 1){};//等待ADC_READY为低电平
		ADC_ShakeValue = ADS1271_ReadData();
		ADC_InvalidCnt++;
		if(ADC_InvalidCnt > 100) break;
    }
	
	/* Set channel 0 period (66000000 ticks). 用于触发内部ADC采样，采集转速信号*/
//    PIT_SetTimerPeriod(PIT1_PERIPHERAL, kPIT_Chnl_0, PIT1_CLK_FREQ / g_adc_set.SampleRate);
//    /* Start channel 0. 开启通道0,正式开始采样*/
//    PIT_StartTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
	while(ADC_READY == 0){};//等待ADC_READY为高电平
	while(1) { //wait ads1271 ready
        while(ADC_READY == 1){};//等待ADC_READY为低电平
		__disable_irq();//关闭中断
		ShakeADC[g_adc_set.shkCount++] = ADS1271_ReadData();
		__enable_irq();//开启中断
		if(g_adc_set.shkCount >= g_sys_para.sampNumber){
			g_adc_set.shkCount = g_sys_para.sampNumber;
			SpeedADC[0] = SpeedADC[1];//采集的第一个数据可能不是一个完整的周期,所以第一个数据丢弃.
			if(g_sys_para.sampNumber == 0){//Android发送中断采集命令后,该值为0
				g_adc_set.spdCount = 0;
			}
			break;
		}
    }

	ADC_SampleStop();
}


/***************************************************************************************
  * @brief   stop adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStop(void)
{
	/* Stop channel 0. */
//    PIT_StopTimer(PIT1_PERIPHERAL, kPIT_Chnl_0);
	/* Stop get temperature*/
	g_sys_para.WorkStatus = false;
	ADC_PwmClkStop();
    /* Stop the timer */
    QTMR_StopTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL);
    QTMR_StopTimer(QUADTIMER1_PERIPHERAL, QUADTIMER1_CHANNEL_0_CHANNEL);
	
    vTaskResume(BAT_TaskHandle);
    vTaskResume(LED_TaskHandle);
	vTaskResume(LPM_TaskHandle);
	
	//结束采集后获取一次温度
	Temperature[g_sys_para.tempCount++] = MXL_ReadObjTemp();
	
	//关闭时钟输出
	SI5351a_SetPDN(SI_CLK0_CONTROL,false);
	SI5351a_SetPDN(SI_CLK1_CONTROL,false);
	
	//关闭电源
	PWR_OFF;
	
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
	ADC_PwmClkConfig(1000000);
	si5351aSetClk0Frequency(1000000);
	g_sys_para.Ltc1063Clk = 1000 * g_adc_set.SampleRate / 25;
    QTMR_SetupPwm(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, g_sys_para.Ltc1063Clk, 50U, false, CLOCK_GetFreq(kCLOCK_IpgClk));
    QTMR_StartTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL, kQTMR_PriSrcRiseEdge);
    /* 等待ADS1271 ready,并读取电压值,如果没有成功获取电压值, 则闪灯提示 */
    while (ADC_READY == 1){};  //wait ads1271 ready
    if(ADS1271_ReadData() == 0) {
        g_sys_para.sampLedStatus = WORK_FATAL_ERR;
    }
	SI5351a_SetPDN(SI_CLK0_CONTROL,false);
	QTMR_StopTimer(QUADTIMER3_PERIPHERAL, QUADTIMER3_CHANNEL_0_CHANNEL);
//	while (1) { //wait ads1271 ready
//        while(ADC_READY == 1){};//等待ADC_READY为低电平
//		ADC_ShakeValue = ADS1271_ReadData();
//    }
	PWR_OFF;//关闭ADC采集相关的电源
    printf("ADC Task Create and Running\r\n");
    while(1)
    {
        /*等待ADC完成采样事件*/
        xReturn = xTaskNotifyWait(pdFALSE, ULONG_MAX, &r_event, portMAX_DELAY);

        /* 判断是否成功等待到事件 */
        if ( pdTRUE == xReturn ) {
            /* 完成采样事件*/
            if(r_event & NOTIFY_FINISH) {
				/* ---------------将震动信号转换-----------------------*/
//				printf("共采样到 %d 个震动信号\r\n", g_adc_set.shkCount);
//				float tempValue = 0;
//                for(uint32_t i = 0; i < g_adc_set.shkCount; i++) {
//                    tempValue = ShakeADC[i] * g_adc_set.bias * 1.0f / 0x800000;
//					printf("%01.5f,",tempValue);
//                }
				
				//计算发送震动信号需要多少个包,蓝牙数据一次发送182个Byte的数据, 而一个采样点需要3Byte表示, 则一次传送58个采样点
				g_sys_para.shkPacks = (g_adc_set.shkCount / ADC_NUM_ONE_PACK) +  (g_adc_set.shkCount%ADC_NUM_ONE_PACK?1:0);
				//计算发送转速信号需要多少个包
				g_sys_para.spdPacks = (g_adc_set.spdCount / ADC_NUM_ONE_PACK) +  (g_adc_set.spdCount%ADC_NUM_ONE_PACK?1:0);
                
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
				
				NorFlash_AddAdcData();
                /* 发送任务通知，并解锁阻塞在该任务通知下的任务 */
                xTaskNotifyGive( BLE_TaskHandle);
            }
        }
    }
}



