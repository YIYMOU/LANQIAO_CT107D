#include <STC15F2K60S2.H>
#include "onewire.h"
#include "iic.h"

#define u8 unsigned char
#define u16 unsigned int

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff,0xc1,0x8e,0xc6,0X89,0x8c};
// 12---U 13---F 14---C  15---H 16---P
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};

u8 smg_cnt = 0;
u8 key_buf = 0;

u16 temperature = 0;

u8 mode = 0;

u16 fre = 0;
u16 fre_tt;
bit fre_open = 0;

u16 Vo = 0;

u16 yuzhi = 100;

u16 fre_temp;
u16 Vo_temp;
u16 temperature_temp;

u16 dowm_tt = 0;

u8 last_mode = 0;
u8 cnt_1ms = 0;

bit led8_open = 0;
u8 led8_tt = 0;

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
	// 0-温度 1-电压 2-频率 3-回显 4-设置
	if(mode == 0)	// 温度
	{
		disbuf[1] = 14;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = temperature / 1000;
		disbuf[6] = temperature / 100 % 10;
		disbuf[7] = temperature / 10 % 10;
		disbuf[8] = temperature % 10;
	}
	
	else if(mode == 1)	// 电压
	{
		disbuf[1] = 12;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = 11;
		disbuf[6] = 11;	
		disbuf[7] = Vo / 100 % 10;
		disbuf[8] = Vo / 10 % 10;	
	}
	else if(mode == 2)	// 频率
	{
		disbuf[1] = 13;
		disbuf[2] = 11;
		disbuf[3] = 11;
		if(fre / 10000 % 10)
		{
			disbuf[4] = fre / 10000 % 10;
			disbuf[5] = fre / 1000 % 10;
			disbuf[6] = fre / 100 % 10;
			disbuf[7] = fre / 10 % 10;
			disbuf[8] = fre % 10;
		}
		else if(fre / 1000 % 10)
		{
			disbuf[4] = 11;
			disbuf[5] = fre / 1000 % 10;
			disbuf[6] = fre / 100 % 10;
			disbuf[7] = fre / 10 % 10;
			disbuf[8] = fre % 10;
		}
		else if(fre / 100 % 10)
		{
			disbuf[4] = 11;
			disbuf[5] = 11;
			disbuf[6] = fre / 100 % 10;
			disbuf[7] = fre / 10 % 10;
			disbuf[8] = fre % 10;
		}
		else if(fre / 10 % 10)
		{
			disbuf[4] = 11;
			disbuf[5] = 11;
			disbuf[6] = 11;
			disbuf[7] = fre / 10 % 10;
			disbuf[8] = fre % 10;
		}
		else
		{
			disbuf[4] = 1;
			disbuf[5] = 11;
			disbuf[6] = 11;
			disbuf[7] = 11;
			disbuf[8] = fre % 10;
		}
	}
	else if(mode == 3)
	{
		if(last_mode == 0)
		{
			disbuf[1] = 15;
			disbuf[2] = 14;
			disbuf[3] = 11;
			disbuf[4] = 11;
			disbuf[5] = temperature_temp / 1000;
			disbuf[6] = temperature_temp / 100 % 10;
			disbuf[7] = temperature_temp / 10 % 10;
			disbuf[8] = temperature_temp % 10;
		}
		else if(last_mode == 1)
		{
			disbuf[1] = 15;
			disbuf[2] = 12;
			disbuf[3] = 11;
			disbuf[4] = 11;
			disbuf[5] = 11;
			disbuf[6] = 11;
			disbuf[7] = Vo_temp / 100 % 10;
			disbuf[8] = Vo_temp / 10 % 10;
		}
		else if(last_mode == 2)
		{
			disbuf[1] = 13;
			disbuf[2] = 11;
			disbuf[3] = 11;
			if(fre / 10000 % 10)
			{
				disbuf[4] = fre_temp / 10000 % 10;
				disbuf[5] = fre_temp / 1000 % 10;
				disbuf[6] = fre_temp / 100 % 10;
				disbuf[7] = fre_temp / 10 % 10;
				disbuf[8] = fre_temp % 10;
			}
			else if(fre / 1000 % 10)
			{
				disbuf[4] = 11;
				disbuf[5] = fre_temp / 1000 % 10;
				disbuf[6] = fre_temp / 100 % 10;
				disbuf[7] = fre_temp / 10 % 10;
				disbuf[8] = fre_temp % 10;
			}
			else if(fre / 100 % 10)
			{
				disbuf[4] = 11;
				disbuf[5] = 11;
				disbuf[6] = fre_temp / 100 % 10;
				disbuf[7] = fre_temp / 10 % 10;
				disbuf[8] = fre_temp % 10;
			}
			else if(fre / 10 % 10)
			{
				disbuf[4] = 11;
				disbuf[5] = 11;
				disbuf[6] = 11;
				disbuf[7] = fre_temp / 10 % 10;
				disbuf[8] = fre_temp % 10;
			}
			else
			{
				disbuf[4] = 1;
				disbuf[5] = 11;
				disbuf[6] = 11;
				disbuf[7] = 11;
				disbuf[8] = fre_temp % 10;
			}
		}
	}
	else if(mode == 4)
	{
		disbuf[1] = 16;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = 11;
		disbuf[6] = 11;
		disbuf[7] = yuzhi / 100;
		disbuf[8] = yuzhi / 10 % 10;		
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
			// 0-温度 1-电压 2-频率 3-回显 4-设置
			switch(key_buf)
			{
				case 0x0e: // 设置
					if(mode != 4)
					{
						mode = 4;
						yuzhi = 10;
					}
					else 
					{
						mode = 0;
						write_eeprom(0x06,yuzhi / 256);
						Delay5ms();
						write_eeprom(0x07,yuzhi % 256);
						Delay5ms();
					}
				break;
				case 0x0d: // 数据回显
					if(mode != 3 && mode != 4)
					{
						last_mode = mode;
						mode = 3;
						fre_temp = read_eeprom(0x00);
						Delay5ms();
						fre_temp = (fre_temp * 256) + read_eeprom(0x01);
						Delay5ms();
						temperature_temp = read_eeprom(0x02);
						Delay5ms();
						temperature_temp = (temperature_temp * 256) + read_eeprom(0x03);
						Delay5ms();
						Vo_temp = read_eeprom(0x04);
						Delay5ms();
						Vo_temp = (Vo_temp * 256) + read_eeprom(0x05);
						Delay5ms();
					}
					else if(mode == 4)
					{
						dowm_tt = 0;
						cnt_1ms = 0;
						if(yuzhi < 500)
							yuzhi += 10;						
					}
				break;
				case 0x0b:	// 存储
					if(mode != 3 && mode != 4)
					{
						write_eeprom(0x00,fre / 256);
						Delay5ms();
						write_eeprom(0x01,fre % 256);
						Delay5ms();
						write_eeprom(0x02,temperature / 256);
						Delay5ms();
						write_eeprom(0x03,temperature % 256);
						Delay5ms();
						write_eeprom(0x04,Vo / 256);
						Delay5ms();
						write_eeprom(0x05,Vo % 256);
						Delay5ms();
					}
				break;
				case 0x07: 	// 切换
					switch(mode)
					{
						case 0:
							mode = 1;
						break;
						case 1: 
							mode = 2;
						break;
						case 2: 
							mode = 0;
						case 3:
							mode = 0;
						break;
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

void timer2(void)interrupt 12
{
	u8 led_state = 0xff;
	
	set_port(0xe0,TAB[11]);
	set_port(0xc0,0x01 << smg_cnt);
	if(((mode == 3 && last_mode == 0) || mode == 0) && smg_cnt == 5)
	{
		set_port(0xe0,TAB[disbuf[smg_cnt + 1]] & 0x7f);
	}
	else if((mode == 1 || mode == 4 || (mode == 3 && last_mode == 1))&& smg_cnt == 6)
	{
		set_port(0xe0,TAB[disbuf[smg_cnt + 1]] & 0x7f);
	}
	else
	{
		set_port(0xe0,TAB[disbuf[smg_cnt + 1]]);
	}
	if(++smg_cnt >= 8) smg_cnt = 0;
	
	if(++fre_tt >= 1000)
	{
		TR0 = 0;
		fre_tt = 0;
		fre = (TH0 << 8) | TL0;
		TH0 = 0;
		TL0 = 0;
		TR0 = 1;
	}
	
	if(mode == 4 && key_buf == 0X0d)
	{			
		if(dowm_tt < 800) 
			dowm_tt++;
		else
		{
			if(yuzhi < 500 && mode == 4 && ++cnt_1ms >= 100)
			{
				cnt_1ms = 0;
				yuzhi += 10;
			}
		}
	}
	
	if(mode == 0)
		led_state &= 0xfe;
	else if(mode == 1)
		led_state &= 0xfd;
	else if(mode == 2)
		led_state &= 0xfb;
	
	if(Vo > yuzhi && (mode == 0 || mode == 2 || mode == 1) && ++led8_tt >= 200)
	{
		led8_tt = 0;
		led8_open = !led8_open;
	}
	if(Vo < yuzhi || (mode != 0 && mode != 2 && mode != 1))
	{
		led8_open = 0;
	}
	if(led8_open)
		led_state &= 0x7f;
	
	set_port(0x80,led_state);
}

void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0X04;
	TL0 = 0;		//设置定时初值
	TH0 = 0;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 0;		//定时器0关闭
}


void main(void)
{
	allinit();
	Timer2Init();
	Timer0Init();
	IE2 |= 0X04;
	EA = 1;
	
	fre_temp = read_eeprom(0x00);
	Delay5ms();
	fre_temp = (fre_temp * 256) + read_eeprom(0x01);
	Delay5ms();
	temperature_temp = read_eeprom(0x02);
	Delay5ms();
	temperature_temp = (temperature_temp * 256) + read_eeprom(0x03);
	Delay5ms();
	Vo_temp = read_eeprom(0x04);
	Delay5ms();
	Vo_temp = (Vo_temp * 256) + read_eeprom(0x05);
	Delay5ms();
	yuzhi = read_eeprom(0x06);
	Delay5ms();
	yuzhi = (yuzhi * 256) + read_eeprom(0x07);
	Delay5ms();
	
	while(1)
	{
		BTN();
		temperature = (u16)(rd_temperature() * 100);
		Vo = read_pcf8591(3) * 1.96;
		display();
	}
}