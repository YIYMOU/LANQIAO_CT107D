#include<stc15f2k60s2.h>
#include "onewire.h"

#define u8 unsigned char
#define u16 unsigned int

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff};
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};

u16 timer_tt = 0;
u16 timer_cnt = 0;

u8 temperature;

u8 pwm_tt = 0;
u8 pwm_level = 2;

u8 mode = 0;

u8 s6_flag = 0;

bit s7_flag = 0;

void delay_ms(u16 ms)
{
	u16 i,j;
	for(i = ms; i > 0; i--)
		for(j = 845; j > 0; j--);
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
	set_port(0xe0,0x00);
}

void BTN(void)
{
	
	static u8 key_buf = 0;
	u8 key_temp = P3 & 0X0F;
	if(!key_buf && key_temp != 0x0f)
	{
		delay_ms(5);
		key_temp = P3 & 0X0F;
		if(!key_buf && key_temp != 0x0f)
		{
			key_buf = key_temp;
		}
	}
	else if(key_buf && key_temp == 0x0f)
	{
		delay_ms(5);
		key_temp = P3 & 0X0F;
		if(key_buf && key_temp == 0x0f)
		{
			switch(key_buf)
			{
				case 0x0e: s7_flag = !s7_flag; break;
				case 0x0d: 
					timer_tt = 0;
					pwm_level = 0;
				break;
				case 0x0b: 
					if(s6_flag == 0)
					{
						s6_flag = 1;
						timer_tt = 60;
					}
					else if(s6_flag == 1)
					{
						s6_flag = 2;
						timer_tt = 120;
					}
					else
					{
						s6_flag = 0;
						timer_tt = 0;
					}
				break;
				case 0x07: if(++mode == 3) mode = 0; break;
			}
		}
		key_buf = 0;
	}
}

void Timer0Init(void)		//100微秒@11.0592MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xAE;		//设置定时初值
	TH0 = 0xFB;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
}

void timer0(void) interrupt 1
{
	static u8 smg_tt = 0,smg_cnt = 1;
	if(++smg_tt >= 20)
	{
		smg_tt = 0;
		if(s7_flag && smg_cnt == 8)
		{
			set_port(0xc0,0x01 << (smg_cnt - 1));
			set_port(0xe0,0xc6);
		}
		else
		{
			set_port(0xc0,0x01 << (smg_cnt - 1));
			set_port(0xe0,TAB[disbuf[smg_cnt]]);
		}
		if(++smg_cnt == 9) smg_cnt = 1;
	}
	pwm_tt++;
	if(pwm_tt == 10)
	{
		pwm_tt = 0;
	}
	else if(pwm_tt > pwm_level)
	{
		P34 = 0;
	}
	else
	{
		P34 = 1;
	}
	
	if(++timer_cnt >= 10000)
	{
		timer_cnt = 0;
		if(timer_tt)
			timer_tt--;
	}
	
	if(timer_tt == 0)
	{
		pwm_level = 0;
		set_port(0x80,0xff);
	}
	else if(mode == 0)
	{
		pwm_level = 2;
		set_port(0x80,0xfe);
	}
	else if(mode == 1)
	{
		pwm_level = 3;
		set_port(0x80,0xfd);
	}
	else if(mode == 2)
	{
		pwm_level = 7;
		set_port(0x80,0xfb);
	}
}

void main(void)
{
	Timer0Init();
	allinit();
	ET0 = 1;
	EA = 1;
	while(1)
	{
		if(s7_flag)
		{
			temperature = (u8)temp_read(); 
			disbuf[1] = 10;
			disbuf[2] = 4;
			disbuf[3] = 10;
			disbuf[4] = 11;
			disbuf[5] = 11;
			disbuf[6] = temperature / 10;
			disbuf[7] = temperature % 10;
		}
		else
		{
			disbuf[1] = 10;
			disbuf[2] = mode + 1;
			disbuf[3] = 10;
			disbuf[4] = 11;
			disbuf[5] = timer_tt / 1000;
			disbuf[6] = timer_tt / 100 % 10;
			disbuf[7] = timer_tt / 10 % 10;
			disbuf[8] = timer_tt % 10;
		}
		BTN();
	}
}
