#include <stc15f2k60s2.h>
#include <intrins.h>
#include "ds1302.h"
#include "onewire.h"
#include "iic.h"
#include <stdio.h>

#define u8 unsigned char
#define u16 unsigned int

#define get() (P3 & 0X3F) | ((P4 & 0X04) << 4) | ((P4 & 0X10) << 3)

sbit RX = P1^1;
sbit TX = P1^0;

u8 RX_buf[30];
bit tx_flag = 0;
u8 rx_cnt = 0;
u8 rx_tt = 0;
bit rx_open = 0;

extern u8 TIME[];

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0XBF,0XFF};
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};

u8 smg_cnt = 0;
u8 key_buf = 0;
u8 smg_tt = 0;

u16 fre = 0;
u16 fre_tt = 0;

u8 display_mode = 1;

u16 distance = 0;
u8 wave_tt = 0;
bit wave_flag = 0;

u16 flicker_tt = 0;

u16 temperature = 0;

u8 ad_out = 0;
u8 guang = 0;
u8 rb2 = 0;

bit key_down = 0;
u16 key_down_tt = 0;
bit key_long = 0;
bit key_long_state = 0;

bit read_mode = 0;

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

void Delay10us()		//@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 27;
	while (--i);
}


void display(void)
{
	if(display_mode == 1)
	{
		disbuf[1] = 11;
		disbuf[2] = 11;
		disbuf[3] = 10;
		disbuf[4] = fre / 10000 % 10;
		disbuf[5] = fre / 1000 % 10;
		disbuf[6] = fre / 100 % 10;
		disbuf[7] = fre / 10 % 10;
		disbuf[8] = fre % 10;
	}
	else if(display_mode == 2)
	{
		disbuf[1] = 11;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = 10;
		disbuf[6] = distance / 100 % 10;
		disbuf[7] = distance / 10 % 10;
		disbuf[8] = distance % 10;
	}
	else if(display_mode == 3)
	{
		disbuf[1] = TIME[0] / 16;
		disbuf[2] = TIME[0] % 10;
		disbuf[3] = 10;
		disbuf[4] = TIME[1] / 16;
		disbuf[5] = TIME[1] % 16;
		disbuf[6] = 10;
		disbuf[7] = TIME[2] / 16;
		disbuf[8] = TIME[2] % 16;
	}
	else if(display_mode == 4)
	{
		disbuf[1] = 11;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = temperature / 1000 % 10;
		disbuf[6] = temperature / 100 % 10;
		disbuf[7] = temperature / 10 % 10;
		disbuf[8] = temperature % 10;
	}
	else if(display_mode == 5)
	{
		disbuf[1] = 11;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = 10;
		if(read_mode)
		{
			disbuf[6] = rb2 / 100 % 10;
			disbuf[7] = rb2 / 10 % 10;
			disbuf[8] = rb2 % 10;
		}
		else
		{
			disbuf[6] = guang / 100 % 10;
			disbuf[7] = guang / 10 % 10;
			disbuf[8] = guang % 10;
		}
	}
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

/*----------------------------
发送串口数据
----------------------------*/
void SendData(u8 dat)
{
    SBUF = dat;
	while(!TI);
	TI = 0;
}

void SendString(u8 *s)
{
    while (*s)                  //检测字符串结束标志
    {
        SendData(*s++);         //发送当前字符
    }
}
void key_scan(void)
{
	u8 key_temp;
	P44 = 1; P42 = 1; P35 = 1; P33 = 0; P32 = 0;
	key_temp = get();
	P44 = 0; P42 = 0; P35 = 0; P33 = 1; P32 = 1;
	key_temp |= get();
	key_temp |= 0x13;
	if(key_temp != 0xff && key_buf == 0)
	{
		Delay5ms();
		P44 = 1; P42 = 1; P35 = 1; P33 = 0; P32 = 0;
		key_temp = get();
		P44 = 0; P42 = 0; P35 = 0; P33 = 1; P32 = 1;
		key_temp |= get();
		key_temp |= 0x13;
		if(key_temp != 0xff && key_buf == 0)
		{
			key_buf = key_temp;
			if(key_buf == 0xd7)
			{
				key_down = 1;
				key_down_tt = 0;
			}
		}
	}
	else if(key_temp == 0xff && key_buf != 0)
	{
		Delay5ms();
		P44 = 1; P42 = 1; P35 = 1; P33 = 0; P32 = 0;
		key_temp = get();
		P44 = 0; P42 = 0; P35 = 0; P33 = 1; P32 = 1;
		key_temp |= get();
		key_temp |= 0x13;
		if(key_temp == 0xff && key_buf != 0)
		{
			switch(key_buf)
			{
				case 0x7b:	// s5
					display_mode = 1;
					TMOD |= 0x04;		//设置定时器模式
				break;
				case 0x77:	// s4
					display_mode = 2;
					wave_flag = 1;
				break;
				case 0xbb: // S9
					display_mode = 3;
				break;
				case 0xb7:	// s8
					display_mode = 4;
				break;
				case 0xdb:	// s13
					display_mode = 5;
					read_mode = !read_mode;
				break;
				case 0xd7:
					if(key_long_state == 1)
					{
						key_long_state = 0;
					}
					else if(read_mode && display_mode == 5)
					{
						write_pcf8591(rb2);
						Delay5ms();
						SendString("ok!!!\r\n");
					}
					key_long = 0;
					key_down = 0;
					key_down_tt = 0;
				break;
				
			}
			key_buf = 0;
		}
	}
	
	if(key_long)
	{
		sprintf(RX_buf,"%02x:%2x:%2x\r\n",(u16)TIME[0],(u16)TIME[1],(u16)TIME[2]);
		SendString(RX_buf);
		sprintf(RX_buf,"wd:%u,rb2:%u,guang:%u\r\n",(u16)temperature,(u16)rb2,(u16)guang);
		SendString(RX_buf);
		key_long = 0;
		key_long_state = 1;
	}
}

void Timer2Init(void)		//100微秒@12.000MHz
{
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = 0x50;		//设置定时初值
	T2H = 0xFB;		//设置定时初值
	AUXR |= 0x10;		//定时器2开始计时
}

void Timer0Init(void)		//100微秒@12.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0x04;		//设置定时器模式
	TL0 = 0;		//设置定时初值
	TH0 = 0;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 0;		//定时器0开始计时
}

