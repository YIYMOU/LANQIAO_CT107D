#include "stc15f2k60s2.h"
#include "iic.h"
#define u8 unsigned char
#define u16 unsigned int

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff};
u8 DISBUF[] = {0,11,11,11,11,11,11,11,11};
bit run = 1;
u16 seting_tt = 0;
u8 run_interval = 4;
u8 seting = 0;
u8 key_buf = 0;
u8 pwm_tt = 0;
u8 pwm_level = 20;
u16 led_speed = 4000;
u8 led_mode = 1;
u8 led_cnt = 0;
u8 led_buf[4][8]={ 
		0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,
		0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,
		0x81,0x42,0x24,0x18,0x81,0x42,0x24,0x18,
		0x18,0x24,0x42,0x81,0x18,0x24,0x42,0x81
};
u16 led_tt = 0;

void delay_ms(u16 ms)
{
	u16 i,j;
	for( i = 845; i > 0; i--)
		for( j = ms; j > 0; j--);
}

void set_port(u8 p2,u8 p0)
{
	P0 = p0;
	P2 &= 0X1F;
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

void BTN(void)
{
	static u8 led_mode_temp;
	u8 key_temp = P3 & 0X0F;
	if(key_temp != 0x0f && !key_buf)
	{
		delay_ms(5);
		key_temp = P3 & 0X0F;
		if(key_temp != 0x0f)
		{
			key_buf = key_temp;
			if(seting == 0 && key_buf == 0x07)
			{
				DISBUF[1] = 11;
				DISBUF[2] = 11;
				DISBUF[3] = 11;
				DISBUF[4] = 11;
				DISBUF[5] = 11;
				DISBUF[6] = 11;
				DISBUF[7] = 10;
				if(pwm_level == 90) DISBUF[8] = 4;
				else if(pwm_level == 60) DISBUF[8] = 3;
				else if(pwm_level == 30) DISBUF[8] = 2;
				else if(pwm_level == 10) DISBUF[8] = 1;
			}
		}
	}
	else if(key_temp == 0x0f && key_buf)
	{
		delay_ms(5);
		key_temp = P3 & 0X0F;
		if(key_temp == 0x0f)
		{
			if(seting == 0 && key_buf == 0x07)
			{
				DISBUF[1] = 11;
				DISBUF[2] = 11;
				DISBUF[3] = 11;
				DISBUF[4] = 11;
				DISBUF[5] = 11;
				DISBUF[6] = 11;
				DISBUF[7] = 11;
				DISBUF[8] = 11;
			}
			else
			{
				switch(key_buf)
				{
					case 0x0e: run = !run; break;
					case 0x0d: 
						if(seting == 0)
						{
							seting = 1;
							led_mode_temp = 1;
							run_interval = 4;
							DISBUF[1] = 10;
							DISBUF[2] = led_mode;
							DISBUF[3] = 10;
							DISBUF[4] = 11;
							DISBUF[5] = 11;
							DISBUF[6] = run_interval;
							DISBUF[7] = 0;
							DISBUF[8] = 0;
						}
						else if(seting == 1)
						{
							seting = 2;
						}
						else if(seting == 2)
						{
							seting = 0;
							led_mode = led_mode_temp;
							led_speed = run_interval * 1000;
							if(led_mode == 1) WRITE_EEPROM(0x01,run_interval);
							else if(led_mode == 2) WRITE_EEPROM(0x02,run_interval);
							else if(led_mode == 3) WRITE_EEPROM(0x03,run_interval);
							else if(led_mode == 4) WRITE_EEPROM(0x04,run_interval);
							delay_ms(5);
						}
					break;
					case 0x0b: 
						if(seting == 1)
						{
							if(led_mode_temp != 4) led_mode_temp++;
							DISBUF[2] = led_mode_temp;
						}
						else if(seting == 2)
						{
							if(run_interval != 12) run_interval++;
							if(run_interval >= 10)
							{
								DISBUF[5] = run_interval / 10;
								DISBUF[6] = run_interval % 10;
								
							}
							else
							{
								DISBUF[6] = run_interval;	
							}							
						}
					break;
					case 0x07:
						if(seting == 1)
						{
							if(led_mode_temp != 1) led_mode_temp--;
							DISBUF[2] = led_mode_temp;
						}
						else if(seting == 2)
						{
							if(run_interval != 4) run_interval--;
							if(run_interval >= 10)
							{
								DISBUF[5] = run_interval / 10;
								DISBUF[6] = run_interval % 10;
							}
							else
							{
								DISBUF[6] = run_interval;	
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
	static u8 i = 1,dis_cnt = 0;
	static bit flag = 0;
	if(++dis_cnt >= 20)
	{
		dis_cnt = 0;
		if(++seting_tt == 400)
		{
			seting_tt = 0;
			flag = !flag;
		}
		if(seting == 1 && flag)
		{
			if(i==1 || i == 2 || i == 3)
			{
				set_port(0xc0,0x01 << (i-1));
				set_port(0xe0,TAB[11]);
			}
			else
			{
				set_port(0xc0,0x01 << (i-1));
				set_port(0xe0,TAB[DISBUF[i]]);
			}
		}
		else if(seting == 2 && flag)
		{
			if(i == 5 || i==6 || i == 7 || i == 8)
			{
				set_port(0xc0,0x01 << (i-1));
				set_port(0xe0,TAB[11]);
			}
			else
			{
				set_port(0xc0,0x01 << (i-1));
				set_port(0xe0,TAB[DISBUF[i]]);
			}
		}
		else
		{
			set_port(0xc0,0x01 << (i-1));
			set_port(0xe0,TAB[DISBUF[i]]);
		}
		if(++i == 9) i = 1;
	}
	if(++led_tt >= led_speed)
	{
		if(run)
		{
			led_tt = 0 ;
			if(++led_cnt == 8) led_cnt = 0;
		}
	}
	if(++pwm_tt >= 100)
	{
		pwm_tt = 0;
	}
	else if(pwm_tt >= pwm_level)
	{
		set_port(0x80,0xff);
	}
	else
	{
		set_port(0x80,~led_buf[led_mode-1][led_cnt]);
	}
}

void main(void)
{
	u8 temp ;
	Timer0Init();
	allinit();
	ET0 = 1;
	EA = 1;
	WRITE_EEPROM(0x01,123);
	delay_ms(5);
	temp = READ_EEPROM(0x01);
	while(1)
	{
		temp = READ_PCF8591(0x03);
		if(temp > 192)
		{
			pwm_level = 90;
		}
		else if(temp > 128)
		{
			pwm_level = 60;
		}
		else if(temp > 64)
		{
			pwm_level = 30;
		}
		else
		{
			pwm_level = 10;
		}
		BTN();
	}
}