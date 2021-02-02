#include <STC15F2K60S2.H>
#include "iic.h"
#define u8 unsigned char
#define u16 unsigned int
u8 code LED[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff};
u8 yi,er,san,si,wu,liu,qi,ba;
u8 shidu,chushui=0,cnt=0;
u16 num=0;
void delay_ms(u16 ms)
{
	u16 i,j;
	for(i=ms;i>0;i--)
		for(j=845;j>0;j--);
}
void INIT()
{
	P2=0XA0;
	P0=0X00;
	
	P2=0X80;
	P0=0XFF;
	
	P2=0XC0;
	P0=0XFF;
	P2=0XFF;
	P0=0XFF;
}
void KEY_BTN()
{
	if(P30==0)
	{
		delay_ms(5);
		if(P30==0)
		{
			if(chushui==0)
			{
				num = 0;
				P2=0XA0;
				P0=0X10;
				chushui=1;
				ET0=1;
				EA=1;
			}
		}
		while(!P30);
	}
	else if(P31==0)
	{
		delay_ms(5);
		if(P31==0){
			if(chushui)
			{
				num*=0.5;
				ba=num%10;
				qi=num%100/10;
				liu=num%1000/100;
				wu=num/1000;
				//num=0;
				P2=0XA0;
				P0=0X00;
				ET0=0;
				EA=0;
				chushui=0;
			}
			
		}
		while(!P31);
	}
}
void display()
{

	P2=0xc0;
	P0=0X01;
	P2=0XFF;
	P0=LED[yi];
	delay_ms(1);
	
	P2=0xc0;
	P0=0X02;
	P2=0XFF;
	P0=LED[er]&0X7F;
	delay_ms(1);
	
	P2=0xc0;
	P0=0X04;
	P2=0XFF;
	P0=LED[san];
	delay_ms(1);
	
	P2=0xc0;
	P0=0X08;
	P2=0XFF;
	P0=LED[si];
	delay_ms(1);
	
	P2=0xc0;
	P0=0X10;
	P2=0XFF;
	P0=LED[wu];
	delay_ms(1);
	
	P2=0xc0;
	P0=0X20;
	P2=0XFF;
	P0=LED[liu]&0X7F;
	delay_ms(1);
	
	P2=0xc0;
	P0=0X40;
	P2=0XFF;
	P0=LED[qi];
	delay_ms(1);
	
	P2=0xc0;
	P0=0X80;
	P2=0XFF;
	P0=LED[ba];
	delay_ms(1);
	
}
void Timer0Init(void)		//1毫秒@11.0592MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xCD;		//设置定时初值
	TH0 = 0xD4;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
}

void timer0(void)interrupt 1
{
	if(++cnt>99)
	{
		cnt=0;
		num++;
		ba=num%10;
		qi=num%100/10;
		liu=num%1000/100;
		wu=num/1000;
		if(num==9999)
		{
				num*=0.5;
				ba=num%10;
				qi=num%100/10;
				liu=num%1000/100;
				wu=num/1000;
				//num=0;
				P2=0XA0;
				P0=0X00;
				ET0=0;
				EA=0;
				chushui=0;
		}
	}
}
void main()
{
	yi=11;er=0;san=5;si=0;wu=0;liu=1;qi=0;ba=0;
	INIT();
	Timer0Init();
	while(1)
	{
			KEY_BTN();
			display();
			shidu = Read_PCF8591();
			if(shidu > 64)
			{
				P2=0X80;
				P0=0XfF;
			}
			else
			{
				P2=0X80;
				P0=0Xfe;
			}
	}
}