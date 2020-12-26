#include "main.h"
#include "fsl_powerquad.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "math.h"

float ShakeADC[ADC_LEN];
float Temperature[64];

TaskHandle_t ADC_TaskHandle = NULL;  /* ADC任务句柄 */

uint32_t timeCapt = 0;
char str[12];

uint32_t ADC_ShakeValue = 0;

float ADC_VoltageValue;

static float GetEnegryWindowCorrected(int windowType) {
    float NBF = 1;
    switch (windowType) {
        case 0://矩形窗0
            NBF = 1;                                                 
            break;
        case 1:   //三角窗1
            NBF = 1.33;
            break;
        case 2:   //汉宁窗2
            NBF = 1.6339;                                            
            break;
        case 3:    //哈明窗3
            NBF = 1.59;                                             
            break;
        case 4:    //布莱克曼窗4
            NBF = 1.65;//待定
            break;
        case 5:    //Kaiser-Bessel(3.0)                      凯赛窗
            NBF = 1.86;                                            
            break;
        case 6:    //top flat  平顶窗
            NBF = 2.26;                                           
            break;
        case 7:    //possion(3.0)
            NBF = 1.65;
            break;
        case 8:    //possion(4.0)
            NBF = 2.08;
            break;
        case 9:    //cauchy(4.0)
            NBF = 1.76;
            break;
        case 10:    //cauchy(5.0)
            NBF = 2.06;
            break;
        case 11:    //Gaussian(3.0)
            NBF = 1.64;
            break;
        case 12:    //Kaiser-Bessel(3.5)
            NBF = 1.93;
            break;

    }
    return NBF;
}


//矩形窗0，三角窗1，汉宁窗2，海明窗3，布莱克曼窗4，凯泽窗5。。。
static float GetRMS(float data[],int len, int windowType) 
{  
    float rms = 0.000;
    float sum = 0.000;
    float EnegryCorrected = GetEnegryWindowCorrected(windowType);
    for (int i = 0; i < len; i++) {
        sum = sum + data[i] * data[i];
    }
    float average = sum / len;
    rms = EnegryCorrected * sqrt(average);
    rms = (float) round(rms * 1000) / 1000.0f;   //保留3位小数
    return rms;
}


/***************************************************************************************
  * @brief   start adc sample
  * @input
  * @return
***************************************************************************************/
void ADC_SampleStart(void)
{
	g_sys_para.tempCount = 0;
    g_adc_set.shkCount = 0;
	memset(ShakeADC,0,ADC_LEN);
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
	int ADC_InvalidCnt = 0;
	while (1) { //wait ads1271 ready
        while(ADC_READY == 1){};//等待ADC_READY为低电平
		ADC_ShakeValue = ADS1271_ReadData();
		ADC_InvalidCnt++;
		if(ADC_InvalidCnt > 100) break;
    }
	
	start_spd_caputer();
	__disable_irq();//关闭中断
	while(ADC_READY == 0){};//等待ADC_READY为高电平
	while(1) { //wait ads1271 ready
        while(ADC_READY == 1){};//等待ADC_READY为低电平
		ShakeADC[g_adc_set.shkCount++] = ADS1271_ReadData();
		if(g_adc_set.shkCount >= g_sys_para.sampNumber){
			g_adc_set.shkCount = g_sys_para.sampNumber;
			break;
		}
    }
	__enable_irq();//开启中断
	stop_spd_caputer();
	if(g_sys_para.sampNumber == 0){//Android发送中断采集命令后,该值为0
		spd_msg->len = 0;
	}
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
	arm_rfft_instance_q31 instance;
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
				float tempValue = 0;
                for(uint32_t i = 0; i < g_adc_set.shkCount; i++) {
					if((uint32_t)ShakeADC[i] < 0x800000){
						ShakeADC[i] = ShakeADC[i] * g_adc_set.bias * 1.0f / 0x800000;
					}else{
						ShakeADC[i] = ((ShakeADC[i] - 0x800000) * g_adc_set.bias * 1.0f / 0x800000) - g_adc_set.bias;
					}
					printf("%01.5f,",ShakeADC[i]);
                }
				
				g_sys_para.shkRMS = GetRMS(ShakeADC, g_adc_set.shkCount, g_adc_set.WindowsType);
#endif
				//计算发送震动信号需要多少个包,蓝牙数据一次发送182个Byte的数据, 而一个采样点需要3Byte表示, 则一次传送58个采样点
				g_sys_para.shkPacks = (g_adc_set.shkCount / ADC_NUM_ONE_PACK) +  (g_adc_set.shkCount%ADC_NUM_ONE_PACK?1:0);
				
				//计算发送转速信号需要多少个包
				g_sys_para.spdPacks = (spd_msg->len / ADC_NUM_ONE_PACK) +  (spd_msg->len%ADC_NUM_ONE_PACK?1:0);
				
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
                xTaskNotifyGive( BLE_WIFI_TaskHandle);
            }
        }
    }
}



