#include "main.h"



extern rtc_datetime_t sampTime;

uint8_t s_buffer[512] = {0};

AdcInfoTotal adcInfoTotal;
AdcInfo adcInfo;
uint32_t inFlashBuf[128] = {0};
void Flash_SavePara(void)
{
	uint16_t i = 0;
	g_adc_set.bias = 2.04;
	memcpy(&inFlashBuf[i++],&g_sys_para.firmCore0Update, 4);
	memcpy(&inFlashBuf[i++],&g_sys_para.firmCore1Update, 4);
	memcpy(&inFlashBuf[i++],&g_sys_para.firmCore0Size, 4);
	memcpy(&inFlashBuf[i++],&g_sys_para.firmCore1Size, 4);
	memcpy(&inFlashBuf[i++],&g_sys_para.firmCrc16, 4);
	memcpy(&inFlashBuf[i++],&g_sys_para.firmPacksTotal, 4);
	memcpy(&inFlashBuf[i++],&g_sys_para.firmCoreIndex, 4);
	memcpy(&inFlashBuf[i++],&g_sys_para.batRegAC, 4);
	memcpy(&inFlashBuf[i++],&g_sys_para.WifiBleInitFlag, 4);
	memcpy(&inFlashBuf[i++],&g_adc_set.bias, 4);
	memcpy(&inFlashBuf[i++],&g_sys_para.Cat1InitFlag, 4);
	
	memory_erase(PARA_ADDR,PAGE_SIZE);
	memory_write(PARA_ADDR,(uint8_t *)inFlashBuf, PAGE_SIZE);
	g_sys_para.batRemainPercentBak = g_sys_para.batRemainPercent;
}

void Flash_ReadPara(void)
{
	uint16_t i = 0;
	memory_read(PARA_ADDR, (uint8_t *)inFlashBuf, PAGE_SIZE);
	
	memcpy(&g_sys_para.firmCore0Update,&inFlashBuf[i++],4);
	memcpy(&g_sys_para.firmCore1Update,&inFlashBuf[i++],4);
	memcpy(&g_sys_para.firmCore0Size, &inFlashBuf[i++],4);
	memcpy(&g_sys_para.firmCore1Size, &inFlashBuf[i++],4);
	memcpy(&g_sys_para.firmCrc16,     &inFlashBuf[i++],4);
	memcpy(&g_sys_para.firmPacksTotal,&inFlashBuf[i++],4);
	memcpy(&g_sys_para.firmCoreIndex, &inFlashBuf[i++],4);
	memcpy(&g_sys_para.batRegAC,      &inFlashBuf[i++],4);
	memcpy(&g_sys_para.WifiBleInitFlag,&inFlashBuf[i++],4);
	memcpy(&g_adc_set.bias,           &inFlashBuf[i++],4);
	memcpy(&g_sys_para.Cat1InitFlag,  &inFlashBuf[i++],4);
	DEBUG_PRINTF("%s: batRegAC=0x%x, bias=%f, WifiBleInitFlag=%d\r\n",
				__func__,g_sys_para.batRegAC,g_adc_set.bias,g_sys_para.WifiBleInitFlag);
	
	//前12字节保存的是 adcInfoTotal 结构体
	SPI_Flash_Read((uint8_t *)&adcInfoTotal.totalAdcInfo, ADC_INFO_ADDR, sizeof(adcInfoTotal));
	
	//判断为首次上电运行
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > ADC_MAX_NUM){
		DEBUG_PRINTF("%s: First run, init adcInfoTotal**************\r\n",__func__);
		//总数为初始化为0
		adcInfoTotal.totalAdcInfo = 0;
		//本次 AdcInfo 结构体保存地址
		adcInfoTotal.addrOfNewInfo = ADC_INFO_ADDR + sizeof(adcInfoTotal);
		//本次数据的开始地址
		adcInfoTotal.addrOfNewData = ADC_DATA_ADDR;
	}
	DEBUG_PRINTF("%s: total=%d, addrOfNewInfo=0x%x, addrOfNewData=0x%x\r\n",__func__,
	               adcInfoTotal.totalAdcInfo,adcInfoTotal.addrOfNewInfo,adcInfoTotal.addrOfNewData);
	
}


