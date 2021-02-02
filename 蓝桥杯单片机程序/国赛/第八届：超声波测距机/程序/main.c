#include <STC15F2K60S2.H>
#include "iic.h"

#define u8 unsigned char
#define u16 unsigned int

sbit TX = P1^0;
sbit RX = P1^1;

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff,0xc6,0x8e}; // 12---C，13---F
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};

u8 key_buf = 0;
u8 smg_cnt = 0;

u8 mode = 0;
u8 res = 0;

u16 distance = 0;
u16 distance_last;
u16 distance_second;
u16 wave_tt = 0;
bit wave_flag = 0;

u8 mang = 20;

u8 measure_intr = 0;
u8 distance_intr = 0;

bit led1_open = 0;
u16 led1_tt = 0;
bit led1_flag = 0;
u8 led1_cnt = 0;


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

void display(void)
{
	if(mode == 0)
	{
		disbuf[1] = 12;
		disbuf[2] = 11;
		disbuf[3] = distance / 100;
		disbuf[4] = distance / 10 % 10;
		disbuf[5] = distance % 10;
		disbuf[6] = distance_last / 100;
		disbuf[7] = distance_last / 10 % 10;
		disbuf[8] = distance_last % 10;
	}
	else if(mode == 1)
	{
		disbuf[1] = distance_intr + 1;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = 11;
		disbuf[6] = distance_second / 100;
		disbuf[7] = distance_second / 10 % 10;
		disbuf[8] = distance_second % 10;
	}
	else if(mode == 2)
	{
		disbuf[1] = 13;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = 11;
		disbuf[6] = 11;
		disbuf[7] = mang / 10;
		disbuf[8] = mang % 10;
	}
}

void BTN(void)
{
	u8 key_temp;
	key_temp = P3 & 0X0F;
	if(key_temp != 0x0f && key_buf == 0)
	{
		Delay5ms();
		key_temp = P3 & 0X0F;
		if(key_temp != 0x0f && key_buf == 0)
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
					if(mode == 1)
					{
						distance_intr = (distance_intr + 1) % 4;
						distance_second = read_eeprom(distance_intr);
					}
					else if(mode == 2)
					{
						switch(mang)
						{
							case 0: mang = 10; break;
							case 10: mang = 20; break;
							case 20: mang = 30; break;
							case 30: mang = 0; break;
						}
					}
				break;
				case 0x0d: 	// 参数设置
					if(mode == 2)
					{
						mode = 0;
						write_eeprom(0x04,mang);
					}
					else
					{
						mode = 2;
					}
				break;
				case 0x0b: // 数据回显
					if(mode == 1)
					{
						mode = 0;
					}
					else if(mode != 2)
					{
						mode = 1;
						distance_intr = 0;
						distance_second = read_eeprom(distance_intr);
					}
				break;
				case 0x07: 	// 启动测量
					if(mode != 2)
					{
						wave_flag = 1;
						mode = 0;
					}
				break;
			}
			key_buf = 0;
		}
	}
}

void Timer2Init(void)		//1毫秒@12.000MHz
{
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = 0x20;		//设置定时初值
	T2H = 0xD1;		//设置定时初值
	AUXR |= 0x10;		//定时器2开始计时
}

void timer2(void) interrupt 12
{
	u8 led_state = 0xff;
	
	set_port(0xe0,TAB[11]);	
	set_port(0xc0,0x01 << smg_cnt);
	set_port(0xe0,TAB[disbuf[smg_cnt + 1]]);
	if(++smg_cnt >= 8) smg_cnt = 0;
//	
//	if(++wave_tt >= 300)
//	{
//		wave_tt = 0;
//		wave_flag = 1;
//	}
	
	if(led1_open && ++led1_tt >= 1000)
	{
		led1_tt = 0;
		led1_flag = !led1_flag;
		if(led1_flag)
		{
			led1_cnt++;
			if(led1_cnt == 3)
			{
				led1_open = 0;
				led1_tt = 0;
			}
		}
	}
	
	if(led1_flag == 0 && led1_open)
	{
		led_state &= 0xfe;
	}
	
	if(mode == 2)
	{
		led_state &= 0xbf;
	}
	if(mode == 1)
	{
		led_state &= 0x7f;
	}
	set_port(0x80,led_state);
}

void Timer1Init(void)		//12微秒@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0xF4;		//设置定时初值
	TH1 = 0xFF;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 0;		//定时器1关闭
}

void wave_send(void)
{
	u8 num = 10;
	TX = 0;
	TL1 = 0xF4;		//设置定时初值
	TH1 = 0xFF;		//设置定时初值
	TR1 = 1;		//定时器1打开
	while(num--)
	{
		TX ^= 1;
		while(!TF1);
		TF1 = 0;
	}
	TR1 = 0;
	TL1 = 0;		//设置定时初值
	TH1 = 0;		//设置定时初值
	TR1 = 1;		//定时器1打开
	while(RX && !TF1);
	TR1 = 0;
	if(TF1)
	{
		TF1 = 0;
		distance = 99;
	}
	else
	{
		distance = ((TH1 << 8) | TL1) * 0.017;
		// x / 1000000 /2 * 340 * 100
	}
}

void main(void)
{
	allinit();
	Timer2Init();
	Timer1Init();
	IE2 |= 0X04;
	EA = 1;
	write_eeprom(0x00,222);
	Delay5ms();
	res = read_eeprom(0x00);
	while(1)
	{
		if(wave_flag)
		{
			wave_flag = 0;
			distance_last = distance;
			wave_send();
			write_eeprom(measure_intr,distance);
			Delay5ms();
			measure_intr = (measure_intr + 1) % 4;
			if(distance <= mang)
			{
				write_pcf8591(0);
			}
			else
			{
				if((distance - mang) * 1.02 > 255)
				{
					write_pcf8591(255);
				}
				else 
					write_pcf8591((distance - mang) * 1.02);
			}
			Delay5ms();
			led1_cnt = 0;
			led1_tt = 0;
			led1_open = 1;
		}
		BTN();
		display();
	}
}

