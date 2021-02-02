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
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x40;		//��ʱ��1ʱ��ΪFosc,��1T
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//�趨��ʱ��1Ϊ16λ�Զ���װ��ʽ
	TL1 = 0xC7;		//�趨��ʱ��ֵ
	TH1 = 0xFE;		//�趨��ʱ��ֵ
	ET1 = 0;		//��ֹ��ʱ��1�ж�
	TR1 = 1;		//������ʱ��1
}

void allinit(void)
{
	set_port(0x80,0xff);
	set_port(0xa0,0x00);
	set_port(0xc0,0xff);
	set_port(0xe0,0xff);
}
/*----------------------------
���ʹ�������
----------------------------*/
void SendData(u8 dat)
{
    while (busy);               //�ȴ�ǰ������ݷ������
    busy = 1;
    SBUF = dat;                 //д���ݵ�UART���ݼĴ���
}
void SendString(u8 *s)
{
    while (*s)                  //����ַ���������־
    {
        SendData(*s++);         //���͵�ǰ�ַ�
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
				// �ر�ע�⣺��ʹ�ô��ڵ�ʱ�򣬻�ʹ�õ�P30,P31�����԰�������ʹ��S7,S6
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
        TI = 0;                 //���TIλ
        busy = 0;               //��æ��־
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