/*******************************************************************************
* 函数名  : Flash_WriteAdcData
* 描述    : 将adc数据写入芯片自带flash, 方便进行fft计算
* 输入    : 
* 返回值  :
*******************************************************************************/
void Flash_WriteAdcData(uint8_t *buff, int len)
{
	memory_erase(FFT_ADC_ADDR, len);
	memory_write(FFT_ADC_ADDR,(uint8_t *)buff, len*4);
}


/*******************************************************************************
* 函数名  : FLASH_SaveAppData
* 描述    : 从指定地址开始写入指定长度的数据
* 输入    : WriteAddr:起始地址(此地址必须为2的倍数!!)  pBuffer:数据指针  NumToWrite:
* 返回值  :
*******************************************************************************/
void FLASH_SaveAppData(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite)
{
	uint32_t secpos=0;
	uint16_t secoff=0;
	uint16_t secremain=0;	   
 	uint16_t i=0;    
	uint8_t *FLEXSPI_BUF;
    if(NumByteToWrite == 0) return;
   	FLEXSPI_BUF = s_buffer;	     
 	secpos = WriteAddr/PAGE_SIZE;//扇区地址  
	secoff = WriteAddr%PAGE_SIZE;//在扇区内的偏移
	secremain = PAGE_SIZE-secoff;//扇区剩余空间大小
	
 	if(NumByteToWrite<=secremain)
		secremain=NumByteToWrite;//不大于4096个字节
	__disable_irq();//关闭中断
    while(1) 
	{
		memory_read(secpos*PAGE_SIZE, (uint8_t *)FLEXSPI_BUF, PAGE_SIZE);
		memory_erase(secpos*PAGE_SIZE, PAGE_SIZE);
        for(i=0;i<secremain;i++)	                    //复制
        {
            FLEXSPI_BUF[i+secoff]=pBuffer[i];	  
        }
		memory_write(secpos*PAGE_SIZE, FLEXSPI_BUF, PAGE_SIZE);
		if(NumByteToWrite==secremain)
			break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>PAGE_SIZE)
				secremain=PAGE_SIZE;	           //下一页还是写不完
			else 
				secremain=NumByteToWrite;			//下一页可以写完了
		}	 
	};
	__enable_irq();
}

