#include "STC15F2K60S2.h"

#define u8 unsigned char
#define u16 unsigned int

u8 buffer[20];
u8 buffer_cnt = 0;

u8 *str = "987654321\r\n";

u8 key_buf = 0;

bit busy;

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

void UartInit(void)		//9600bps@12.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设定定时器1为16位自动重装方式
	TL1 = 0xC7;		//设定定时初值
	TH1 = 0xFE;		//设定定时初值
	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1
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
    while (busy);               //等待前面的数据发送完成
    busy = 1;
    SBUF = dat;                 //写数据到UART数据寄存器
}
void SendString(u8 *s)
{
    while (*s)                  //检测字符串结束标志
    {
        SendData(*s++);         //发送当前字符
    }
}

void BTN(void)
{
	u8 key_temp;
	key_temp = P3 & 0X0F;
	if(key_temp != 0x0f && !key_buf)
	{
		Delay5ms();
		key_temp = P3 & 0X0F;
		if(key_temp != 0x0f && !key_buf)
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
				// 特别注意：在使用串口的时候，会使用到P30,P31，所以按键不能使用S7,S6
				case 0x0b: SendString(buffer); break;
				case 0x07: SendString(str);break;
				
			}
			key_buf = 0;
		}
	}
}

void uartInt(void) interrupt 4
{
	u8 temp;
	if(RI)
	{
		RI = 0;
		temp = SBUF;
		if(temp == '#')
		{
			buffer[buffer_cnt++] = '\r';
			buffer[buffer_cnt++] = '\n';
			buffer[buffer_cnt] = '\0';
			buffer_cnt = 0;
		}
		else 
		{
			buffer[buffer_cnt++] = temp;
		}
	}
    if (TI)
    {
        TI = 0;                 //清除TI位
        busy = 0;               //清忙标志
    }
	
}

void main(void)
{
	allinit();
	UartInit();
	ES = 1;
	EA = 1;
	while(1)
	{
		BTN();
	}
}