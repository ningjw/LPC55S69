#include "main.h"

#define MLX_I2C_MASTER_BASE         LPI2C3_BASE
#define MLX_I2C_MASTER              ((LPI2C_Type *)MLX_I2C_MASTER_BASE)

#define MLX_ADDR     0x5A

#define RAM_ACCESS    0x00   //RAM access command
#define EEPROM_ACCESS 0x20   //EEPROM access command

#define EEPROM_SET_TEMP_HIGH  0x00 //100* (Tmax + 273.15)
#define EEPROM_SET_TEMP_LOW   0x01 //100* (Tmin + 273.15)
#define EEPROM_SET_TEMP_ENV   0x03
#define EEPROM_SET_SLAVE_ADDR 0x0E

#define RAM_TEMP_ENV     0x06 
#define RAM_TEMP_OBJ     0x07 

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void MLX_WriteReg(uint8_t reg, uint8_t value) {
	i2c_master_transfer_t masterXfer = {0};
    uint8_t data = value;
    masterXfer.slaveAddress = MLX_ADDR;
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
uint16_t MLX_ReadReg(uint8_t reg) 
{
	i2c_master_transfer_t masterXfer = {0};
    uint8_t value[3];
    uint16_t ret = 0;
    masterXfer.slaveAddress = MLX_ADDR;
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

/*******************************************************************************
* Function Name  : SMBus_ReadTemp
* Description    : Calculate and return the temperature
* Input          : None
* Output         : None
* Return         : SMBus_ReadMemory(0x00, 0x07)*0.02-273.15
*******************************************************************************/
float MXL_ReadEnvTemp(void)
{
    float    temp = 0;
    uint16_t regValue = 0;
    regValue = MLX_ReadReg(RAM_ACCESS | RAM_TEMP_ENV);
    temp = regValue * 0.02f - 273.15f;
    return  temp;
}


/*******************************************************************************
* Function Name  : MXL_ReadObjTemp
* Description    : Read object temperature
* Input          : None
* Output         : None
* Return         : SMBus_ReadMemory(0x00, 0x07)*0.02-273.15
*******************************************************************************/
float MXL_ReadObjTemp(void)
{
    float    temp = 0;
    uint16_t regValue = 0;
    regValue = MLX_ReadReg(RAM_ACCESS | RAM_TEMP_OBJ);
    temp = regValue * 0.02f - 273.15f;
    return  temp;
}






