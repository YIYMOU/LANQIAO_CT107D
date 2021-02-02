#include "stc15f2k60s2.h"
#include "ds1302.h"
#include "onewire.h"
#define u8 unsigned char
#define u16 unsigned int

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff};
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};
u8 alarmbuf[] = {0,0,0};
u8 timebuf[] = {0,0,0};
bit alarm_run = 0;
u16 alarm_tt = 0;
u16 alarm_cnt = 0;
u8 seting_clock = 0;
u8 seting_alarm = 0;
bit temperature_flag = 0;
extern u8 time[];
void delay_ms(u16 ms)
{
	u16 i,j;
	for(i = ms; i > 0; i--)
		for(j = 845; j > 0; j--);
}

void set_port(u8 p2, u8 p0)
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
	static u8 key_buf = 0;
	u8 key_temp = P3 & 0X0F;
	if(key_temp != 0x0f && !key_buf)
	{
		delay_ms(5);
		key_temp = P3 & 0X0F;
		if(key_temp != 0x0f)
		{
			key_buf = key_temp;
			if(!seting_clock && !seting_alarm)
			{
				if(key_buf == 0x07)
				{
					temperature_flag = 1;
				}
			}
		}
	}
	else if(key_temp == 0x0f && key_buf)
	{
		delay_ms(5);
		key_temp = P3 & 0X0F;
		if(temperature_flag)
		{
			temperature_flag = 0;
			alarm_run = 0;
			set_port(0x80,0xff);
		}
		else if(alarm_run)
		{
			alarm_run = 0;
			set_port(0x80,0xff);
		}
		else
		{
			if(key_temp == 0x0f)
			{
				switch(key_buf)
				{
					case 0x0e:
						if(seting_clock == 0 && seting_alarm == 0) seting_clock = 1;
						else if(seting_clock == 1 && seting_alarm == 0) seting_clock = 2;
						else if(seting_clock == 2 && seting_alarm == 0) seting_clock = 3;
						else if(seting_clock == 3 && seting_alarm == 0)
						{
							seting_clock = 0;
							time[2] = (timebuf[0] / 10) << 4 | (timebuf[0] % 10) ;
							time[1] = (timebuf[1] / 10) << 4 | (timebuf[1] % 10) ;
							time[0] = (timebuf[2] / 10) << 4 | (timebuf[2] % 10) ;
							ds1302_init();
						}
					break;
					case 0x0d: 
						if(seting_clock == 0 && seting_alarm == 0) 
						{
							seting_alarm = 1;
							alarmbuf[0] = (alarmbuf[0] % 16) | (alarmbuf[0] / 16 * 10) ;
							alarmbuf[1] = (alarmbuf[1] % 16) | (alarmbuf[1] / 16 * 10) ;
							alarmbuf[2] = (alarmbuf[2] % 16) | (alarmbuf[2] / 16 * 10) ;
						}
						else if(seting_clock == 0 && seting_alarm == 1) seting_alarm = 2;
						else if(seting_clock == 0 && seting_alarm == 2) seting_alarm = 3;
						else if(seting_clock == 0 && seting_alarm == 3)
						{
							seting_alarm = 0;
							alarmbuf[0] = (alarmbuf[0] / 10) << 4 | (alarmbuf[0] % 10) ;
							alarmbuf[1] = (alarmbuf[1] / 10) << 4 | (alarmbuf[1] % 10) ;
							alarmbuf[2] = (alarmbuf[2] / 10) << 4 | (alarmbuf[2] % 10) ;
						}
					break;
					case 0x0b: 
						if(seting_clock)
						{
							if(seting_clock == 1)
							{
								if(timebuf[seting_clock - 1] != 23) timebuf[seting_clock - 1]++;
							}
							else
							{
								if(timebuf[seting_clock - 1] != 59) timebuf[seting_clock - 1]++;
							}
						}
						else if(seting_alarm)
						{
							if(seting_alarm == 1)
							{
								if(alarmbuf[seting_alarm - 1] != 23) alarmbuf[seting_alarm - 1]++;
							}
							else
							{
								if(alarmbuf[seting_alarm - 1] != 59) alarmbuf[seting_alarm - 1]++;
							}
						}
					break;
					case 0x07: 
						if(seting_clock)
						{
							if(timebuf[seting_clock - 1] != 0) timebuf[seting_clock - 1]--;
						}
						else if(seting_alarm)
						{
							if(seting_alarm == 1)
							{
								if(alarmbuf[seting_alarm - 1] != 0) alarmbuf[seting_alarm - 1]--;
							}
							else
							{
								if(alarmbuf[seting_alarm - 1] != 0) alarmbuf[seting_alarm - 1]--;
							}
						}
					break;
				}
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
	static u8 i = 1,dis_tt = 0;
	static u16 discnt = 0;
	static bit alarm_flag = 1;
	static bit dis_flag = 1;
	if(++dis_tt >= 20)
	{
		dis_tt = 0;
		set_port(0xc0,0xff);
		set_port(0xe0,0xff);
		if(++discnt >= 500)
		{
			discnt = 0;
			dis_flag = !dis_flag;
		}
		if((seting_clock == 1 || seting_alarm == 1) && dis_flag)
		{
			if(i == 1 || i == 2)
			{
				set_port(0xc0,0x01 << (i - 1));
				set_port(0xe0,TAB[11]);
			}
			else
			{
				set_port(0xc0,0x01 << (i - 1));
				set_port(0xe0,TAB[disbuf[i]]);
			}
		}
		else if((seting_clock == 2 || seting_alarm == 2) && dis_flag)
		{
			if(i == 4 || i == 5)
			{
				set_port(0xc0,0x01 << (i - 1));
				set_port(0xe0,TAB[11]);
			}
			else
			{
				set_port(0xc0,0x01 << (i - 1));
				set_port(0xe0,TAB[disbuf[i]]);
			}
		}
		else if((seting_clock == 3 || seting_alarm == 3) && dis_flag)
		{
			if(i == 7 || i == 8)
			{
				set_port(0xc0,0x01 << (i - 1));
				set_port(0xe0,TAB[11]);
			}
			else
			{
				set_port(0xc0,0x01 << (i - 1));
				set_port(0xe0,TAB[disbuf[i]]);
			}
		}
		else if(temperature_flag && i == 8)
		{
			set_port(0xc0,0x01 << (i - 1));
			set_port(0xe0,0xc6);
		}
		else
		{
			set_port(0xc0,0x01 << (i - 1));
			set_port(0xe0,TAB[disbuf[i]]);
		}
		if(++i == 9) i = 1;
	}
	if(alarm_run && ++alarm_tt >= 2000)
	{
		alarm_tt = 0;
		if(alarm_flag)
		{
			set_port(0x80,0xfe);
		}
		else
		{
			set_port(0x80,0xff);
		}
		alarm_flag = ! alarm_flag;
		if(++alarm_cnt >= 25)
		{
			alarm_cnt = 0;
			alarm_run = 0;
			set_port(0x80,0xff);
		}
	}
}

void main(void)
{
	u16 temp;
	allinit();
	Timer0Init();
	ds1302_init();
	ET0 = 1;
	EA = 1;
	while(1)
	{
		BTN();
		if(seting_clock == 0 && seting_alarm == 0 && !temperature_flag)
		{
			ds1302_read();
			disbuf[1] = time[2] / 16;
			disbuf[2] = time[2] % 16;
			disbuf[3] = 10;
			disbuf[4] = time[1] / 16;
			disbuf[5] = time[1] % 16;
			disbuf[6] = 10;
			disbuf[7] = time[0] / 16;
			disbuf[8] = time[0] % 16;			
		}
		else if(seting_clock && !temperature_flag)
		{
			disbuf[1] = timebuf[0] / 10;
			disbuf[2] = timebuf[0] % 10;
			disbuf[3] = 10;
			disbuf[4] = timebuf[1] / 10;
			disbuf[5] = timebuf[1] % 10;
			disbuf[6] = 10;           
			disbuf[7] = timebuf[2] / 10;
			disbuf[8] = timebuf[2] % 10;	
		}
		else if(seting_alarm && !temperature_flag)
		{
			disbuf[1] = alarmbuf[0] / 10;
			disbuf[2] = alarmbuf[0] % 10;
			disbuf[3] = 10;
			disbuf[4] = alarmbuf[1] / 10;
			disbuf[5] = alarmbuf[1] % 10;
			disbuf[6] = 10;           
			disbuf[7] = alarmbuf[2] / 10;
			disbuf[8] = alarmbuf[2] % 10;	
		}
		else if(temperature_flag)
		{
			temp = (u16)ds18b20_read();
			disbuf[1] = 11;
			disbuf[2] = 11;
			disbuf[3] = 11;
			disbuf[4] = 11;
			disbuf[5] = 11;
			disbuf[6] = temp / 10;           
			disbuf[7] = temp % 10;
		}
		if(alarmbuf[2] == time[0] && alarmbuf[1] == time[1] && alarmbuf[0] == time[2])
		{
			alarm_run = 1;
			alarm_cnt = 0;
		}
	}
}
