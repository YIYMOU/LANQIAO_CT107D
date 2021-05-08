#include "STC15F2K60S2.h"
#include "onewire.h"
#include "iic.h"

#define u8 unsigned char
#define u16 unsigned int
	
#define					TEMP_INTERFACE						0
#define					DAC_INTERFACE							1
#define					SETTING_INTERFACE					2
	
#define get() (P3 & 0X3F) | ((P4 & 0X10) << 3) | ((P4 & 0X04) << 4)
	
u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff,0xc6,0x8c,0x88};
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};

u8 smg_cnt = 0;

u8 key_buf = 0;

u16 sec_tick = 0;
u8 sec_cnt = 0;

u8 interface = 0;

u8 temperature_para = 25;
u8 temperature_para_temp = 25;

bit mode = 0;

float volt = 3.25;

float temperature = 0.0;

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
	P2 = (P2 & 0x1F) | p2;	// ʹ��74HC573������0x1F��Ϊ�˲�Ӱ�������Ķ˿�
	P0 = p0;
	P2 &= 0x1F;							// ����������
}

void allinit(void)
{
	set_port(0x80,0xFF);	// �ر�����LED
	set_port(0xA0,0x00);	// �رռ̵�����������������
	set_port(0xC0,0x00);	// �ر������λѡ
	set_port(0xE0,0x00);	// �ر�����ܶ�ѡ
}

void display(void)
{
	if(interface == TEMP_INTERFACE)
	{
		disbuf[1] = 12;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = (u16)(temperature * 100) / 1000;
		disbuf[6] = (u16)(temperature * 100) / 100 % 10;
		disbuf[7] = (u16)(temperature * 100) / 10 % 10;
		disbuf[8] = (u16)(temperature * 100) / 1000 % 10;
	}
	else if(interface == SETTING_INTERFACE)
	{
		disbuf[1] = 13;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = 11;
		disbuf[6] = 11;
		disbuf[7] = temperature_para_temp / 10;
		disbuf[8] = temperature_para_temp % 10;
	}
	else if(interface == DAC_INTERFACE)
	{
		disbuf[1] = 14;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = 11;
		disbuf[6] = (u16)(volt * 100) / 100;
		disbuf[7] = (u16)(volt * 100) / 10 % 10;
		disbuf[8] = (u16)(volt * 100) % 10;
	}
	
}

void Timer2Init(void)		//1����@12.000MHz
{
	AUXR |= 0x04;		//��ʱ��ʱ��1Tģʽ
	T2L = 0x20;		//���ö�ʱ��ֵ
	T2H = 0xD1;		//���ö�ʱ��ֵ
	AUXR |= 0x10;		//��ʱ��2��ʼ��ʱ
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
			switch(key_buf)	// ����Ҫ�ر�ע����key_buf����Ҫ����д����key_temp
			{
//				case 0x7e: disbuf[1] = 7; disbuf[2] = 10; disbuf[3] = 10; break;
//				case 0x7d: disbuf[1] = 6; disbuf[2] = 10; disbuf[3] = 10; break;
				case 0x7b: 				// s5
					mode = !mode;
				
				break;
				case 0x77:  			// s6
					switch(interface)
					{
						case TEMP_INTERFACE: 
							temperature_para_temp = temperature_para;
							interface = SETTING_INTERFACE; 
							
						break;
						case SETTING_INTERFACE:
							temperature_para = temperature_para_temp;
							interface = DAC_INTERFACE; 
						break;
						case DAC_INTERFACE: 
							interface = TEMP_INTERFACE; 
						break;
					}
				break;
				
//				case 0xbe: disbuf[1] = 1; disbuf[2] = 1; disbuf[3] = 10; break;
//				case 0xbd: disbuf[1] = 1; disbuf[2] = 0; disbuf[3] = 10; break;
				case 0xbb:   			// s9
					temperature_para_temp++;
				
				break;
				case 0xb7:   			// s8
					temperature_para_temp--;
				break;
				
//				case 0xde: disbuf[1] = 1; disbuf[2] = 5; disbuf[3] = 10; break;
//				case 0xdd: disbuf[1] = 1; disbuf[2] = 4; disbuf[3] = 10; break;
//				case 0xdb: disbuf[1] = 1; disbuf[2] = 3; disbuf[3] = 10; break;
//				case 0xd7: disbuf[1] = 1; disbuf[2] = 2; disbuf[3] = 10; break;
//				                       
//				case 0xee: disbuf[1] = 1; disbuf[2] = 9; disbuf[3] = 10; break;
//				case 0xed: disbuf[1] = 1; disbuf[2] = 8; disbuf[3] = 10; break;
//				case 0xeb: disbuf[1] = 1; disbuf[2] = 7; disbuf[3] = 10; break;
//				case 0xe7: disbuf[1] = 1; disbuf[2] = 6; disbuf[3] = 10; break;				
			}
			key_buf = 0;
		}
	}
	
	
}

void smg_proc(void)
{
	set_port(0xc0, 0x01 << smg_cnt);
	if((interface == TEMP_INTERFACE && smg_cnt == 5) || (interface == DAC_INTERFACE && smg_cnt == 5))
		set_port(0xe0, TAB[disbuf[smg_cnt + 1]]  & 0x7F);
	else
		set_port(0xe0, TAB[disbuf[smg_cnt + 1]]);
	if(++smg_cnt >= 8) smg_cnt = 0;
	
	if(++sec_tick == 1000)
	{
		sec_tick = 0;
		(sec_cnt != 59)?(sec_cnt++):(sec_cnt = 0);
	}
}

void dac_proc(void)
{
	if(mode == 0)
	{
		if(temperature < temperature_para)
			write_pcf8591(0);
		else
			write_pcf8591(255);
	}
	else
	{
		if(temperature < 20)
			write_pcf8591(0);
		else if(temperature > 40)
			write_pcf8591(255);
		else
			write_pcf8591((6.0 / 40 * temperature - 2) / 5.0 * 255);
	}
}

void led_proc(void)
{
	u8 led_state = 0xff;
	if(mode == 0)
		led_state &= 0xFE;
	if(interface == TEMP_INTERFACE)
		led_state &= 0xFD;
	else if(interface == SETTING_INTERFACE)
		led_state &= 0xFB;
	else if(interface == DAC_INTERFACE)
		led_state &= 0xF7;
	set_port(0x80,led_state);
}

/********************* Timer2�жϺ���************************/
void timer2_int (void) interrupt 12
{
	smg_proc();
	
	led_proc();
}


void main(void)
{
	allinit();											// �ϵ��ʼ��
	Timer2Init();										// ��ʱ��2��ʼ��
	IE2 |= 0x04;                    // ����ʱ��2�ж�
	EA = 1;													// �������ж�
	
	while(1)
	{
		temperature = read_temperature();
		
		dac_proc();
		
		display();
		
		KBD();
	}
}
