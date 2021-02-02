#include <stc15f2k60s2.h>

#define u8 unsigned char
#define u16 unsigned int

#define get() (P3 & 0X3F) | ((P4 & 0X10) << 3) | ((P4 & 0X04) << 4)

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff};
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};

u8 key_buf = 0;
u8 smg_cnt = 0;

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
	P2 = (P2 & 0X1F) | p2;	// 不能影响其他位，否则可能会对其他的协议造成影响
	P0 = p0;
	P2 &= 0X1F;
}

void allinit(void)
{
	set_port(0x80,0xff);
	set_port(0xc0,0xff);
	set_port(0xe0,0xff);
	set_port(0xa0,0x00);
}

void KBD(void)
{
	u8 key_temp ;
	P3 = 0X0F; P44 = 0; P42 = 0;
	key_temp = get();
	P3 = 0XF0; P44 = 1; P42 = 1;
	key_temp |= get();
	
	if(key_temp != 0xff && !key_buf)
	{
		Delay5ms();
		P3 = 0X0F; P44 = 0; P42 = 0;
		key_temp = get();
		P3 = 0XF0; P44 = 1; P42 = 1;
		key_temp |= get();
		if(key_temp != 0xff && !key_buf)
		{	
			key_buf = key_temp;
		}
	}
	else if(key_temp == 0xff && key_buf)
	{
		Delay5ms();
		P3 = 0X0F; P44 = 0; P42 = 0;
		key_temp = get();
		P3 = 0XF0; P44 = 1; P42 = 1;
		key_temp |= get();
		if(key_temp == 0xff && key_buf)
		{	
			switch(key_buf)	// 这里要特别注意是key_buf，不要粗心写成了key_temp
			{
				case 0x7e: disbuf[1] = 7; disbuf[2] = 10; disbuf[3] = 10; break;
				case 0x7d: disbuf[1] = 6; disbuf[2] = 10; disbuf[3] = 10; break;
				case 0x7b: disbuf[1] = 5; disbuf[2] = 10; disbuf[3] = 10; break;
				case 0x77: disbuf[1] = 4; disbuf[2] = 10; disbuf[3] = 10; break;
				
				case 0xbe: disbuf[1] = 1; disbuf[2] = 1; disbuf[3] = 10; break;
				case 0xbd: disbuf[1] = 1; disbuf[2] = 0; disbuf[3] = 10; break;
				case 0xbb: disbuf[1] = 0; disbuf[2] = 9; disbuf[3] = 10; break;
				case 0xb7: disbuf[1] = 0; disbuf[2] = 8; disbuf[3] = 10; break;
				
				case 0xde: disbuf[1] = 1; disbuf[2] = 5; disbuf[3] = 10; break;
				case 0xdd: disbuf[1] = 1; disbuf[2] = 4; disbuf[3] = 10; break;
				case 0xdb: disbuf[1] = 1; disbuf[2] = 3; disbuf[3] = 10; break;
				case 0xd7: disbuf[1] = 1; disbuf[2] = 2; disbuf[3] = 10; break;
				                       
				case 0xee: disbuf[1] = 1; disbuf[2] = 9; disbuf[3] = 10; break;
				case 0xed: disbuf[1] = 1; disbuf[2] = 8; disbuf[3] = 10; break;
				case 0xeb: disbuf[1] = 1; disbuf[2] = 7; disbuf[3] = 10; break;
				case 0xe7: disbuf[1] = 1; disbuf[2] = 6; disbuf[3] = 10; break;				
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
	set_port(0xc0, 0x01 << smg_cnt);
	set_port(0xe0, TAB[disbuf[smg_cnt + 1]]);
	if(++smg_cnt >= 8) smg_cnt = 0;
}

void main()
{
	allinit();
	Timer2Init();
	IE2 |= 0X04;	// 打开定时器2的中断，谨记
	EA = 1;
	while(1)
	{
		KBD();
	}
}