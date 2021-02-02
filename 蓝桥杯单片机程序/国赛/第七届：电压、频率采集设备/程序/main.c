#include <STC15F2K60S2.H>
#include "iic.h"
#include "ds1302.h"
#include "intrins.h"

#define u8 unsigned char
#define u16 unsigned int
// 7 6 5 4 3 2 1 0
#define get() (P3 & 0X3F) | ((P4 & 0X04) << 4) | ((P4 & 0X10) << 3)

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0XBF,0XFF};
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};

extern u8 TIME[] ;

u8 key_buf = 0;
u8 smg_cnt = 0;

u8 mode;

u16 fre = 0;
u16 fre_tt = 0;
bit fre_start = 0;
bit fre_flag = 1;

u16 Vl = 1000;
u16 Vh = 2000;
u16 Vl_temp = 0;
u16 Vh_temp = 0;

u16 sec_tt = 0;
bit flicker_flag = 0;

u16 ad = 0;

u8 hour = 0;
u8 min = 0;
u8 sec = 0;
u8 event = 0;

u8 time_flag = 0;

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

void display(void)
{
	if(mode == 0)
	{
		disbuf[1] = 10;
		disbuf[2] = 1;
		disbuf[3] = 10;
		disbuf[4] = 11;
		disbuf[5] = ad / 1000;
		disbuf[6] = ad / 100 % 10;
		disbuf[7] = ad / 10 % 10;
		disbuf[8] = ad % 10;
		
	}
	else if(mode == 1)
	{
		disbuf[1] = TIME[2] / 16;
		disbuf[2] = TIME[2] % 16;
		disbuf[3] = 10;
		disbuf[4] = TIME[1] / 16;
		disbuf[5] = TIME[1] % 16;
		disbuf[6] = 10;
		disbuf[7] = TIME[0] / 16;
		disbuf[8] = TIME[0] % 16;
	}
	else if(mode == 3 || mode == 4)
	{
		disbuf[1] = Vh_temp / 1000;
		disbuf[2] = Vh_temp / 100 % 10;
		disbuf[3] = Vh_temp / 10 % 10;
		disbuf[4] = Vh_temp % 10;
		disbuf[5] = Vl_temp / 1000;
		disbuf[6] = Vl_temp / 100 % 10;
		disbuf[7] = Vl_temp / 10 % 10;
		disbuf[8] = Vl_temp % 10;
	}
	else if(mode == 5 || mode == 6 || mode == 7)
	{
		disbuf[1] = hour / 10;
		disbuf[2] = hour % 10;
		disbuf[3] = 10;
		disbuf[4] = min / 10;
		disbuf[5] = min % 10;
		disbuf[6] = 10;
		disbuf[7] = sec / 10;
		disbuf[8] = sec % 10;
	}
	else if(mode == 8)
	{
		if(fre_flag)
		{
			disbuf[1] = 10;
			disbuf[2] = 2;
			disbuf[3] = 10;
			disbuf[4] = fre / 10000;
			disbuf[5] = fre / 1000 % 10;
			disbuf[6] = fre / 100 % 10;
			disbuf[7] = fre / 10 % 10;
			disbuf[8] = fre % 10;
		}
		else
		{ // 1000000
			disbuf[1] = 10;
			disbuf[2] = 2;
			disbuf[3] = 10;
			disbuf[4] = (1000000 / fre) / 10000;
			disbuf[5] = (1000000 / fre) / 1000 % 10;
			disbuf[6] = (1000000 / fre) / 100 % 10;
			disbuf[7] = (1000000 / fre) / 10 % 10;
			disbuf[8] = (1000000 / fre) % 10;
		}
	}
	else if(mode == 9)
	{
		if(time_flag)
		{
			disbuf[1] = hour / 16;
			disbuf[2] = hour % 16;
			disbuf[3] = 10;
			disbuf[4] = min / 16;
			disbuf[5] = min % 16;
			disbuf[6] = 10;
			disbuf[7] = sec / 16;
			disbuf[8] = sec % 16;	
		}
		else
		{
			disbuf[1] = 11;
			disbuf[2] = 11;
			disbuf[3] = 11;
			disbuf[4] = 11;
			disbuf[5] = 11;
			disbuf[6] = 11;
			disbuf[7] = 0;
			disbuf[8] = event;
		}
	}
}