/***************************************************************************************
  * @brief   增加一条ADC数据
  * @input   
  * @return  
***************************************************************************************/
void W25Q128_AddAdcData(void)
{
	char tempTime[20] = {0};
	//前12字节保存的是 adcInfoTotal 结构体
	SPI_Flash_Read((uint8_t *)&adcInfoTotal.totalAdcInfo, ADC_INFO_ADDR, sizeof(adcInfoTotal));
	
	//判断为首次上电运行
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > ADC_MAX_NUM){
		DEBUG_PRINTF("%s: init adcInfoTotal\r\n",__func__);
		//总数为初始化为0
		adcInfoTotal.totalAdcInfo = 0;
		//本次 AdcInfo 结构体保存地址
		adcInfoTotal.addrOfNewInfo = ADC_INFO_ADDR + sizeof(adcInfoTotal);
		//本次数据的开始地址
		adcInfoTotal.addrOfNewData = ADC_DATA_ADDR;
	}
	
	DEBUG_PRINTF("%s: total=%d, addrOfNewInfo=0x%x, addrOfNewData=0x%x\r\n",__func__,
	               adcInfoTotal.totalAdcInfo,adcInfoTotal.addrOfNewInfo,adcInfoTotal.addrOfNewData);
	
	//初始化 adcInfo 结构体 数据时间
	sprintf(tempTime, "%d%02d%02d%02d%02d%02d", 
		                       sampTime.year%100, sampTime.month, sampTime.day, 
	                           sampTime.hour, sampTime.minute, sampTime.second);
	memcpy(adcInfo.AdcDataTime, tempTime, sizeof(adcInfo.AdcDataTime));
	
	//初始化 adcInfo 结构体 数据长度
	adcInfo.AdcDataLen = sizeof(ADC_Set) + g_adc_set.shkCount*4 + spd_msg->len*4;
	//初始化 adcInfo 结构体 数据地址
	adcInfo.AdcDataAddr = adcInfoTotal.addrOfNewData;
	if((adcInfo.AdcDataAddr % 4) != 0){//判断地址是否四字节对齐
		adcInfo.AdcDataAddr = adcInfo.AdcDataAddr + (4 - (adcInfo.AdcDataAddr % 4));
	}
	if((adcInfo.AdcDataAddr + adcInfo.AdcDataLen) > SPI_FLASH_SIZE_BYTE){//判断地址是否超过flash范围
		adcInfo.AdcDataAddr = ADC_DATA_ADDR;
	}
	
	DEBUG_PRINTF("%s: AdcDataLen=%d, \r\n",__func__,adcInfo.AdcDataLen);
	
	//保存 adcInfo 结构体
	SPI_Flash_Write((uint8_t *)&adcInfo.AdcDataAddr, adcInfoTotal.addrOfNewInfo, sizeof(adcInfo));
	SPI_Flash_Read((uint8_t *)&adcInfo.AdcDataAddr, adcInfoTotal.addrOfNewInfo, sizeof(adcInfo));
	
	//保存 ADC_Set 结构体
	SPI_Flash_Write((uint8_t *)&g_adc_set.DetectType, adcInfo.AdcDataAddr, sizeof(ADC_Set));
	SPI_Flash_Read((uint8_t *)&adcInfo.AdcDataAddr, adcInfoTotal.addrOfNewInfo, sizeof(adcInfo));
	
	SPI_Flash_Read((uint8_t *)&g_adc_set.DetectType, adcInfo.AdcDataAddr, sizeof(ADC_Set));
	SPI_Flash_Read((uint8_t *)&adcInfo.AdcDataAddr, adcInfoTotal.addrOfNewInfo, sizeof(adcInfo));
	
	//保存 震动数据
	SPI_Flash_Write((uint8_t *)&ShakeADC[0], adcInfo.AdcDataAddr+sizeof(ADC_Set), g_adc_set.shkCount*4);
	SPI_Flash_Read((uint8_t *)&ShakeADC[0], adcInfo.AdcDataAddr+sizeof(ADC_Set), g_adc_set.shkCount*4);

	//保存 转速数据
	SPI_Flash_Write((uint8_t *)&spd_msg->spdData[0], adcInfo.AdcDataAddr+sizeof(ADC_Set)+g_adc_set.shkCount*4, spd_msg->len*4);
	
	//更新 adcInfoTotal 结构体中的总采样条数
	adcInfoTotal.totalAdcInfo++;//调用该函数,表示需要增加一条adc采样数据
	if(adcInfoTotal.totalAdcInfo > ADC_MAX_NUM){//判断出已达到最大值
		adcInfoTotal.totalAdcInfo = ADC_MAX_NUM;
	}
	//更新 adcInfoTotal 结构体中的下次采样信息保存地址
	adcInfoTotal.addrOfNewInfo = adcInfoTotal.addrOfNewInfo + sizeof(adcInfoTotal);
	if(adcInfoTotal.addrOfNewInfo >= SPI_FLASH_SIZE_BYTE){
		adcInfoTotal.addrOfNewInfo = ADC_INFO_ADDR + sizeof(AdcInfoTotal);
	}
	//更新 adcInfoTotal 结构体中的下次采样数据保存地址
	adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + adcInfo.AdcDataLen;
	if( adcInfoTotal.addrOfNewData % 4 != 0){//判断新地址不是4字节对齐的, 需要进行4字节对齐
		adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + (4-adcInfoTotal.addrOfNewData%4);
	}
	if (adcInfoTotal.addrOfNewData > SPI_FLASH_SIZE_BYTE){//判断出新地址超出flash范围,则从头开始记录
		adcInfoTotal.addrOfNewData = ADC_DATA_ADDR;
		adcInfoTotal.freeOfKb = 0;
	}else{
		adcInfoTotal.freeOfKb = (SPI_FLASH_SIZE_BYTE - adcInfoTotal.addrOfNewData)/1024;
	}
	
	//更新 adcInfoTotal 结构体
	SPI_Flash_Write((uint8_t *)&adcInfoTotal.totalAdcInfo, ADC_INFO_ADDR, sizeof(AdcInfoTotal));
}

