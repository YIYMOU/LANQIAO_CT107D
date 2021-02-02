#include <STC15F2K60S2.H>
#include "iic.h"
#include "onewire.h"
#include <string.h>
#include <stdio.h>

#define u8 unsigned char
#define u16 unsigned int

#define get() (P3 & 0X3C) | 0XC3

sbit TX = P1^0;
sbit RX = P1^1;

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0XBF,0XFF,0xc6,0XC7,0XC8,0X8C};
// C:12,L:13,U:14,P:15
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};

u8 smg_cnt = 0;
u8 key_buf = 0;

u8 buffer[20];
u8 rx_cnt = 0;
u8 rx_tt = 0;
bit rx_flag = 0;

u16 down_tt = 0;
u16 down_tt2 = 0;

u16 change_cnt = 0;


u16 temperature = 0;
u16 temperature_tt = 0;
bit temperature_flag = 0;

bit downlong = 0;
bit downlong1 = 0;


bit downlong2 = 0;
bit downlong3 = 0;

u8 mode = 0;
u16 distance;

u8 wave_tt = 0;
u8 wave_flag = 0;

u16 argc_temperature = 3000;
u16 argc_distance = 35;
u16 argc_temperature_temp = 3000;
u16 argc_distance_temp = 35;
bit argc_flag = 0;

bit dac_enable = 1;
bit send_enable = 0;

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

void display(void)
{
	// C:12,L:13,U:14,P:15
	// 0:�¶���ʾ, 1:������ʾ��2:���������ʾ��3:������ʾ
	if(mode == 0)	// �¶���ʾ
	{
		disbuf[1] = 12;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = temperature / 1000;
		disbuf[6] = temperature / 100 % 10;
		disbuf[7] = temperature / 10 % 10;
		disbuf[8] = temperature % 10;
	}
	else if(mode == 1)	// ������ʾ
	{
		disbuf[1] = 13;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = 11;
		disbuf[6] = 11;
		disbuf[7] = distance / 10 % 10;
		disbuf[8] = distance % 10;
	}
	else if(mode == 2)	// ���������ʾ
	{
		disbuf[1] = 14;
		disbuf[2] = 11;
		disbuf[3] = 11;
		if(change_cnt / 10000)
		{
			disbuf[4] = change_cnt / 10000;
			disbuf[5] = change_cnt / 1000 % 10;
			disbuf[6] = change_cnt / 100 % 10;
			disbuf[7] = change_cnt / 10 % 10;
			disbuf[8] = change_cnt % 10;
		}
		else if(change_cnt / 1000 % 10)
		{
			disbuf[4] = 11;
			disbuf[5] = change_cnt / 1000 % 10;
			disbuf[6] = change_cnt / 100 % 10;
			disbuf[7] = change_cnt / 10 % 10;
			disbuf[8] = change_cnt % 10;
		}
		else if(change_cnt / 100 % 10)
		{
			disbuf[4] = 11;
			disbuf[5] = 11;
			disbuf[6] = change_cnt / 100 % 10;
			disbuf[7] = change_cnt / 10 % 10;
			disbuf[8] = change_cnt % 10;
		}
		else if(change_cnt / 10 % 10)
		{
			disbuf[4] = 11;
			disbuf[5] = 11;
			disbuf[6] = 11;
			disbuf[7] = change_cnt / 10 % 10;
			disbuf[8] = change_cnt % 10;
		}
		else
		{
			disbuf[4] = 11;
			disbuf[5] = 11;
			disbuf[6] = 11;
			disbuf[7] = 11;
			disbuf[8] = change_cnt;
		}
	}
	else if(mode == 3)	// ��������
	{
		disbuf[1] = 15;
		disbuf[2] = 11;
		disbuf[3] = 11;
		if(!argc_flag)
			disbuf[4] = 1;
		else 
			disbuf[4] = 2;
		disbuf[5] = 11;
		disbuf[6] = 11;
		if(!argc_flag)
		{
			disbuf[7] = argc_temperature_temp / 1000 % 10;
			disbuf[8] = argc_temperature_temp / 100 % 10;
		}
		else 
		{
			disbuf[7] = argc_distance_temp / 10 % 10;
			disbuf[8] = argc_distance_temp % 10;
		}
	}
}

