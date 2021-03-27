#include "main.h"

#define TMP101_ADDR  0x48

#define IIC_TMP_SDA(a) GPIO_PinWrite(GPIO,BOARD_TMP_SDA_PORT,BOARD_TMP_SDA_PIN,a)
#define IIC_TMP_SCL(a) GPIO_PinWrite(GPIO,BOARD_TMP_SCL_PORT,BOARD_TMP_SCL_PIN,a)
#define READ_TMP_SDA   GPIO_PinRead(GPIO,BOARD_TMP_SDA_PORT,BOARD_TMP_SDA_PIN)

static __inline__ void TMP_SDA_OUT(void)
{
	gpio_pin_config_t TMP_SDA_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 1U
    };
    /* Initialize GPIO functionality on pin PIO1_21 (pin M7)  */
    GPIO_PinInit(BOARD_TMP_SDA_GPIO, BOARD_TMP_SDA_PORT, BOARD_TMP_SDA_PIN, &TMP_SDA_config);
}

static __inline__ void TMP_SDA_IN(void)
{
	gpio_pin_config_t TMP_SDA_config = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic = 0U
    };
    /* Initialize GPIO functionality on pin PIO1_21 (pin M7)  */
    GPIO_PinInit(BOARD_TMP_SDA_GPIO, BOARD_TMP_SDA_PORT, BOARD_TMP_SDA_PIN, &TMP_SDA_config);
}


//产生IIC起始信号
void PT_IIC_Start(void)
{
	TMP_SDA_OUT();     //sda线输出
	IIC_TMP_SDA(1);	  	  
	IIC_TMP_SCL(1);
	delay_us(4);
 	IIC_TMP_SDA(0);//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_TMP_SCL(0);//钳住I2C总线，准备发送或接收数据 
}


//产生IIC停止信号
void PT_IIC_Stop(void)
{
	TMP_SDA_OUT();//sda线输出
	IIC_TMP_SCL(0);
	IIC_TMP_SDA(0);//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_TMP_SCL(1); 
	delay_us(4);			
	IIC_TMP_SDA(1);//发送I2C总线结束信号				   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
uint8_t PT_IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	TMP_SDA_IN();      //SDA设置为输入  
	IIC_TMP_SDA(1);
	delay_us(1);	   
	IIC_TMP_SCL(1);
	delay_us(1);	 
	while(READ_TMP_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			PT_IIC_Stop();
			return 1;
		}
	}
	IIC_TMP_SCL(0);//时钟输出0 	   
	return 0;  
}
//产生ACK应答
void PT_IIC_Ack(void)
{
	IIC_TMP_SCL(0);
	TMP_SDA_OUT();
	IIC_TMP_SDA(0);
	delay_us(2);
	IIC_TMP_SCL(1);
	delay_us(2);
	IIC_TMP_SCL(0);
}
//不产生ACK应答		    
void PT_IIC_NAck(void)
{
	IIC_TMP_SCL(0);
	TMP_SDA_OUT();
	IIC_TMP_SDA(1);
	delay_us(2);
	IIC_TMP_SCL(1);
	delay_us(2);
	IIC_TMP_SCL(0);
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void PT_IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   
	TMP_SDA_OUT(); 	    
    IIC_TMP_SCL(0);//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        IIC_TMP_SDA((txd&0x80)>>7);
        txd<<=1; 	  
		delay_us(2);   //对TEA5767这三个延时都是必须的
		IIC_TMP_SCL(1);
		delay_us(2); 
		IIC_TMP_SCL(0);	
		delay_us(2);
    }	 
}
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8_t PT_IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	TMP_SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        IIC_TMP_SCL(0); 
        delay_us(2);
		IIC_TMP_SCL(1);
        receive<<=1;
        if(READ_TMP_SDA)receive++;   
		delay_us(1); 
    }					 
    if (!ack)
        PT_IIC_NAck();//发送nACK
    else
        PT_IIC_Ack(); //发送ACK   
    return receive;
}



void TMP101_WriteReg(uint8_t reg, uint8_t value) 
{
	__disable_irq();
	PT_IIC_Start();
	PT_IIC_Send_Byte(TMP101_ADDR<<1);
	PT_IIC_Wait_Ack();
	PT_IIC_Send_Byte(reg);
	PT_IIC_Wait_Ack();
	PT_IIC_Send_Byte(value);
	PT_IIC_Wait_Ack();
	PT_IIC_Stop();
	__enable_irq();
}


float TMP101_ReadTemp(void) 
{
	float tmp = 0;
	uint16_t value;
	__disable_irq();
	PT_IIC_Start();
	PT_IIC_Send_Byte(TMP101_ADDR<<1);
	PT_IIC_Wait_Ack();
	PT_IIC_Send_Byte(0);
	PT_IIC_Wait_Ack();
	PT_IIC_Start();
	PT_IIC_Send_Byte((TMP101_ADDR<<1) | 1);
	PT_IIC_Wait_Ack();
	value = PT_IIC_Read_Byte(1);
	value =(value << 8) | PT_IIC_Read_Byte(0);
	PT_IIC_Stop();
	__enable_irq();
	value = value >> 4;
	
	if(value & 0x800){//负温度
		tmp = -1.0 * (0xFFF - value) / 16.0;
	}else{//正温度
		tmp = value / 16.0;
	}
	return tmp;
}

void TMP101_Init(void) 
{	
	//bit0: Shutdown Mode (SD) - 1 enable
	//bit1: Thermostat Mode (TM) - 1 Interrupt mode
	//bit2:Polarity (POL) - 1 Active high , State low
	/* 写Configuration Register */
	TMP101_WriteReg(0x01, 0xFF);
}

