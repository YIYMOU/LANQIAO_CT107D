#include <STC15F2K60S2.H>
#include "ds1302.h"
#include "onewire.h"

#define u8 unsigned char
#define u16 unsigned int

// 1 1 1 1 1 1 1 1 
#define get() (P3 & 0X3F) | ((P4 & 0X04) << 4) | ((P4 & 0X10) << 3)

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff};
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};
u8 key_buf = 0;
extern u8 TIME[];
u8 temperature[] = {0,0,0,0,0,0,0,0,0,0};
u8 set_time = 1;
u8 set_time_temp = 1;
u8 display_mode = 1;
u16 led_tt = 0;
u8 led_open = 0;
u8 index = 0;
u16 temperature_tt = 0;
u8 sec_cnt = 0;
u8 temp_cnt = 11;
u8 read_flag = 0;
bit seting = 1;
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

void set_display_mode(u8 mode)
{
	if(mode == 1)	//
	{
		disbuf[1] = 11;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = 11;
		disbuf[6] = 10;
		disbuf[7] = set_time_temp / 10;
		disbuf[8] = set_time_temp % 10;
	}
	else if(mode == 2)
	{
		disbuf[1] = TIME[2] / 16;
		disbuf[2] = TIME[2] % 16;
		if(TIME[0] % 2)
		{
			disbuf[3] = 10;
			disbuf[6] = 10;
		}
		else
		{
			disbuf[3] = 11;
			disbuf[6] = 11;
		}
		disbuf[4] = TIME[1] / 16;
		disbuf[5] = TIME[1] % 16;
		disbuf[7] = TIME[0] / 16;
		disbuf[8] = TIME[0] % 16;	
	}
	else if(display_mode == 3)
	{
		disbuf[1] = 10;
		disbuf[2] = index / 10;
		disbuf[3] = index % 10;
		disbuf[4] = 11;
		disbuf[5] = 11;
		disbuf[6] = 10;
		disbuf[7] = temperature[index] / 10;
		disbuf[8] = temperature[index] % 10;
	}
}
void set_port(u8 p2,u8 p0)
{
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

void BTN(void)
{
	u8 temp = P3 & 0X0F;
	if(temp != 0x0f && key_buf == 0)
	{
		Delay5ms();
		temp = P3 & 0x0f;
		if(temp != 0x0f && key_buf == 0)
		{
			key_buf = temp;
		}
	}
	else if(temp == 0x0f && key_buf)
	{
		Delay5ms();
		temp = P3 & 0x0f;
		if(temp == 0x0f && key_buf)
		{
			switch(key_buf)
			{
				case 0x0e: 
					if(display_mode == 3)
					{
						seting = 1;
						display_mode = 1;
					}
				break;
				case 0x0d: 
					if(display_mode == 3)
					{
						index = (index + 1) %10;	
						led_open = 0;
					}
				break;
				case 0x0b: 
					if(display_mode == 1)
					{
						set_time = set_time_temp;
						set_time_temp = 1;
						display_mode = 2;
						temp_cnt = 0;
						temperature_tt = 0;
						sec_cnt = 0;
					}
				break;
				case 0x07: 
					if(display_mode == 1)
					{
						if(set_time_temp == 0)
						{
							set_time_temp = 1;
						}
						if(set_time_temp == 1)
						{
							set_time_temp = 5;
						}
						else if(set_time_temp == 5)
						{
							set_time_temp = 30;
						}
						else if(set_time_temp == 30)
						{
							set_time_temp = 60;
						}
						else if(set_time_temp == 60)
						{
							set_time_temp = 1;
						}
					}
				break;
			}
		}
		key_buf = 0;
	}
}

//void KBD(void)
//{
//	u8 key_temp;
//	P3 = 0X0F; P44 = 0; P42 = 0;
//	key_temp = get();
//	P3 = 0Xf0; P44 = 1; P42 = 1;
//	key_temp |= get();
//	if(key_temp != 0xff && key_buf == 0)
//	{
//		Delay5ms();
//		P3 = 0X0F; P44 = 0; P42 = 0;
//		key_temp = get();
//		P3 = 0Xf0; P44 = 1; P42 = 1;
//		key_temp |= get();
//		if(key_temp != 0xff && key_buf == 0)
//		{
//			key_buf = key_temp;
//		}
//	}
//	else if(key_temp == 0xff && key_buf)
//	{
//		Delay5ms();
//		P3 = 0X0F; P44 = 0; P42 = 0;
//		key_temp = get();
//		P3 = 0Xf0; P44 = 1; P42 = 1;
//		key_temp |= get();
//		if(key_temp == 0xff && key_buf)
//		{
//			switch(key_buf)
//			{
//				case 0xe7: disbuf[1] = 0; disbuf[2] = 7; disbuf[3] = 10; break;
//				case 0xeb: disbuf[1] = 0; disbuf[2] = 6; disbuf[3] = 10; break;
//				case 0xed: disbuf[1] = 0; disbuf[2] = 5; disbuf[3] = 10; break;
//				case 0xee: disbuf[1] = 0; disbuf[2] = 4; disbuf[3] = 10; break;
//				case 0xd7: disbuf[1] = 1; disbuf[2] = 1; disbuf[3] = 10; break;
//				case 0xdb: disbuf[1] = 1; disbuf[2] = 0; disbuf[3] = 10; break;
//				case 0xdd: disbuf[1] = 0; disbuf[2] = 9; disbuf[3] = 10; break;
//				case 0xde: disbuf[1] = 0; disbuf[2] = 8; disbuf[3] = 10; break;
//				case 0xb7: disbuf[1] = 1; disbuf[2] = 5; disbuf[3] = 10; break;
//				case 0xbb: disbuf[1] = 1; disbuf[2] = 4; disbuf[3] = 10; break;
//				case 0xbd: disbuf[1] = 1; disbuf[2] = 3; disbuf[3] = 10; break;
//				case 0xbe: disbuf[1] = 1; disbuf[2] = 2; disbuf[3] = 10; break;
//				case 0x77: disbuf[1] = 1; disbuf[2] = 9; disbuf[3] = 10; break;
//				case 0x7b: disbuf[1] = 1; disbuf[2] = 8; disbuf[3] = 10; break;
//				case 0x7d: disbuf[1] = 1; disbuf[2] = 7; disbuf[3] = 10; break;
//				case 0x7e: disbuf[1] = 1; disbuf[2] = 6; disbuf[3] = 10; break;
//			}
//			
//		}
//		key_buf = 0;
//	}
//}

void Timer1Init(void)		//100微秒@12.000MHz
{
	AUXR |= 0x40;		//定时器时钟1T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0x50;		//设置定时初值
	TH1 = 0xFB;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 1;		//定时器1开始计时
}


void timer0(void) interrupt 3
{
	static u8 smg_tt = 0, smg_cnt = 0;
	static u8 led_flag = 0;
	if(++smg_tt == 20)
	{
		smg_tt = 0;
		set_port(0xc0, 0x01 << smg_cnt);
		set_port(0xe0, TAB[disbuf[smg_cnt + 1]]);
		if(++smg_cnt == 8) smg_cnt = 0;
	}
	
	if(++led_tt == 5000)
	{
		
		led_tt = 0;
		if(led_open == 0)
		{
			set_port(0x80,0xff);
		}
		else
		{
			if(led_flag)
			{
				set_port(0x80,0xfe);
			}
			else 
			{
				set_port(0x80,0xff);
			}
			led_flag = !led_flag;
		}
	}
	
	if(temp_cnt != 11 && ++temperature_tt == 10000)
	{
		temperature_tt = 0;
		if(temp_cnt == 10)
		{
			led_open = 1;
			display_mode = 3;
			index = 0;
			temp_cnt = 11;
		}
		else if(++sec_cnt == set_time)
		{
			sec_cnt = 0;
			read_flag = 1;
		}
	}
}

void main()
{
	allinit();
	Timer1Init();
	EA = 1;
	ET1 = 1;
	dis1302_init();
	while(1)
	{
		dis1302_read();
//		disbuf[1] = TIME[2] / 16;
//		disbuf[2] = TIME[2] % 16;
//		disbuf[3] = 10;
//		disbuf[4] = TIME[1] / 16;
//		disbuf[5] = TIME[1] % 16;
//		disbuf[6] = 10;
//		disbuf[7] = TIME[0] / 16;
//		disbuf[8] = TIME[0] % 16;
//		temperature = (u8)(rd_temperature());
		if(display_mode != 3)
		{
			index = 10;
		}
		if(read_flag)
		{
			temperature[temp_cnt++] = (u8)(rd_temperature());
			read_flag = 0;
		}
		set_display_mode(display_mode);
		BTN();
	}
	
}