void KBD(void)
{
	u8 key_temp;
	P3 |= 0X0F; P44 = 0; P42 = 0;
	key_temp = get();
	P3 &= 0XF0; P44 = 1; P42 = 1;
	key_temp |= get();
	key_temp |= 0X30;
	if(key_temp != 0xff && key_buf == 0)
	{
		Delay5ms();
		P3 |= 0X0F; P44 = 0; P42 = 0;
		key_temp = get();
		P3 &= 0XF0; P44 = 1; P42 = 1;
		key_temp |= get();
		key_temp |= 0X30;	
		if(key_temp != 0xff && key_buf == 0)
		{
			key_buf = key_temp;
		}
	}
	else if(key_temp == 0xff && key_buf)
	{
		Delay5ms();
		P3 |= 0X0F; P44 = 0; P42 = 0;
		key_temp = get();
		P3 &= 0XF0; P44 = 1; P42 = 1;
		key_temp |= get();
		key_temp |= 0X30;
		if(key_temp == 0xff && key_buf)
		{
			switch(key_buf)
			{
				case 0x7e:
					if(mode == 0 || mode == 8 || mode == 9)
					{
						mode = 1;
					}
					else if(mode == 5 || mode == 6 || mode == 7)
					{
						TIME[0] = ((sec / 10) << 4) | (sec % 10);
						TIME[1] = ((min / 10) << 4) | (min % 10);
						TIME[2] = ((hour / 10) << 4) | (hour % 10);
						init_ds1302();
						mode = 1;
					}
				break;	// 显示时间信息
				case 0x7d:	// 电压测量
					if(mode == 1 || mode == 8 || mode == 9)
					{
						mode = 0;
					}
					else if(mode == 3 || mode == 4)
					{
						mode = 0;
						if(Vl_temp < Vh_temp)
						{
							write_eeprom(0x10,Vl_temp / 100);
							Delay5ms();
							write_eeprom(0x11,Vh_temp / 100);
							Delay5ms();
							Vl = Vl_temp;
							Vh = Vh_temp;
						}
					}
				break;
				case 0x7b: 
					if(mode == 1 ||  mode == 0 || mode == 9)
					{
						mode = 8;
						if(!fre_start)
						{
							fre_start = 1;
							TR0 = 0;
							TH0 = 0;
							TL0 = 0;
							fre_tt = 0;
							TR0 = 1;
						}
					}
				break;
				case 0x77: 
					if(mode == 0)
					{
						mode = 3;	// 电压阈值调整
						Vl_temp = Vl;
						Vh_temp = Vh;
					}
					else if(mode == 3)
					{
						mode = 4;
					}
					else if(mode == 4)
					{
						mode = 3;
					}
					else if(mode == 1)
					{
						mode = 5;
						sec = (TIME[0] / 16) * 10 + (TIME[0] % 16);
						min = (TIME[1] / 16) * 10 + (TIME[1] % 16);
						hour = (TIME[2] / 16) * 10 + (TIME[2] % 16);
					}
					else if(mode == 5)
					{
						mode = 6;
					}
					else if(mode == 6)
					{
						mode = 7;
					}
					else if(mode == 7)
					{
						mode = 5;
					}
					else if(mode == 8)
					{
						fre_flag = !fre_flag;
					}
					else if(mode == 9)
					{
						time_flag = !time_flag;
					}
				break;
				
				case 0xbe: 	// 加
					if(mode == 3)
					{
						if(Vh_temp < 5000)
						{
							Vh_temp += 500;
						}
					}
					else if(mode == 4)
					{
						if(Vl_temp < 5000)
						{
							Vl_temp += 500;
						}
					}
					else if(mode == 5)
					{
						if(hour < 24)
						{
							hour++;
						}
					}
					else if(mode == 6)
					{
						if(min < 24)
						{
							min++;
						}
					}
					else if(mode == 7)
					{
						if(sec < 24)
						{
							sec++;
						}
					}
				break;
				case 0xbd: 	// 减
					if(mode == 3)
					{
						if(Vh_temp > 0)
						{
							Vh_temp -= 500;
						}
					}
					else if(mode == 4)
					{
						if(Vl_temp > 0)
						{
							Vl_temp -= 500;
						}
					}
					else if(mode == 5)
					{
						if(hour > 0)
						{
							hour--;
						}
					}
					else if(mode == 6)
					{
						if(min > 0)
						{
							min--;
						}
					}
					else if(mode == 7)
					{
						if(sec > 0)
						{
							sec--;
						}
					}
				break;
				case 0xbb: 
					if(mode == 1 || mode == 0 || mode == 8)
					{
						mode = 9;
						event = read_eeprom(0x00);
						Delay5ms();
						sec = read_eeprom(0x01);
						Delay5ms();
						min = read_eeprom(0x02);
						Delay5ms();
						hour = read_eeprom(0x03);
						Delay5ms();
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
	bit flag = 1;
	set_port(0xc0,0x01 << smg_cnt);
	if(++sec_tt >= 1000)
	{
		sec_tt = 0;
		flicker_flag = !flicker_flag;
	}
	if(flicker_flag)
	{
		if(mode == 3 && (smg_cnt == 0 || smg_cnt == 1 || smg_cnt == 2 || smg_cnt == 3))
		{
			set_port(0xe0,TAB[11]);
		}
		else if(mode == 4 && (smg_cnt == 4 || smg_cnt == 5 || smg_cnt == 6 || smg_cnt == 7))
		{
			set_port(0xe0,TAB[11]);
		}
		else if(mode == 5 && ((smg_cnt == 0) || (smg_cnt == 1)))
		{
			set_port(0xe0,TAB[11]);
		}
		else if(mode == 6 && ((smg_cnt == 3) || (smg_cnt == 4)))
		{
			set_port(0xe0,TAB[11]);
		}
		else if(mode == 7 && ((smg_cnt == 6) || (smg_cnt == 7)))
		{
			set_port(0xe0,TAB[11]);
		}
		else
		{
			set_port(0xe0,TAB[disbuf[smg_cnt + 1]]);
		}
	}
	else
		set_port(0xe0,TAB[disbuf[smg_cnt + 1]]);
	if(++smg_cnt >= 8) smg_cnt = 0;
	
	if(fre_start && ++fre_tt >= 1000)
	{
		TR0 = 0;
		fre_tt = 0;
		fre = (TH0 << 8) | TL0;
		TH0 = 0;
		TL0 = 0;
		TR0 = 1;
	}
}

void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xf0;		//设置定时器模式
	TMOD |= 0x04;		//设置定时器模式
	TL0 = 0;		//设置定时初值
	TH0 = 0;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 0;		//定时器0开始计时
}

void main(void)
{
	Timer2Init();
	allinit();
	IE2 |= 0X04;
	EA = 1;
	init_ds1302();
	Timer0Init();
	write_pcf8591(153);
	event = read_eeprom(0x00);
	Delay5ms();
	sec = read_eeprom(0x01);
	Delay5ms();
	min = read_eeprom(0x02);
	Delay5ms();
	hour = read_eeprom(0x03) * 100;
	Delay5ms();
	Vl = read_eeprom(0x10) * 100;
	Delay5ms();
	Vh = read_eeprom(0x11);
	Delay5ms();
	if(Vh % 500) Vh = 2000;
	if(Vl % 500) Vl = 1000;
 	while(1)
	{
		if(mode == 0)
		{
			ad = (u16)read_pcf8591(3) * 19.6;
			ad = (u16)read_pcf8591(3) * 19.6;
		}
		read_ds1302();
		if(ad >	Vh || ad <Vl)
		{
			if(ad >	Vh)
				write_eeprom(0x00,1);
			else
				write_eeprom(0x00,0);
			Delay5ms();
			write_eeprom(0x01,TIME[0]);
			Delay5ms();
			write_eeprom(0x02,TIME[1]);
			Delay5ms();
			write_eeprom(0x03,TIME[2]);
			Delay5ms();
		}
		display();
		KBD();
	}
}