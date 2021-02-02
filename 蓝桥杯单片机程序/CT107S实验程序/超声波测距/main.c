#include <stc15f2k60s2.h>
#include <intrins.h>
#define u8 unsigned char
#define u16 unsigned int

sbit TX = P1^0;
sbit RX = P1^1;

u8 code TAB[] = {0xc0 ,0xf9 ,0xa4 ,0xb0 ,0x99 ,0x92 ,0x82 ,0xf8 ,0x80 ,0x90 , 0xbf ,0xff };
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};
u8 smg_cnt = 0;

u8 wave_tt = 0;
bit wave_flag = 0;

void Delay10us()		//@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 27;
	while (--i);
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

void Timer2Init(void)		//1毫秒@12.000MHz
{
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = 0x20;		//设置定时初值
	T2H = 0xD1;		//设置定时初值
	AUXR |= 0x10;		//定时器2开始计时
}

void timer2(void) interrupt 12
{
	set_port(0xc0,0x01 << smg_cnt);
	set_port(0xe0,TAB[disbuf[smg_cnt + 1]]);
	if(++smg_cnt >= 8) smg_cnt = 0;
	
	if(++wave_tt >= 200)
	{
		wave_tt = 0;
		wave_flag = 1;
	}
}

void Timer1Init(void)		//8微秒@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0;		//设置定时初值
	TH1 = 0;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 0;		//定时器1开始计时
}

unsigned char Wave_Recv(void)
{
	u8 distance, num = 20;
	TX = 0;
	TR1 = 0;		//定时器1开始计时
	// TX 引脚发送 40kHz 方波信号驱动超声波发送探头
	while(num--)
	{
		Delay10us();
		TX ^= 1;
	}
	TR1 = 0;
	TL1 = 0;
	TH1 = 0;
	TR1 = 1;
	while(RX && !TF1);	// 注意这里应该是&& ,只要两个条件中有一个不满足就应该跳出循环
	TR1 = 0;
	if(TF1)
	{
		TF1 = 0;
		distance = 255;
	}
	else
	{
		distance = ((TH1 << 8) + TL1) * 0.017;
		// x / 1000000 /2 * 340 * 100 cm
	}
	return distance;
}

void main(void)
{
	IE2 |= 0x04;
	EA = 1;
	allinit();
	Timer2Init();
	Timer1Init();
	while(1)
	{
		if(wave_flag)
		{
			u8 distance;
			wave_flag = 0;
			distance = Wave_Recv();
			disbuf[5] = 10;
			disbuf[6] = distance / 100 % 10;
			disbuf[7] = distance / 10 % 10;
			disbuf[8] = distance % 10;
		}
	}
}