/*
  ����˵��: IIC������������
  �������: Keil uVision 4.10 
  Ӳ������: CT107��Ƭ���ۺ�ʵѵƽ̨ 8051��12MHz
  ��    ��: 2011-8-9
*/

#include "reg52.h"
#include "intrins.h"

#define somenop {_nop_();_nop_();_nop_();_nop_();_nop_();}    


#define SlaveAddrW 0xA0
#define SlaveAddrR 0xA1

//�������Ŷ���
sbit SDA = P2^1;  /* ������ */
sbit SCL = P2^0;  /* ʱ���� */


//������������
void IIC_Start(void)
{
	SDA = 1;
	SCL = 1;
	somenop;somenop;somenop;somenop;somenop;somenop;somenop;somenop;
	SDA = 0;
	somenop;somenop;somenop;somenop;somenop;somenop;somenop;somenop;
	SCL = 0;	
}

//����ֹͣ����
void IIC_Stop(void)
{
	SDA = 0;
	SCL = 1;
	somenop;somenop;somenop;somenop;somenop;somenop;somenop;somenop;
	SDA = 1;
}

//Ӧ��λ����
void IIC_Ack(bit ackbit)
{
	if(ackbit) 
	{	
		SDA = 0;
	}
	else 
	{
		SDA = 1;
	}
	somenop;somenop;somenop;somenop;somenop;somenop;somenop;somenop;
	SCL = 1;
	somenop;somenop;somenop;somenop;somenop;somenop;somenop;somenop;
	SCL = 0;
	SDA = 1; 
	somenop;somenop;somenop;somenop;somenop;somenop;somenop;somenop;
}

//�ȴ�Ӧ��
bit IIC_WaitAck(void)
{
	SDA = 1;
	somenop;somenop;somenop;somenop;somenop;somenop;somenop;somenop;
	SCL = 1;
	somenop;somenop;somenop;somenop;somenop;somenop;somenop;somenop;
	if(SDA)    
	{   
		SCL = 0;
		IIC_Stop();
		return 0;
	}
	else  
	{ 
		SCL = 0;
		return 1;
	}
}

//ͨ��I2C���߷�������
void IIC_SendByte(unsigned char byt)
{
	unsigned char i;
	for(i=0;i<8;i++)
	{   
		if(byt&0x80) 
		{	
			SDA = 1;
		}
		else 
		{
			SDA = 0;
		}
		somenop;somenop;somenop;somenop;somenop;somenop;somenop;somenop;
		SCL = 1;
		byt <<= 1;
		somenop;somenop;somenop;somenop;somenop;somenop;somenop;somenop;
		SCL = 0;
	}
}

//��I2C�����Ͻ�������
unsigned char IIC_RecByte(void)
{
	unsigned char da;
	unsigned char i;
	
	for(i=0;i<8;i++)
	{   
		SCL = 1;
		somenop;somenop;somenop;somenop;somenop;somenop;somenop;somenop;
		da <<= 1;
		if(SDA) 
		da |= 0x01;
		SCL = 0;
		somenop;somenop;somenop;somenop;somenop;somenop;somenop;somenop;
	}
	return da;
}

unsigned char read_pcf8591(unsigned char ch)
{
	unsigned char ret;
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(ch);
	IIC_WaitAck();
	
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	ret = IIC_RecByte();
	IIC_Ack(0);
	IIC_Stop();
	return ret;
}

void write_pcf8591(unsigned char add, unsigned char dat)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(add);
	IIC_WaitAck();
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}