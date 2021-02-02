#include <STC15F2K60S2.H>
#include "iic.h"
#include "ds1302.h"
#define u8 unsigned char 
#define u16 unsigned int
u8 code LED[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff};
u8 yi=0,er=8,san=10,si=3,wu=0,liu=10,qi=9,ba=9,zidong=1,shidu,fazhi=50,shezhi=0,flag=0,relay=0;
extern unsigned char Time[];
void INIT(void)
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
void buzz_open(void)
{
		P2=0XA0;
		P0|=0X40;
}
void buzz_close(void)
{
		P2=0XA0;
		P0&=0Xbf;
}
void relay_open(void)
{
		P2=0XA0;
		P0|=0X10;
}
void relay_close(void)
{
		P2=0XA0;
		P0&=0Xef;
}
void delay_ms(u16 ms)
{
	u16 i,j;
	for(i=ms ; i>0 ; i--)
		for(j=845 ; j>0 ; j--);
}
void display(void)
{
	P2=0XC0;
	P0=0X01;
	P2=0XFF;
	P0=LED[yi];
	delay_ms(1);
	
	P2=0XC0;
	P0=0X02;
	P2=0XFF;
	P0=LED[er];
	delay_ms(1);
	
	P2=0XC0;
	P0=0X04;
	P2=0XFF;
	P0=LED[san];
	delay_ms(1);
	
	P2=0XC0;
	P0=0X08;
	P2=0XFF;
	P0=LED[si];
	delay_ms(1);
	
	P2=0XC0;
	P0=0X10;
	P2=0XFF;
	P0=LED[wu];
	delay_ms(1);
	
	P2=0XC0;
	P0=0X20;
	P2=0XFF;
	P0=LED[liu];
	delay_ms(1);
	
	P2=0XC0;
	P0=0X40;
	P2=0XFF;
	P0=LED[qi];
	delay_ms(1);
	
	P2=0XC0;
	P0=0X80;
	P2=0XFF;
	P0=LED[ba];
	delay_ms(1);
}
void KEY_BTN(void)
{
	if(P30==0)
	{
		delay_ms(5);
		if(P30==0)
		{
			zidong=!zidong;
		}
		while(!P30);
	}
	if(P31==0)
	{
		delay_ms(5);
		if(P31==0)
		{
			if(!zidong)
			{
				flag=!flag;
			}
			else
			{
				if(!shezhi)
				{
					shezhi=1;
					yi=10;
					er=10;
					san=11;
					si=11;
					wu=11;
					liu=11;
				}
				else
				{
					shezhi=0;
					Write_AT24C02(123,0x0a);
					delay_ms(5);
					//temp=Read_AT24C02(0x0a);
				}
			}
		}
		while(!P31);
	}
	if(P32==0)
	{
		delay_ms(5);
		if(P32==0)
		{
			if(!zidong)
			{
				relay=1;
			}
			else
			{
				if(shezhi)
				{
					fazhi++;
				}
			}
		}
		while(!P32);
	}
	if(P33==0)
	{
		delay_ms(5);
		if(P33==0)
		{
			if(!zidong)
			{
				relay=0;
			}
			else
			{
				if(shezhi)
				{
					fazhi--;
				}
			}
		}
		while(!P33);
	}
}
void main(void)
{
	INIT();
	Ds1302_Init();
	while(1)
	{	
		
		if(shezhi)
		{
			qi=fazhi/10;
			ba=fazhi%10;
		}
		else
		{
			DS1302_Timeget();
			si=Time[1]/16;wu=Time[1]%16;san=10;
			yi=Time[2]/16;er=Time[2]%16;liu=11;
			shidu=Read_PCF8591(0X03);
			shidu=shidu*0.39;
			qi=shidu/10;
			ba=shidu%10;
			if(zidong)
			{
				flag=0;
				relay=0;
				P2=0X80;
				P0=0XFE;
				buzz_close();
				if(shidu<fazhi)
				{
					relay_open();
				}
				else
				{
					relay_close();
				}
			}
			else
			{
				P2=0X80;
				P0=0XFD;
				if(shidu<fazhi)
				{
					if(flag)
					{
						buzz_close();
					}
					else
					{
						buzz_open();
					}
				}
				else
				{
					buzz_close();
				}
				if(!relay)
				{
					relay_close();
				}
				else
				{
					relay_open();
				}
			}
		}
		KEY_BTN();
		display();
	}
}