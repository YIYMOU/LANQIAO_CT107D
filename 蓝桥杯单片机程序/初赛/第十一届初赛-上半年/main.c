#include <stc15f2k60s2.h>
#include "iic.h"

#define u8 unsigned char
#define u16 unsigned int

#define get() (P3 & 0X3F) | ( (P4 & 0X10) << 3) | ((P4 & 0X04) << 4)

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff};
u8 disbuf[] = {0,1,11,11,11,11,6,7,8};

u8 smg_mode = 0;

u8 undefined_key = 0;
u16 volt_tt = 0;
u16 volt;
u16 volt_seting;
u8 count = 0;
bit volt_flag = 0;
bit count_flag1 = 0;
bit count_flag2 = 0;
bit count_first = 1;
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
	P2 &= 0x1f;
	P2 |= p2;
	P0 = p0;
	P2 &= 0x1f;
}

void allinit(void)
{
	set_port(0x80,0xff);
	set_port(0xa0,0x00);
	set_port(0xc0,0xff);
	set_port(0xe0,0xff);
}

void KBD(void)
{
	static u8 key_buf = 0;
	u8 key_temp;
	P3 = 0X0F; P44 = 0; P42 = 0;
	key_temp = get();
	P3 = 0XF0; P44 = 1; P42 = 1;
	key_temp |= get();
	if(key_buf == 0 && key_temp != 0xff)
	{
		Delay5ms();
		P3 = 0X0F; P44 = 0; P42 = 0;
		key_temp = get();
		P3 = 0XF0; P44 = 1; P42 = 1;
		key_temp |= get();
		if(key_buf == 0 && key_temp != 0xff)
		{
			key_buf = key_temp;
		}
	}
	else if(key_buf && key_temp == 0xff)
	{
		Delay5ms();
		P3 = 0X0F; P44 = 0; P42 = 0;
		key_temp = get();
		P3 = 0XF0; P44 = 1; P42 = 1;
		key_temp |= get();
		if(key_buf && key_temp == 0xff)
		{
			switch(key_buf)
			{
				case 0x7e: undefined_key++; break; 
				case 0x7d: undefined_key++; break; 
				case 0x7b: undefined_key++; break; 
				case 0x77: undefined_key++; break; 
				case 0xbe: undefined_key++; break; 
				case 0xbd: undefined_key++; break; 
				case 0xbb: undefined_key++; break; 
				case 0xb7: undefined_key++; break; 
				case 0xde: undefined_key++; break; 
				case 0xdd: undefined_key++; break; 
				case 0xdb:	//S12
					undefined_key = 0;
					if(smg_mode == 2)
						count = 0;
				break; 
				case 0xd7:	//S12
					undefined_key = 0;
					if(smg_mode == 1)
						write_at24c02(0x00,(u8)(volt_seting / 10));
					if(++smg_mode == 3) smg_mode = 0;
				break; 
				case 0xee: undefined_key++; break; 
				case 0xed: undefined_key++; break; 
				case 0xeb: 
					undefined_key = 0;
					if(smg_mode == 1)
					{
						if(volt_seting == 0) 
							volt_seting = 500;
						else
							volt_seting -= 50;
					}
				break; 
				case 0xe7: 
					undefined_key = 0;
					if(smg_mode == 1)
					{
						if(volt_seting >= 500) 
							volt_seting = 0;
						else
							volt_seting += 50;
					}
				break; 
			}
		}
		key_buf = 0;
	}
}

void Timer0Init(void)		//100微秒@12.000MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0x50;		//设置定时初值
	TH0 = 0xFB;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
}

void timer0(void) interrupt 1
{
	static u8 smg_tt = 0,smg_cnt = 1;
	static u8 led_temp = 0xff;
	if(++smg_tt == 20)
	{
		smg_tt = 0;
		if(smg_mode == 0 && (smg_cnt == 6 || smg_cnt == 1))
		{
			if(smg_cnt == 6)
			{
				set_port(0xc0,0x01 << (smg_cnt - 1));
				set_port(0xe0,TAB[disbuf[smg_cnt]] & 0x7f);
			}
			else if(smg_cnt == 1)
			{
				set_port(0xc0,0x01 << (smg_cnt - 1));
				set_port(0xe0,0xc1);
			}
		}
		else if(smg_mode == 1 && (smg_cnt == 6 || smg_cnt == 1))
		{
			if(smg_cnt == 6)
			{
				set_port(0xc0,0x01 << (smg_cnt - 1));
				set_port(0xe0,TAB[disbuf[smg_cnt]] & 0x7f);
			}
			else if(smg_cnt == 1)
			{
				set_port(0xc0,0x01 << (smg_cnt - 1));
				set_port(0xe0,0x8c);
			}
		}
		else if(smg_mode == 2 && (smg_cnt == 1))
		{
			set_port(0xc0,0x01 << (smg_cnt - 1));
			set_port(0xe0,0x89);
		}
		else
		{
			set_port(0xc0,0x01 << (smg_cnt - 1));
			set_port(0xe0,TAB[disbuf[smg_cnt]]);
		}
		if(++smg_cnt == 9) smg_cnt = 1;
	}
	if(volt_flag && ++volt_tt >= 50000)
	{
		volt_tt = 0;
		led_temp &= 0xfe;
	}
	else if(!volt_flag)
	{
		led_temp |= 0x01;
	}
	if(count % 2)
	{
		led_temp &= 0xfd;
	}
	else
	{
		led_temp |= 0x02;
	}
	if(undefined_key >= 3)
	{
		led_temp &= 0xfb;
	}
	else
	{
		led_temp |= 0x04;
	}
	set_port(0x80,led_temp);
}

void main(void)
{
	ET0 = 1;
	EA = 1;
	allinit();
	Timer0Init();
	Delay5ms();
	volt_seting = read_at24c02(0x00) * 10;
	while(1)
	{
		volt = (u16)(read_pcf8591(3) * 1.96 + 0.5);
		volt = (u16)(read_pcf8591(3) * 1.96 + 0.5);
		if(volt < volt_seting)
		{
			volt_flag = 1;
		}
		else
		{
			volt_flag = 0;
			volt_tt = 0;
		}
		if(volt > volt_seting && count_first)
		{
			count_first = 0;
			count_flag2 = 1;
			count_flag1 = 0;
		}
		else if(count_first && (volt < volt_seting))
		{
			count_first = 0;
			count_flag2 = 0;
			count_flag1 = 1;
		}
		if((volt > volt_seting) && count_flag1)
		{
			count++;
			count_flag2 = 1;
			count_flag1 = 0;
		}
		else if(count_flag2 && (volt < volt_seting))
		{
			count++;
			count_flag2 = 0;
			count_flag1 = 1;
		}
		if(smg_mode == 0)
		{
			disbuf[6] = volt / 100;
			disbuf[7] = volt / 10 % 10;
			disbuf[8] = volt % 10;
		}
		else if(smg_mode == 1)
		{
			disbuf[6] = volt_seting / 100;
			disbuf[7] = volt_seting / 10 % 10;
			disbuf[8] = volt_seting % 10;
		}
		else if(smg_mode == 2)
		{
			disbuf[6] = 11;
			disbuf[7] = count / 10 % 10;
			disbuf[8] = count % 10;
		}
		KBD();
	}
}
