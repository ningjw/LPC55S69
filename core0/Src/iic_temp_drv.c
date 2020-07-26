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

}


/***************************************************************************************
  * @brief   Read MLX register
  * @input   reg - register number
  * @return  register value
***************************************************************************************/
uint16_t MLX_ReadReg(uint8_t reg) 
{
	return 0;
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