void wave_rec(void)
{
	u8 num = 10;
	TX = 0;
	TL0 = 0xF4;		//���ö�ʱ��ֵ
	TH0 = 0xFF;		//���ö�ʱ��ֵ
	TR0 = 1;		//��ʱ��0��
	while(num--)
	{
		while(!TF0);
		TF0 = 0;
		TX ^= 1;
	}
	TR0 = 0;		//��ʱ��0�ر�
	TL0 = 0;		//���ö�ʱ��ֵ
	TH0 = 0;		//���ö�ʱ��ֵ
	TR0 = 1;		//��ʱ��0��
	while(RX && !TF0);
	TR0 = 0;
	if(TF0)
	{
		TF0 = 0;
		distance = 99;
	}
	else
	{
		distance = ((TH0 << 8) | TL0) * 0.017;
		if(distance > 99) distance = 99;
	}
}

/*----------------------------
���ʹ�������
----------------------------*/
void SendData(u8 dat)
{
    SBUF = dat;                 //д���ݵ�UART���ݼĴ���
	while(!TI);
	TI = 0;
}
/*----------------------------
�����ַ���
----------------------------*/
void SendString(char *s)
{
    while (*s)                  //����ַ���������־
    {
        SendData(*s++);         //���͵�ǰ�ַ�
    }
}

