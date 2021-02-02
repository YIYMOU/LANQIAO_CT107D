/*
  ����˵��: DS1302��������
  �������: Keil uVision 4.10 
  Ӳ������: CT107��Ƭ���ۺ�ʵѵƽ̨ 8051��12MHz
  ��    ��: 2011-8-9
*/

#include <stc15f2k60s2.h>
#include <intrins.h>

sbit SCK=P1^7;		
sbit SDA=P2^3;		
sbit RST = P1^3;   // DS1302��λ												

unsigned char TIME[] = {0X55,0X59,0X23};

void Write_Ds1302(unsigned  char temp) 
{
	unsigned char i;
	for (i=0;i<8;i++)     	
	{ 
		SCK=0;
		SDA=temp&0x01;
		temp>>=1; 
		SCK=1;
	}
}   

void Write_Ds1302_Byte( unsigned char address,unsigned char dat )     
{
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1; 	_nop_();
 	Write_Ds1302(address);
 	Write_Ds1302(dat);
 	RST=0; 
}

unsigned char Read_Ds1302_Byte ( unsigned char address )
{
 	unsigned char i,temp=0x00;
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1;	_nop_();
 	Write_Ds1302(address);
 	for (i=0;i<8;i++) 	
 	{		
		SCK=0;
		temp>>=1;	
 		if(SDA)
 		temp|=0x80;	
 		SCK=1;
	} 
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
	SCK=1;	_nop_();
	SDA=0;	_nop_();
	SDA=1;	_nop_();
	return (temp);			
}
void init_ds1302(void)
{
	Write_Ds1302_Byte(0x8e,0x00);
	Write_Ds1302_Byte(0x80,TIME[0]);
	Write_Ds1302_Byte(0x82,TIME[1]);
	Write_Ds1302_Byte(0x84,TIME[2]);
	Write_Ds1302_Byte(0x8e,0x80);
}
void read_ds1302(void)
{
	Write_Ds1302_Byte(0x8e,0x00);
	TIME[0] = Read_Ds1302_Byte(0x80|1);
	TIME[1] = Read_Ds1302_Byte(0x82|1);
	TIME[2] = Read_Ds1302_Byte(0x84|1);
	Write_Ds1302_Byte(0x8e,0x80);
}