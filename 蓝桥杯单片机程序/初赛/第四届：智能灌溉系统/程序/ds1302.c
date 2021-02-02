/*
  ����˵��: DS1302��������
  �������: Keil uVision 4.10 
  Ӳ������: CT107��Ƭ���ۺ�ʵѵƽ̨ 8051��12MHz
  ��    ��: 2011-8-9
*/

#include <reg52.h>
#include <intrins.h>

sbit SCK=P1^7;		
sbit SDA=P2^3;		
sbit RST = P1^3;   // DS1302��λ												
unsigned char Time[]={0x00,0x30,0x08,0x00,0x00,0x00,0x00};
void Write_Ds1302_Byte(unsigned  char temp) 
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

void Write_Ds1302( unsigned char address,unsigned char dat )     
{
 	RST=0;
	_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
 	SCK=0;
	_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
 	RST=1;	
   	_nop_();  _nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
 	Write_Ds1302_Byte(address);	
 	Write_Ds1302_Byte(dat);		
 	RST=0; 
}

unsigned char Read_Ds1302 ( unsigned char address )
{
 	unsigned char i,temp=0x00;
 	RST=0;
	_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
 	SCK=0;
	_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
 	RST=1;
	_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
 	Write_Ds1302_Byte(address);
 	for (i=0;i<8;i++) 	
 	{		
		SCK=0;
		temp>>=1;	
 		if(SDA)
 		temp|=0x80;	
 		SCK=1;
	} 
 	RST=0;
	_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
 	RST=0;
	SCK=0;
	_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
	SCK=1;
	_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
	SDA=0;
	_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
	SDA=1;
	_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
	return (temp);			
}
void Ds1302_Init(void)
{
	unsigned char add=0x80,i;
	Write_Ds1302(0x8e,0x00);     
	for(i=0 ; i<7 ; i++)
	{
		Write_Ds1302(add,Time[i]);
		add+=2;
	}
	Write_Ds1302(0x8e,0x80);     
}
void DS1302_Timeget(void)
{
	unsigned char add=0x81,i;
	Write_Ds1302(0x8e,0x00);     
	for(i=0 ; i<7 ; i++)
	{
		Time[i]=Read_Ds1302(add);
		add+=2;
	}
	Write_Ds1302(0x8e,0x80);  
}