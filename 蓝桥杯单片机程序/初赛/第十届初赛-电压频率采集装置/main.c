#include <stc15f2k60s2.h>
#include "iic.h"
#define u8 unsigned char
#define u16 unsigned int

#define get() (P3 & 0X3F) | ((P4 & 0X10) << 3) | ((P4 & 0X04) << 4)

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff,
	0x8e, // F -- 12
	0xc1 // U -- 13
};
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};

u16 zheng = 0,fan = 0,zheng_now = 0,fan_now = 0,fre_tt = 0,fre;
bit fre_flag = 0;

u16 volt,volt_tt = 0,s5_tt = 0;

bit s5_flag = 0;
bit s6_flag = 1;
bit s7_flag = 1;

bit mode = 0;
u8 led_state = 0xff;
void Delay5ms()		//@12.000MHz
{
	unsigned char i, j;

	i = 59;
	j = 90;
	do
	{
		while (--j);
	} while (--i);
}

void set_port(u8 p2,u8 p0)
{
	P0 = p0;
	P2 &= 0X1F;
	P2 |= p2;
	P0 = p0;
	P2 &= 0X1F;
}

void allinit(void)
{
	set_port(0x80,0xff);
	set_port(0xa0,0x00);
	set_port(0xc0,0xff);
	set_port(0xe0,0xff);
}

void display_mode1(void)
{
	disbuf[1] = 12;
	disbuf[2] = 11;
	disbuf[3] = 11;
	if(fre >= 10000)
	{
		disbuf[4] = fre / 10000;
		disbuf[5] = fre / 1000 % 10;
		disbuf[6] = fre / 100 % 10;
		disbuf[7] = fre / 10 % 10;
		disbuf[8] = fre % 10;
	}
	else if(fre >= 1000)
	{
		disbuf[4] = 11;
		disbuf[5] = fre / 1000 % 10;
		disbuf[6] = fre / 100 % 10;
		disbuf[7] = fre / 10 % 10;
		disbuf[8] = fre % 10;
	}
	else if(fre >= 100)
	{
		disbuf[4] = 11;
		disbuf[5] = 11;
		disbuf[6] = fre / 100 % 10;
		disbuf[7] = fre / 10 % 10;
		disbuf[8] = fre % 10;
	}
}

void display_mode2(void)
{
	disbuf[1] = 13;
	disbuf[2] = 11;
	disbuf[3] = 11;
	disbuf[4] = 11;
	disbuf[5] = 11;
	disbuf[6] = volt / 100 % 10;
	disbuf[7] = volt / 10 % 10;
	disbuf[8] = volt % 10;
}

void BTN(void)
{
	static key_buf = 0;
	u8 key_temp = P3 & 0X0F;
	if(key_temp != 0x0f && !key_buf)
	{
		Delay5ms();
		key_temp = P3 & 0X0F;
		if(key_temp != 0x0f && !key_buf)
		{
			key_buf = key_temp;
		}
	}
	else if(key_temp == 0x0f && key_buf)
	{
		Delay5ms();
		key_temp = P3 & 0X0F;
		if(key_temp == 0x0f && key_buf)
		{
			switch(key_buf)
			{
				case 0x0e: 
					s7_flag = !s7_flag;
				break;
				case 0x0d: 
					s6_flag = !s6_flag;
				break;
				case 0x0b: 	// s5
					s5_flag = !s5_flag;
					if(s5_flag == 0)
					{
						write_pcf8591(0x40,102);
						Delay5ms();
					}
				break;
				case 0x07: 	// s4
					mode = !mode;
				break;
			}
		}
		key_buf = 0;
	}
}

void Timer0Init(void)
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0x04;		//设置定时器模式
	TL0 = 0;		//设置定时初值
	TH0 = 0;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 0;		//定时器0开始计时
}

void Timer1Init(void)		//100微秒@12.000MHz
{
	AUXR |= 0x40;		//定时器时钟1T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0x50;		//设置定时初值
	TH1 = 0xFB;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 1;		//定时器1开始计时
}


void timer1(void) interrupt 3
{
	static u8 smg_cnt = 1,smg_tt = 0;
	if(++smg_tt >= 20)
	{
		smg_tt = 0;
		if(s7_flag)
		{
			if(mode == 0 && smg_cnt == 6)
			{
				set_port(0xc0,0x01 << (smg_cnt - 1));
				set_port(0xe0,TAB[disbuf[smg_cnt]] & 0x7f);
			}
			else
			{
				set_port(0xc0,0x01 << (smg_cnt - 1));
				set_port(0xe0,TAB[disbuf[smg_cnt]]);
			}
		}
		else
		{
			set_port(0xc0,0x01 << (smg_cnt - 1));
			set_port(0xe0,TAB[11]);
		}
		if(++smg_cnt >= 9) smg_cnt = 1;
	}
	
	if(++fre_tt >= 10000)
	{
		fre_tt = 0;
		TR0 = 0;
		fre = (TH0 << 8) | TL0;
		TH0 = 0;
		TL0 = 0;
		TR0 = 1;
	}
	
	if(++volt_tt >= 800)
	{
		volt_tt = 0;
	}
	
	if(s6_flag)
		{
			led_state = 0xff;
			if(mode)
			{
				led_state &= 0xfd;
			}
			else
			{
				led_state &= 0xfe;
			}
			if(volt >= 350 || volt < 250 && volt >= 150)
			{
				led_state &= 0xfb;
			}
			if(fre >= 10000 || fre >= 1000 && fre < 5000)
			{
				led_state &= 0xf7;
			}
			if(!s5_flag)
			{
				led_state &= 0xef;
			}
			set_port(0x80,led_state);
		}
		else
		{
			set_port(0x80,0xff);
		}
}

void main(void)
{
	allinit();
	Timer0Init();
	Timer1Init();
	ET1 = 1;
	EA = 1;
	write_pcf8591(0x40,102);
	Delay5ms();
	while(1)
	{
		BTN();
		if(volt_tt == 399 || volt_tt == 799)
		{
			volt = (u16)(read_pcf8591(3) * 1.96);
			if(s5_flag)
			{
				write_pcf8591(0x40,102);
			}
			else
			{
				write_pcf8591(0x40,volt / 2);
			}
		}
		if(mode == 0)
		{
			display_mode2();
		}
		else if(mode == 1)
		{
			display_mode1();
		}
	}
}