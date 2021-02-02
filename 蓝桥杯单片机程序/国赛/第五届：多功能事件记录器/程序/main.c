#include <stc15f2k60s2.h>
#include "onewire.h"
#include "ds1302.h"
#include "iic.h"
#include "intrins.h"
#include "string.h"

#define u8 unsigned char
#define u16 unsigned int

u8 str1[] = {"{20-20%}{23-50-00}{"};
u8 str2[] = {"}\r\n"};

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff,0xc6,0x89}; // 12��ʾC,13--H
u8 disbuf[] = {0,11,11,11,11,10,6,7,8};

u8 key_buf = 0;
u8 smg_cnt = 0;

bit busy;
bit uartsend_open = 0;
bit uartsend_flag = 0;
u16 uart_tt = 0;
unsigned char UartRec[20];
u8 uartrec_cnt = 0;
u8 guang;

bit uartrec_flag = 0;
bit cmd_corecct = 0;
u8 uartrec_tt = 0;

u8 display_mode = 0;	// Ĭ��0��ʾ��ʪ�ȣ�1��ʾʱ�䣬2��ʾ���һ������ͣ����ʱ��

u8 H;

u8 work_mode = 0;	// 0��ʾ�Զ����䣬1��ʾ�Զ���¼���ϵ�Ĭ���Զ�����

u16 stay_time = 0;
u16 stay_time_now = 0;
u16 stay_tt = 0;
u8 coming_flag = 0;
u8 stay_flag = 0;
u16 guang_tt = 0;
u16 guang_flag = 0;

u8 story_cnt = 0;

u8 read_cnt = 0;
u16 read_tt = 0;

bit DataIsNull = 1;

u8 datacnt = 0;

extern u8 TIME[];

float temperature;

bit flicker_flag = 0;
u16 flicker_tt = 0;

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

