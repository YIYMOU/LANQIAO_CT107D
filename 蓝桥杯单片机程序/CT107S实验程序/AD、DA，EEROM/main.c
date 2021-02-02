#include <stc15f2k60s2.h>
#include "iic.h"

#define u8 unsigned char
#define u16 unsigned int

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff};
u8 disbuf[] = {0,11,11,11,11,10,6,7,8};

u8 key_buf = 0;
u8 smg_cnt = 0;

u8 ch = 1; 

extern u8 TIME[];

float temperature;

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
	P2 |= p2;	// ����Ӱ������λ��������ܻ��������Э�����Ӱ��
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

void BTN(void)
{
	u8 key_temp ;
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
			switch(key_buf)	// ����Ҫ�ر�ע����key_buf����Ҫ����д����key_temp
			{
//				case 0x0e: disbuf[1] = 7; disbuf[2] = 10; break;
//				case 0x0d: disbuf[1] = 6; disbuf[2] = 10; break;
				case 0x0b: ch = 1; break;
				case 0x07: ch = 3; break;
			}
			key_buf = 0;
		}
	}
}

void Timer2Init(void)		//1����@12.000MHz
{
	AUXR |= 0x04;		//��ʱ��ʱ��1Tģʽ
	T2L = 0x20;		//���ö�ʱ��ֵ
	T2H = 0xD1;		//���ö�ʱ��ֵ
	AUXR |= 0x10;		//��ʱ��2��ʼ��ʱ
}

void timer2(void) interrupt 12
{
	set_port(0xc0, 0x01 << smg_cnt);
	set_port(0xe0, TAB[disbuf[smg_cnt + 1]]);
	if(++smg_cnt >= 8) smg_cnt = 0;
}

void main()
{
	u8 temp;
	allinit();
	Timer2Init();
	IE2 |= 0X04;	// �򿪶�ʱ��2���жϣ�����
	EA = 1;
	
	write_at24c02(0x00,234);	// ��EEROM��ַΪ0x00��λ��д��234
	Delay5ms();
	write_pcf8591(153);	// DA���3V
	Delay5ms();
	temp = read_at24c02(0x00);	// ��EEROM��ַΪ0x00�����ݶ�����
	disbuf[1] = temp / 100;
	disbuf[2] = temp / 10 % 10;
	disbuf[3] = temp % 10;
	disbuf[4] = 11;
	disbuf[5] = 11;
	while(1)
	{
		
		temp = read_pcf8591(ch);
		temp = read_pcf8591(ch);	// �����PCF8591��ʱ�򣬶�����
		disbuf[6] = temp / 100 % 10;
		disbuf[7] = temp / 10 % 10;
		disbuf[8] = temp % 10;
		
		BTN();
	}
}