#include "main.h"

#define TMP101_ADDR  0x48

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void MLX_WriteReg(uint8_t reg, uint8_t value) {
	i2c_master_transfer_t masterXfer = {0};
    uint8_t data = value;
    masterXfer.slaveAddress = TMP101_ADDR;
    masterXfer.direction = kI2C_Write;
    masterXfer.subaddress = reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data  = &data;;
    masterXfer.dataSize = 1;
    masterXfer.flags = kI2C_TransferDefaultFlag;
    
	I2C_MasterTransferBlocking(FLEXCOMM1_PERIPHERAL, &masterXfer);
}


/***************************************************************************************
  * @brief   Read MLX register
  * @input   reg - register number
  * @return  register value
***************************************************************************************/
uint16_t TMP101_ReadReg(uint8_t reg) 
{
	i2c_master_transfer_t masterXfer = {0};
    uint8_t value[3];
    uint16_t ret = 0;
    masterXfer.slaveAddress = TMP101_ADDR;
    masterXfer.direction = kI2C_Read;
    masterXfer.subaddress = (uint32_t)reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data = value;
    masterXfer.dataSize = 3;
    masterXfer.flags = kI2C_TransferRepeatedStartFlag;
    
    I2C_MasterTransferBlocking(FLEXCOMM1_PERIPHERAL, &masterXfer);
    ret = (value[1]<<8) | value[0];
	return ret;
}

uint16_t TMP101_ReadTemp(void) 
{
	uint16_t temp; 
	temp = TMP101_ReadReg(0x00);
	temp = temp >> 4;
	temp=(temp/16.0); /*仅处理了正的温度 负温度取反后加1 再按正温度处理*/
	
	return temp;
}

void TMP101_ShutDown(void) 
{
	/* 写Configuration Register  12位温度 连续转换*/
	MLX_WriteReg(0x01, 0xFF);
}

void TMP101_Init(void) 
{
	/* 写Configuration Register  12位温度 连续转换*/
	MLX_WriteReg(0x01, 0xFE);
}

