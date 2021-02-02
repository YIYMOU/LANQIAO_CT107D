#include <stc15f2k60s2.h>
#include <DS1302.h>
#include <iic.h>

#define u8 unsigned char
#define u16 unsigned int

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0XBF,0Xff,0x00};
u8 disbuf[] = {12,12,12,12,12,12,12,12,12};

u8 smg_cnt = 0;

u8 key_buf = 0;

u8 wave_tt = 0;
bit wave_flag = 0;
bit wave_open = 0;

sbit TX = P1^0;
sbit RX = P1^1;
u8 distance;

u8 time_set = 0;
u8 sec = 0,min = 0,hour = 0;

u16 flicker_tt = 0;
bit flicker_open = 0;

u8 alarm_distance = 30;
bit alarm_set = 0;

bit relay = 0;
bit led_open = 0;
u16 led_tt = 0;
bit led_flag = 0;

bit on_flag = 0;

extern u8 TIME[];

void delay_ms(u16 ms)
{
	u16 i,j;
	for(i = 845; i > 0; i--)
		for(j = ms; j > 0; j--);
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

void BTN(void)
{
	u8 key_tmep;
	key_tmep = P3 & 0X0F;
	if(key_tmep != 0x0f && !key_buf)
	{
		delay_ms(5);
		key_tmep = P3 & 0X0F;
		if(key_tmep != 0x0f && !key_buf)
		{
			key_buf = key_tmep;
		}
	}
	else if(key_tmep == 0x0f && key_buf)
	{
		delay_ms(5);
		key_tmep = P3 & 0X0F;
		if(key_tmep == 0x0f && key_buf)
		{
			switch(key_buf)
			{
				case 0x0e: wave_open = !wave_open; break;	// s7
				case 0x0d: 
						if(wave_open == 0)
						{
							if(time_set == 0)
							{
								sec = (TIME[0] / 16) * 10 + (TIME[0] % 16);
								min = (TIME[1] / 16) * 10 + (TIME[1] % 16);
								hour = (TIME[2] / 16) * 10 + (TIME[2] % 16);
								time_set = 1;
							}	
							else if(time_set == 1)
							{
								time_set = 2;
							}
							else if(time_set == 2)
							{
								time_set = 3;
							}
							else if(time_set == 3)
							{
								time_set = 0;
								TIME[0] = ((sec / 10) << 4) | (sec % 10);
								TIME[1] = ((min / 10) << 4) | (min % 10);
								TIME[2] = ((hour / 10) << 4) | (hour % 10);
								ds1302_init();
							}
						}
						else 
						{
							alarm_set = !alarm_set;
							if(alarm_set == 0)
							{
								write_at24c02(0x00,alarm_distance);
								delay_ms(2);
							}
						}
				break;	// s6
				case 0x0b: 
						if(time_set == 1)
						{
							if(hour < 23)
								hour++;
						}
						else if(time_set == 2)
						{
							if(min < 59)
								min++;
						}
						else if(time_set == 3)
						{
							if(sec < 59)
								sec++;
						}
						else if(alarm_set)
						{
							if(alarm_distance < 255)
								alarm_distance++;
						}
				break;	// s5
				case 0x07: 
						if(time_set == 1)
						{
							if(hour > 0)
								hour--;
						}
						else if(time_set == 2)
						{
							if(min > 0)
								min--;
						}
						else if(time_set == 3)
						{
							if(sec > 0)
								sec--;
						}
						else if(alarm_set)
						{
							if(alarm_distance > 0)
								alarm_distance--;
						}
				break;	// s4
			}
			key_buf = 0;
		}
	}
}

void display_mode(void)
{
	if(wave_open == 0)
	{
		if(time_set)
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
		else
		{
			read_ds1302();
			disbuf[1] = TIME[2] / 16;
			disbuf[2] = TIME[2] % 16;
			disbuf[3] = 10;
			disbuf[4] = TIME[1] / 16;
			disbuf[5] = TIME[1] % 16;		
			disbuf[6] = 10;
			disbuf[7] = TIME[0] / 16;
			disbuf[8] = TIME[0] % 16;
		}			
	}
	else if(wave_open == 1)
	{	
		if(alarm_set == 0)
		{
			disbuf[1] = 11;
			disbuf[2] = 11;
			disbuf[3] = 11;
			disbuf[4] = 11;
			disbuf[5] = 11;	
			disbuf[6] = distance / 100 % 10;
			disbuf[7] = distance / 10 % 10;
			disbuf[8] = distance % 10;	
		}
		else
		{
			disbuf[1] = 11;
			disbuf[2] = 11;
			disbuf[3] = 11;
			disbuf[4] = 10;
			disbuf[5] = 10;	
			disbuf[6] = alarm_distance / 100 % 10;
			disbuf[7] = alarm_distance / 10 % 10;
			disbuf[8] = alarm_distance % 10;
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
	if(time_set == 0)
	{
		set_port(0xc0,0x01 << smg_cnt);
		set_port(0xe0,TAB[disbuf[smg_cnt + 1]]);
	}
	else
	{
		if(++flicker_tt >= 500)
		{
			flicker_tt = 0;
			flicker_open = !flicker_open;
		}
		if(time_set == 1 && (smg_cnt == 0 || smg_cnt == 1))
		{
			if(flicker_open)
			{	
				set_port(0xc0,0x01 << smg_cnt);
				set_port(0xe0,TAB[disbuf[smg_cnt + 1]]);			
			}
			else
			{
				set_port(0xc0,0x01 << smg_cnt);
				set_port(0xe0,TAB[11]);	
			}
		}
		else if(time_set == 2 && (smg_cnt == 3 || smg_cnt == 4))
		{
			if(flicker_open)
			{	
				set_port(0xc0,0x01 << smg_cnt);
				set_port(0xe0,TAB[disbuf[smg_cnt + 1]]);			
			}
			else
			{
				set_port(0xc0,0x01 << smg_cnt);
				set_port(0xe0,TAB[11]);	
			}
		}
		else if(time_set == 3 && (smg_cnt == 6 || smg_cnt == 7))
		{
			if(flicker_open)
			{	
				set_port(0xc0,0x01 << smg_cnt);
				set_port(0xe0,TAB[disbuf[smg_cnt + 1]]);			
			}
			else
			{
				set_port(0xc0,0x01 << smg_cnt);
				set_port(0xe0,TAB[11]);	
			}
		}
		else
		{
			set_port(0xc0,0x01 << smg_cnt);
			set_port(0xe0,TAB[disbuf[smg_cnt + 1]]);
		}
	}
	if(++smg_cnt >= 8) smg_cnt = 0;
	
	if(wave_open && ++wave_tt >= 200)
	{
		wave_tt = 0;
		wave_flag = 1;
	}
	if(on_flag)
	{
		if(wave_open && distance < alarm_distance)
		set_port(0xa0,0x10);
		else
			set_port(0xa0,0x00);
		
		if(wave_open && distance < alarm_distance * 1.2)
		{
			if(++led_tt >= 300)
			{
				led_tt = 0;
				if(led_flag)
				{
					led_flag = 0;
					set_port(0x80,0xfe);
				}
				else
				{
					led_flag = 1;
					set_port(0x80,0xff);
				}
			}
		}
		else
			set_port(0x80,0xff);
	}
}

void Timer0Init(void)		//12微秒@12.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xF4;		//设置定时初值
	TH0 = 0xFF;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 0;		//定时器0关闭
}

unsigned char wave_rec(void)
{
	u8 dis,num = 10;
	TL0 = 0xF4;		//设置定时初值
	TH0 = 0xFF;		//设置定时初值
	TX = 0;
	TR0 = 1;		//定时器0打开
	while(num--)
	{
		while(!TF0);
		TX ^= 1;
		TF0 = 0;
	}
	TR0 = 0;		//定时器0关闭
	TL0 = 0;		//设置定时初值
	TH0 = 0;		//设置定时初值
	TR0 = 1;		//定时器0打开
	while(!TF0 && RX);
	TR0 = 0;		//定时器0关闭
	if(TF0)
	{
		TF0 = 0;
		dis = 255;
	}
	else
	{
		dis = ((TH0 << 8) | TL0) * 0.017;
	}
	return dis;
}


void main(void)
{
	set_port(0x80,0x00);
	set_port(0xa0,0x40);
	IE2 |= 0x04;                    //开定时器2中断
    EA = 1;
	Timer2Init();
	Timer0Init();
	delay_ms(1000);
	on_flag = 1;
	allinit();
	ds1302_init();
	read_at24c02(0x00);
	while(1)
	{
		BTN();
		if(wave_open && wave_flag)
		{
			wave_flag = 0;
			distance = wave_rec();
		}
		display_mode();
	}
}


