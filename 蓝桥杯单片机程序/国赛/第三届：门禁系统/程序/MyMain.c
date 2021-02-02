#include <stc15f2k60s2.h>
#include "ds1302.h"
#include "iic.h"

#define u8 unsigned char
#define u16 unsigned int

#define get() (P3 & 0x3f) | ((P4 & 0X04) << 4) | ((P4 & 0X10) << 3) // 8 4 2 1   8 4 2 1 


sbit TX = P1^0;
sbit RX = P1^1;

extern u8 TIME[];

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff};
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};

u8 smg_tt = 0,smg_cnt = 0;
u8 key_buf = 0;

u8 display_mode = 0;

bit zidong = 0;
bit correct = 0;
u8 incorrect_cnt = 0;

bit relay = 0;
bit buzzer = 0;
u16 relay_tt = 0;
u16 buzzer_tt = 0;

u8 key[] = {0,1,2,3,4,5,6};
u8 key_cnt = 0;

u16 wave_tt = 0;
bit wave_flag = 0;
bit display_flag = 0;

void delay_ms(u16 ms)
{
	u16 i,j;
	for(i = 845; i > 0 ; i--)
		for(j = ms; j > 0; j--);
}

void set_port(u8 p2,u8 p0)
{
	P0 = p0;
	P2 &= 0X1F;
	P2 |= (p2); 
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

bit key_check()
{
	u8 temp1,temp2,temp3;
	temp1 = eeprom_read(0X00);
	temp2 = eeprom_read(0X01);
	temp3 = eeprom_read(0X02);
	if((key[1] * 10 + key[2]) == temp1 && (key[3] * 10 + key[4]) == temp2 && (key[5] * 10 + key[6]) == temp3)
		return 1;
	else 
		return 0;
}
void time_check(void)
{
	if(TIME[2] >= 0X07 && TIME[2] < 0X22 || TIME[2] == 0X22 && TIME[1] == 0X00 && TIME[0] == 0X00)
	{
		zidong = 1;
	}
	else
	{
		zidong = 0;
	}
}

void display(void)
{
	if(zidong && display_mode == 0)
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
	else
	{
		if(display_mode == 0)
		{
			disbuf[1] = 10;
			disbuf[2] = 10;	
			if(key_cnt >= 1)
				disbuf[3] = key[1];
			else
				disbuf[3] = 11;
			if(key_cnt >= 2)
				disbuf[4] = key[2];
			else
				disbuf[4] = 11;
			if(key_cnt >= 3)
				disbuf[5] = key[3];
			else
				disbuf[5] = 11;
			if(key_cnt >= 4)
				disbuf[6] = key[4];
			else
				disbuf[6] = 11;
			if(key_cnt >= 5)
				disbuf[7] = key[5];
			else
				disbuf[7] = 11;
			if(key_cnt >= 6)
				disbuf[8] = key[6];
			else
				disbuf[8] = 11;
		}
		if(display_mode == 13)
		{
			if(!correct)
			{
				disbuf[1] = 11;
				disbuf[2] = 10;
			}	
			else
			{
				disbuf[1] = 10;
				disbuf[2] = 11;
			}
			if(key_cnt >= 1)
				disbuf[3] = key[1];
			else
				disbuf[3] = 11;
			if(key_cnt >= 2)
				disbuf[4] = key[2];
			else
				disbuf[4] = 11;
			if(key_cnt >= 3)
				disbuf[5] = key[3];
			else
				disbuf[5] = 11;
			if(key_cnt >= 4)
				disbuf[6] = key[4];
			else
				disbuf[6] = 11;
			if(key_cnt >= 5)
				disbuf[7] = key[5];
			else
				disbuf[7] = 11;
			if(key_cnt >= 6)
				disbuf[8] = key[6];
			else
				disbuf[8] = 11;
		}
	}
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

void KBD(void)
{
	u8 key_temp;
	P3 = 0X0F; P44 = 0; P42 = 0;
	key_temp = get();
	P3 = 0XF0; P44 = 1; P42 = 1;
	key_temp |= get();
	if(key_temp != 0xff && key_buf == 0)
	{
		delay_ms(5);
		P3 = 0X0F; P44 = 0; P42 = 0;
		key_temp = get();
		P3 = 0Xf0; P44 = 1; P42 = 1;
		key_temp |= get();
		if(key_temp != 0xff && key_buf == 0)
		{
			key_buf = key_temp;
		}
	}
	else if(key_temp == 0xff && key_buf)
	{
		delay_ms(5);
		P3 = 0X0F; P44 = 0; P42 = 0;
		key_temp = get();
		P3 = 0Xf0; P44 = 1; P42 = 1;
		key_temp |= get();
		if(key_temp == 0xff && key_buf)
		{
			switch(key_buf)
			{
				case 0x7e: if(key_cnt < 6) key[++key_cnt] = 0; break; // S7
				case 0x7d: if(key_cnt < 6) key[++key_cnt] = 4; break; // S6
				case 0x7b: if(key_cnt < 6) key[++key_cnt] = 8; break; // S5
				//case 0x77: disbuf[1] = 0; disbuf[2] = 4; disbuf[3] = 10; break; // S4
				
				case 0xbe: if(key_cnt < 6) key[++key_cnt] = 1; break; // S11
				case 0xbd: if(key_cnt < 6) key[++key_cnt] = 5; break; // S10
				case 0xbb: if(key_cnt < 6) key[++key_cnt] = 9; break; // S9
				//case 0xb7: disbuf[1] = 0; disbuf[2] = 8; disbuf[3] = 10; break; // S8
				
				case 0xde: if(key_cnt < 6) key[++key_cnt] = 2; break; // S15
				case 0xdd: if(key_cnt < 6) key[++key_cnt] = 6; break; // S14
				case 0xdb: display_mode = 13; key_cnt = 0;break; // 设置
				case 0xd7: 
						if(key_cnt == 6)
						{
							key_cnt = 0;
							if(key_check() && display_mode == 0)
							{
								relay = 1;
								relay_tt = 0;
								incorrect_cnt = 0;
							}
							else if(!key_check() && display_mode == 0)
							{
								relay = 0;
								if(++incorrect_cnt == 3)
								{
									incorrect_cnt = 0;
									buzzer = 1;
								}
							}
							else if(correct && display_mode == 13)
							{
								correct = 0;
								eeprom_write(0X00,key[1] * 10 + key[2]);
								delay_ms(5);
								eeprom_write(0X01,key[3] * 10 + key[4]);
								delay_ms(5);
								eeprom_write(0X02,key[5] * 10 + key[6]);
								delay_ms(5);
								display_mode = 0;
							}
							else if(key_check() && display_mode == 13)
							{
								correct = 1;
							}
							else if(!key_check() && display_mode == 13)
							{
								if(++incorrect_cnt == 3)
								{
									buzzer = 1;
									display_mode = 0;
									incorrect_cnt = 0;
								}
								correct = 0;
							}
						}
				break; // 确认
				
				case 0xee: if(key_cnt < 6) key[++key_cnt] = 3; break; // S19
				case 0xed: if(key_cnt < 6) key[++key_cnt] = 7; break; // S18
				case 0xeb: key_cnt = 0; break; // 复位
				case 0xe7: display_mode = 0; break; // 退出
			}
			key_buf = 0;
		}
	}
}

void timer1(void) interrupt 3
{
	u8 p0 = 0;
	if(++smg_tt >= 20)
	{
		smg_tt = 0;
		set_port(0xc0,0x01 << smg_cnt);
		set_port(0xe0,TAB[disbuf[smg_cnt + 1]]);
		if(++smg_cnt >= 8) smg_cnt = 0;
	}
	
	if(++wave_tt >= 2000)
	{
		wave_tt = 0;
		wave_flag = 1;
	}
	
	if(++relay_tt >= 50000)
	{
		relay_tt = 0;
		relay = 0;
	}
	
	if(++buzzer_tt >= 30000)
	{
		buzzer_tt = 0;
		buzzer = 0;
	}
	
	if(relay)
		p0 |= 0x10;
	if(buzzer)
		p0 |= 0x40;
	
	set_port(0xa0,p0);
}

void Timer0Init(void)		//12微秒@12.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xF4;		//设置定时初值
	TH0 = 0xFF;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 0;		//定时器0停止
}

u8 wave_rec(void)
{
	u8 distance , num = 10;
	TL0 = 0xF4;		//设置定时初值
	TH0 = 0xFF;		//设置定时初值
	TX = 0;
	TR0 = 1;
	while(num--)
	{
		while(!TF0);
		TX ^= 1;
		TF0 = 0;
	}
	TR0 = 0;
	TL0 = 0;		//设置定时初值
	TH0 = 0;		//设置定时初值
	TR0 = 1;
	while(RX && !TF0);
	TR0 = 0;
	if(TF0)
	{
		TF0 = 0;
		distance = 255;
	}
	else
	{
		distance = ((TH0 << 8) | TL0) * 0.017;
	}
	return distance;
}

void main(void)
{
	ET1 = 1;
	EA = 1;
	Timer1Init();
	Timer0Init();
	allinit();
	ds1302_init();
	
	eeprom_write(0X00,65);
	delay_ms(0);
	eeprom_write(0X01,43);
	delay_ms(0);
	eeprom_write(0X02,21);
	delay_ms(0);
	
	while(1)
	{
		if(zidong && wave_flag)
		{
			wave_flag = 0;
			if(wave_rec() < 30)
			{
				relay_tt = 0;
				relay = 1;
			}
		}
		ds1302_get();
		time_check();
		display();
		KBD();
	}
}