/***************************************************************************************
  * @brief   通过时间读取一条adc数据
  * @input   
  * @return  返回数据地址
***************************************************************************************/
char W25Q128_ReadAdcData(char *adcDataTime)
{
	uint32_t tempAddr;
	uint8_t  ret = false;
	
	//前12字节保存的是 adcInfoTotal 结构体
	SPI_Flash_Read((uint8_t *)&adcInfoTotal.totalAdcInfo, ADC_INFO_ADDR, sizeof(adcInfoTotal));
	
	//flash中还未保存有数据,直接返回
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > ADC_MAX_NUM){
		return ret;
	}
	
	for(uint32_t i = 0; i<adcInfoTotal.totalAdcInfo; i++){
		//往前查找文件
		tempAddr = adcInfoTotal.addrOfNewInfo - (i+1)*sizeof(AdcInfo);
		if(tempAddr < (ADC_INFO_ADDR + sizeof(AdcInfoTotal))){
			tempAddr = ADC_DATA_ADDR - sizeof(adcInfo);
		}
		//读取数据到 adcInfo 结构体
		SPI_Flash_Read((uint8_t *)&adcInfo.AdcDataAddr, tempAddr, sizeof(adcInfo));
		
		//找到文件
		if( memcmp(adcDataTime, adcInfo.AdcDataTime, sizeof(adcInfo.AdcDataTime)) == 0){
			ret = true;
			break;
		}
	}
	if ( ret == true){
		//读取 ADC_Set 结构体数据
		SPI_Flash_Read((uint8_t *)&g_adc_set.DetectType, adcInfo.AdcDataAddr, sizeof(ADC_Set));
		
		//读取 震动数据
		SPI_Flash_Read((uint8_t *)ShakeADC, adcInfo.AdcDataAddr+sizeof(ADC_Set), g_adc_set.shkCount);
		
		//读取 转速数据
		if (spd_msg->len != 0 && spd_msg!= NULL){
			SPI_Flash_Read((uint8_t *)spd_msg->spdData, adcInfo.AdcDataAddr+sizeof(ADC_Set)+g_adc_set.shkCount, spd_msg->len);
		}
	}
	return ret;
}

/***************************************************************************************
  * @brief   获取最近几次数据采集的文件名
  * @input   
  * @return  返回数据地址
***************************************************************************************/
void W25Q128_ReadAdcInfo(int si, int num, char *buf)
{
	uint32_t tempAddr,ei;
	
	//前12字节保存的是 adcInfoTotal 结构体
	SPI_Flash_Read((uint8_t *)&adcInfoTotal.totalAdcInfo, ADC_INFO_ADDR, sizeof(adcInfoTotal));
	
	//flash中还未保存有数据,直接返回
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > ADC_MAX_NUM){
		adcInfoTotal.totalAdcInfo = 0;
		adcInfoTotal.freeOfKb = (SPI_FLASH_SIZE_BYTE - adcInfoTotal.addrOfNewData)/1024;
		return;
	}
	
	if(si > ADC_MAX_NUM)si= ADC_MAX_NUM;
	if(si <= 0 ) si = 1;
	
	ei = num+si;
	if(ei > adcInfoTotal.totalAdcInfo+1) ei=adcInfoTotal.totalAdcInfo+1;
	
	for(uint32_t i = si; i<ei; i++){
		//往前查找文件
		tempAddr = adcInfoTotal.addrOfNewInfo - i*sizeof(AdcInfo);
		if(tempAddr < (ADC_INFO_ADDR + sizeof(AdcInfoTotal))){
			tempAddr = ADC_DATA_ADDR - sizeof(adcInfo);
		}

		//读取数据到 adcInfo 结构体
		SPI_Flash_Read((uint8_t *)&adcInfo.AdcDataAddr, tempAddr, sizeof(adcInfo));
		strncat(buf, adcInfo.AdcDataTime, 12);
		strcat(buf, ",");//添加分隔符
	}
	if(strlen(buf) > 0){//去掉最后一个分隔符
		buf[strlen(buf)-1] = 0x00;
	}
}