void KBD(void)
{
	u8 key_temp;
	P35 = 1; P34 = 1; P32 = 0; P33 = 0;
	key_temp = get();
	P35 = 0; P34 = 0; P32 = 1; P33 = 1;
	key_temp |= get();
	if(key_temp != 0xff && key_buf == 0)
	{
		Delay5ms();
		P35 = 1; P34 = 1; P32 = 0; P33 = 0;
		key_temp = get();
		P35 = 0; P34 = 0; P32 = 1; P33 = 1;
		key_temp |= get();
		if(key_temp != 0xff && key_buf == 0)
		{
			key_buf = key_temp;
		}
	}
	else if(key_temp == 0xff && key_buf)
	{
		Delay5ms();
		P35 = 1; P34 = 1; P32 = 0; P33 = 0;
		key_temp = get();
		P35 = 0; P34 = 0; P32 = 1; P33 = 1;
		key_temp |= get();
		if(key_temp == 0xff && key_buf)
		{
			// 0:�¶���ʾ, 1:������ʾ��2:���������ʾ��3:������ʾ
			switch(key_buf)
			{
				case 0xdb:	// s13
					if(!downlong1) // �̰���������
					{
						if(mode != 3)
						{
							mode = 3;
							argc_temperature_temp = argc_temperature;
							argc_distance_temp = argc_distance;
							argc_flag = 0;
						}	
						else
						{
							mode = 0;
							if(argc_temperature != argc_temperature_temp || argc_distance != argc_distance_temp)
							{
								argc_temperature = argc_temperature_temp;
								argc_distance = argc_distance_temp;
								change_cnt++;
								write_eeprom(0x00,change_cnt / 256);
								Delay5ms();
								write_eeprom(0x01,change_cnt % 256);
								Delay5ms();
							}
						}
					}
					down_tt = 0;
					downlong1 = 0;
				break;
				case 0xd7:	// s12
					if(!downlong3) // �̰������л�
					{
						if(mode == 3)
						{
							argc_flag = !argc_flag;
						}
						else if(mode != 3)
						{
							mode = (mode + 1) % 3;
						}
					}
					down_tt2 = 0;
					downlong3 = 0;
				break;
				                       
				case 0xeb:	// s17:��
					if(mode == 3 && argc_flag == 0)
					{
						if(argc_temperature_temp + 200 < 9900)
							argc_temperature_temp += 200;
					}
					else if(mode == 3 && argc_flag == 1)
					{
						if(argc_distance_temp + 5 < 99)
							argc_distance_temp += 5;
					}
				break;
				case 0xe7:	// s16:��
					if(mode == 3 && argc_flag == 0)
					{
						if(argc_temperature_temp > 0)
							argc_temperature_temp -= 200;
					}
					else if(mode == 3 && argc_flag == 1)
					{
						if(argc_distance_temp > 0)
							argc_distance_temp -= 5;
					}
				break;
			}
			key_buf = 0;
		}
	}
	if(downlong && key_buf == 0xdb)	// s13������
	{
		downlong = 0;
		downlong1 = 1;
		dac_enable = !dac_enable;
	}
	if(downlong2 && key_buf == 0xd7) // s12������
	{
		downlong2 = 0;
		downlong3 = 1;
		change_cnt = 0;
		write_eeprom(0x00,change_cnt / 256);
		Delay5ms();
		write_eeprom(0x01,change_cnt % 256);
		Delay5ms();
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
	u8 led_state = 0xff;
	
	set_port(0xe0,TAB[11]);
	// 0:�¶���ʾ, 1:������ʾ��2:���������ʾ��3:������ʾ
	set_port(0xc0,0x01 << smg_cnt);
	if(mode == 0 && smg_cnt == 5)
	{
		set_port(0xe0,TAB[disbuf[smg_cnt + 1]] & 0x7f);
	}
	else 
		set_port(0xe0,TAB[disbuf[smg_cnt + 1]]);
	
	if(++smg_cnt >= 8) smg_cnt = 0;
	
	if(rx_flag && ++rx_tt >= 50)
	{
		rx_flag = 0;
		rx_tt = 0;
		buffer[rx_cnt++] = '\0';
		rx_cnt = 0;
		
		if(strcmp(buffer,"ST\r\n") == 0)
		{
			send_enable = 1;
			sprintf(buffer,"$%d,%d.%d\r\n",distance,temperature / 100,temperature % 100);
		}
		else if(strcmp(buffer,"PARA\r\n") == 0)
		{
			send_enable = 1;
			sprintf(buffer,"$%d,%d\r\n",argc_distance,argc_temperature / 100);
		}
		else
		{
			send_enable = 1;
			sprintf(buffer,"ERROR\r\n");
		}
	}
	
	if(!downlong && !downlong1 && key_buf == 0xdb && ++down_tt >= 1000)
	{
		downlong = 1;
		down_tt = 0;
	}
	
	if(!downlong2 && !downlong3 && key_buf == 0xd7 && ++down_tt2 >= 1000)
	{
		downlong2 = 1;
		down_tt2 = 0;
	}
	
	if(++temperature_tt >= 400)
	{
		temperature_tt = 0;
		temperature_flag = 1;
	}
	
	if(++wave_tt >= 255)
	{
		wave_tt = 0;
		wave_flag = 1;
	}
	
	if(temperature > argc_temperature)
	{
		led_state &= 0xfe;
	}
	if(distance < argc_distance)
	{
		led_state &= 0xfd;
	}
	if(dac_enable)
	{
		led_state &= 0xfb;
	}
	
	set_port(0x80,led_state);
}

void Timer0Init(void)		//12΢��@12.000MHz
{
	AUXR &= 0x7F;		//��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TL0 = 0xF4;		//���ö�ʱ��ֵ
	TH0 = 0xFF;		//���ö�ʱ��ֵ
	TF0 = 0;		//���TF0��־
	TR0 = 0;		//��ʱ��0�ر�
}

void UartInit(void)		//4800bps@12.000MHz
{
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x40;		//��ʱ��1ʱ��ΪFosc,��1T
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//�趨��ʱ��1Ϊ16λ�Զ���װ��ʽ
	TL1 = 0x8F;		//�趨��ʱ��ֵ
	TH1 = 0xFD;		//�趨��ʱ��ֵ
	ET1 = 0;		//��ֹ��ʱ��1�ж�
	TR1 = 1;		//������ʱ��1
}

void uartint()interrupt 4
{
	if (RI)
    {
		RI = 0;
		rx_flag = 1;
        buffer[rx_cnt++] = SBUF;
    }
}

void main(void)
{
	allinit();
	Timer2Init();
	Timer0Init();
	UartInit();
	ES = 1;
	IE2 |= 0X04;
	EA = 1;
	if(read_eeprom(0x1f) != 123)
	{
		change_cnt = 0;
		write_eeprom(0x1f,123);
		Delay5ms();
	}
	else
	{
		change_cnt = read_eeprom(0x00);
		Delay5ms();
		change_cnt = change_cnt * 256 + read_eeprom(0x01);
		Delay5ms();
	}
	while(1)
	{
		if(temperature_flag)
		{
			temperature_flag = 0;
			temperature = (u16)(rd_temperature() * 100);
		}
		KBD();
		if(wave_flag)
		{
			wave_flag = 0;
			wave_rec();
		}
		if(send_enable)
		{
			send_enable = 0;
			SendString(buffer);
		}
		if(distance <= argc_distance)
		{
			write_pcf8591(102,dac_enable);			
		}
		else 
		{
			write_pcf8591(204,dac_enable);	
		}
		display();
	}
}