void display()
{
	if(display_mode == 0)
	{
		disbuf[1] = (u16)(temperature) / 10 % 10;
		disbuf[2] = (u16)(temperature) % 10;
		disbuf[3] = 12;
		disbuf[4] = 11;
		disbuf[5] = 11;
		disbuf[6] = H / 10;
		disbuf[7] = H % 10;
		disbuf[8] = 13;
	}
	else if(display_mode == 1)
	{
		disbuf[1] = TIME[2] / 16;
		disbuf[2] = TIME[2] % 16;
		disbuf[3] = 10;
		disbuf[4] = TIME[1] / 16;
		disbuf[5] = TIME[1] % 16;
		disbuf[6] = 10;
		disbuf[7] = TIME[0] / 16;
		disbuf[8] = TIME[0] % 16;
	}
	else if(display_mode == 2)
	{
		disbuf[1] = 11;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 10;
		disbuf[5] = stay_time / 1000;
		disbuf[6] = stay_time / 100 % 10;
		disbuf[7] = stay_time / 10 % 10;
		disbuf[8] = stay_time % 10;		
	}
	
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
				case 0x0b: work_mode = !work_mode; uartsend_open = 0; break;
				case 0x07: display_mode = (display_mode + 1) % 3; break;
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
	u8 led_state = 0xff;
	if(++flicker_tt >= 1000)
	{
		flicker_tt = 0;
		flicker_flag = !flicker_flag;
	}
	if(display_mode == 1 && flicker_flag == 0 && (smg_cnt == 2 || smg_cnt == 5))
	{
		set_port(0xc0, 0x01 << smg_cnt);
		set_port(0xe0, TAB[11]);
	}
	else
	{
		set_port(0xc0, 0x01 << smg_cnt);
		set_port(0xe0, TAB[disbuf[smg_cnt + 1]]);
	}
	
	if(++smg_cnt >= 8) smg_cnt = 0;
	
	if(uartsend_open && ++uart_tt >= 1000)
	{
		uart_tt = 0;
		uartsend_flag = 1;
		if(work_mode)
		{
			read_cnt = (read_cnt + 1) % datacnt;
		}
	}
	
	if(uartrec_flag && ++uartrec_tt >= 50)
	{
		uartrec_tt = 0;
		uartrec_flag = 0;
		uartrec_cnt = 0;
	}
	
	if(stay_flag && ++stay_tt >= 1000)
	{
		stay_tt = 0;
		stay_time_now++;
	}
	
	if(++guang_tt >= 300)
	{
		guang_tt = 0;
		guang_flag = 1;
	}
	
	if(work_mode == 0)
	{
		led_state &= 0xfe;
	}
	else 
	{
		led_state &= 0xfd;
	}
	
	if(coming_flag)
	{
		led_state &= 0xfb;
	}
	
	set_port(0x80,led_state);
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

void UartSend()
{
	if(uartsend_open && uartsend_flag)
	{
		uartsend_flag = 0;
// 12�¶ȣ�45ʪ�ȣ�9.10ʱ��12.13���ӣ�15.16�룬19����
		if(work_mode == 0)
		{
			str1[1] = (u16)temperature / 10 + '0';
			str1[2] = (u16)temperature % 10 + '0';
			str1[4] = (H / 10) + '0';
			str1[5] = (H % 10) + '0';
			str1[9] = (TIME[2] / 16) + '0';
			str1[10] = (TIME[2] % 16) + '0';
			str1[12] = (TIME[1] / 16) + '0';
			str1[13] = (TIME[1] % 16) + '0';
			str1[15] = (TIME[0] / 16) + '0';
			str1[16] = (TIME[0] % 16) + '0';
			SendString(str1);
			SendData(coming_flag + '0');
			SendString(str2);
		}
		else
		{
			if(DataIsNull)
			{
				;
			}
			else
			{
				u16 temp;
				temp = read_at24c02(read_cnt * 7 + 0);
				str1[1] = (temp / 10) + '0';
				str1[2] = (temp % 10) + '0';
				temp = read_at24c02(read_cnt * 7 + 1);
				str1[4] = (temp / 10) + '0';
				str1[5] = (temp % 10) + '0';
				temp = read_at24c02(read_cnt * 7+ 2);
				str1[9] = (temp / 16) + '0';
				str1[10] = (temp % 16) + '0';
				temp = read_at24c02(read_cnt * 7 + 3);
				str1[12] = (temp / 16) + '0';
				str1[13] = (temp % 16) + '0';
				temp = read_at24c02(read_cnt * 7 + 4);
				str1[15] = (temp / 16) + '0';
				str1[16] = (temp % 16) + '0';
				SendString(str1);
				temp = read_at24c02(read_cnt * 7+ 5) * 16 + read_at24c02(read_cnt * 7 + 6);
				if(temp / 10000)
					SendData((temp / 10000) + '0');
				else if(temp / 1000 % 10)
					SendData((temp / 1000 % 10) + '0');
				else if(temp / 100 % 10)
					SendData((temp / 100 % 10) + '0');
				else if(temp / 10 % 10)
					SendData((temp / 10 % 10) + '0');
				SendData((temp % 10) + '0');
				SendString(str2);	
			}		
		}
	}

}

/*----------------------------
UART �жϷ������
-----------------------------*/
void Uart() interrupt 4
{
	
    if (RI)
    {
		if(uartrec_cnt == 0)
		{
			uartrec_flag = 1;
			uartrec_tt = 0;
			uartsend_open = 0;
		}
        RI = 0;                 //���RIλ
        UartRec[uartrec_cnt++] = SBUF;
		if(uartrec_cnt == 6)
		{
			UartRec[uartrec_cnt++] = '\0';
			if(strcmp(UartRec,"AAASSS") == 0)
			{
				uartsend_open = 1;
			}
		}
    }
    if (TI)
    {
        TI = 0;                 //���TIλ
        busy = 0;               //��æ��־
    }
}

void UartInit(void)		//1200bps@12.000MHz
{
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x40;		//��ʱ��1ʱ��ΪFosc,��1T
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//�趨��ʱ��1Ϊ16λ�Զ���װ��ʽ
	TL1 = 0x3C;		//�趨��ʱ��ֵ
	TH1 = 0xF6;		//�趨��ʱ��ֵ
	ET1 = 0;		//��ֹ��ʱ��1�ж�
	TR1 = 1;		//������ʱ��1
}


void main()
{
	allinit();
	Timer2Init();
	UartInit();
	ES = 1;
	IE2 |= 0X04;	// �򿪶�ʱ��2���жϣ�����
	EA = 1;
	init_ds1302();
	write_pcf8591(153);
	while(1)
	{
		
		temperature = read_temperature();
		read_ds1302();
		H = read_pcf8591(3) * 0.39;
		H = read_pcf8591(3) * 0.39;
		if(guang_flag)
		{		
			guang = read_pcf8591(1);
			guang = read_pcf8591(1);	
			guang_flag = 0;
		}
		if(guang < 40)
		{
			if(stay_flag == 0)
			{
				stay_tt = 0;
				write_at24c02(story_cnt * 7 + 0,(u8)temperature);
				Delay5ms();
				write_at24c02(story_cnt * 7 + 1,H);
				Delay5ms();
				write_at24c02(story_cnt * 7 + 2,TIME[2]);
				Delay5ms();
				write_at24c02(story_cnt * 7 + 3,TIME[1]);
				Delay5ms();
				write_at24c02(story_cnt * 7 + 4,TIME[0]);
				Delay5ms();
			}
			stay_flag = 1;
			coming_flag = 1;
		}
		else 
		{
			if(stay_time_now != 0 || coming_flag == 1)
			{
				stay_time = stay_time_now;
				stay_time_now = 0;				
				if(work_mode == 1)
				{
					write_at24c02(story_cnt * 7 + 5,stay_time / 16);
					Delay5ms();
					write_at24c02(story_cnt * 7 + 6,stay_time % 16);
					Delay5ms();
					story_cnt = (story_cnt + 1) % 5;
					DataIsNull = 0;
					if(datacnt != 5) datacnt++;
				}
			}
			stay_tt = 0;
			coming_flag = 0;
			stay_flag = 0;
		}
		display();
		BTN();
		UartSend();
	}
}