void tiemr2(void) interrupt 12
{
	bit smg_state = 0;
	if(++smg_tt == 10)
	{
		smg_tt = 0;
		set_port(0xe0,TAB[11]);
		set_port(0xc0, 0x01 << smg_cnt);
		if(++flicker_tt <= 250)
		{
			smg_state = 1;
		}
		else if(flicker_tt >= 500)
		{
			flicker_tt = 0;
			smg_state = 0;
		}
		if((smg_cnt == 2 || smg_cnt == 5) && smg_state == 0 && display_mode == 3)
		{
			set_port(0xe0,TAB[11]);
		}
		else if(display_mode == 4 && smg_cnt == 5)
		{
			set_port(0xe0, TAB[disbuf[smg_cnt + 1]] & 0x7f);	
		}
		else 
		{
			set_port(0xe0, TAB[disbuf[smg_cnt + 1]]);	
		}
		if(++smg_cnt >= 8) smg_cnt = 0;
		
		if(display_mode == 1 && ++fre_tt >= 1000)
		{
			TR0 = 0;
			fre_tt = 0;
			fre = ((TH0 << 8) + TL0);
			TH0 = 0;
			TL0 = 0;
			TR0 = 1;
		}
		
		if(display_mode == 2 && ++wave_tt >= 250)
		{
			wave_tt = 0;
			wave_flag = 1;
		}
		
		if(rx_open && ++rx_tt >= 80)
		{
			rx_tt = 0;
			rx_open = 0;
			RX_buf[rx_cnt++] = '\r';
			RX_buf[rx_cnt++] = '\n';
			RX_buf[rx_cnt++] = '\0';
			rx_cnt = 0;
			tx_flag = 1;
		}
		
		if(key_down && ++key_down_tt >= 500)
		{
			key_down_tt = 0;
			key_long = 1;
		}
	}
}

void UartInit(void)		//1200bps@12.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设定定时器1为16位自动重装方式
	TL1 = 0x3C;		//设定定时初值
	TH1 = 0xF6;		//设定定时初值
	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1
}
/*----------------------------
UART 中断服务程序
-----------------------------*/
void Uart() interrupt 4
{
    if (RI)
    {
        RI = 0;                 //清除RI位
		if(rx_cnt == 0)
		{
			rx_open = 1;
			rx_tt = 0;
		}
        RX_buf[rx_cnt++] = SBUF;
    }
}


void wave_rev(void)
{
	u8 num = 20;
	TMOD &= 0xF0;		//设置定时器模式
	TR0 = 0;		//定时器0停止
	while(num--)
	{
		TX = 0;
		Delay10us();
		TX = 1;
		Delay10us();
	}
	TH0 = 0;
	TL0 = 0;
	TR0 = 1;
	while(RX && !TF0);
	TR0 = 0;
	if(TF0)
	{
		TF0 = 0;
		distance = 999;
	}
	else 
	{
		distance = ((TH0 << 8) | (TL0)) * 0.017;
	}
}

void main(void)
{
	u8 temp = 0;
	allinit();
	Timer2Init();
	Timer0Init();
	IE2 |= 0X04;
	UartInit();
	ES = 1;
	EA = 1;
	ds1302_init();
	write_eeprom(0x00,234);
	Delay5ms();
	temp = read_eeprom(0x00);
	disbuf[1] = temp / 100;
	disbuf[2] = temp / 10 % 10;
	disbuf[3] = temp % 10;
	disbuf[4] = 10;
	while(1)
	{
		
		key_scan();
		
		if(wave_flag && display_mode == 2)
		{
			wave_flag = 0;
			wave_rev();
		}
		if(display_mode == 3)
		{
			ds1302_get();
		}
		if(display_mode == 4)
		{
			temperature = rd_temperature();
		}
		if(display_mode == 5)
		{
			if(read_mode)
			{
				rb2 = read_pcf8591(3);
			}
			else
			{
				guang = read_pcf8591(1);
			}
		}
		if(tx_flag)
		{
			tx_flag = 0;
			SendString(RX_buf);
		}
		
		display();
	}
}
