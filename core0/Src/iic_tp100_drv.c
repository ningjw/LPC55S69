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


//����IIC��ʼ�ź�
void PT_IIC_Start(void)
{
	TMP_SDA_OUT();     //sda�����
	IIC_TMP_SDA(1);	  	  
	IIC_TMP_SCL(1);
	delay_us(4);
 	IIC_TMP_SDA(0);//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_TMP_SCL(0);//ǯסI2C���ߣ�׼�����ͻ�������� 
}


//����IICֹͣ�ź�
void PT_IIC_Stop(void)
{
	TMP_SDA_OUT();//sda�����
	IIC_TMP_SCL(0);
	IIC_TMP_SDA(0);//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_TMP_SCL(1); 
	delay_us(4);			
	IIC_TMP_SDA(1);//����I2C���߽����ź�				   	
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
uint8_t PT_IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	TMP_SDA_IN();      //SDA����Ϊ����  
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
	IIC_TMP_SCL(0);//ʱ�����0 	   
	return 0;  
}
//����ACKӦ��
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
//������ACKӦ��		    
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
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void PT_IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   
	TMP_SDA_OUT(); 	    
    IIC_TMP_SCL(0);//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {              
        IIC_TMP_SDA((txd&0x80)>>7);
        txd<<=1; 	  
		delay_us(2);   //��TEA5767��������ʱ���Ǳ����
		IIC_TMP_SCL(1);
		delay_us(2); 
		IIC_TMP_SCL(0);	
		delay_us(2);
    }	 
}
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
uint8_t PT_IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	TMP_SDA_IN();//SDA����Ϊ����
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
        PT_IIC_NAck();//����nACK
    else
        PT_IIC_Ack(); //����ACK   
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
	
	if(value & 0x800){//���¶�
		tmp = -1.0 * (0xFFF - value) / 16.0;
	}else{//���¶�
		tmp = value / 16.0;
	}
	return tmp;
}

void TMP101_Init(void) 
{	
	//bit0: Shutdown Mode (SD) - 1 enable
	//bit1: Thermostat Mode (TM) - 1 Interrupt mode
	//bit2:Polarity (POL) - 1 Active high , State low
	/* дConfiguration Register */
	TMP101_WriteReg(0x01, 0xFF);
}

