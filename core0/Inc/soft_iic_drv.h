#ifndef __SOFT_IIC_DRV_H
#define __SOFT_IIC_DRV_H


void delay_us(uint32_t nus);
void IIC_Start(void);
void IIC_Stop(void);
uint8_t IIC_Wait_Ack(void);
void IIC_Ack(void);
void IIC_NAck(void);
void IIC_Send_Byte(uint8_t txd);
uint8_t IIC_Read_Byte(unsigned char ack);


#endif
