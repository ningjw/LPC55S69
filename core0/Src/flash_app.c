#include "main.h"


uint8_t s_buffer[512] = {0};
AdcInfoTotal adcInfoTotal;
AdcInfo adcInfo;

/***************************************************************************************
  * @brief   将电池电量信息写入Flash
  * @input   
  * @return  
***************************************************************************************/
void Flash_SaveBatPercent()
{
	FLASH_Erase(&flashInstance, BAT_INFO_ADDR, PAGE_SIZE, kFLASH_ApiEraseKey);
	FLASH_Program(&flashInstance, BAT_INFO_ADDR, (uint8_t *)&g_sys_para.batRegAC, 4);
	g_sys_para.batRemainPercentBak = g_sys_para.batRemainPercent;
}


/***************************************************************************************
  * @brief   保存固件升级参数,在接受到完整的固件包后,调用该函数
  * @input   
  * @return  
***************************************************************************************/
void Flash_SaveUpgradePara(void)
{
	FLASH_Erase(&flashInstance, IAP_INFO_ADDR, PAGE_SIZE, kFLASH_ApiEraseKey);
    FLASH_Program(&flashInstance, BAT_INFO_ADDR, &g_sys_para.firmUpdate, 20);
}

/*******************************************************************************
* 函数名  : STMFLASH_Write
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
		FLASH_Erase(&flashInstance, secpos*PAGE_SIZE, PAGE_SIZE, kFLASH_ApiEraseKey);
        for(i=0;i<secremain;i++)	                    //复制
        {
            FLEXSPI_BUF[i+secoff]=pBuffer[i];	  
        }
		FLASH_Program(&flashInstance, secpos*PAGE_SIZE, FLEXSPI_BUF, PAGE_SIZE);
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
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
	
}

/***************************************************************************************
  * @brief   通过时间读取一条adc数据
  * @input   
  * @return  返回数据地址
***************************************************************************************/
char W25Q128_ReadAdcData(char *adcDataTime)
{
	
}

/***************************************************************************************
  * @brief   获取最近几次数据采集的文件名
  * @input   
  * @return  返回数据地址
***************************************************************************************/
void W25Q148_ReadAdcInfo(int si, int num, char *buf)
{
	
}
