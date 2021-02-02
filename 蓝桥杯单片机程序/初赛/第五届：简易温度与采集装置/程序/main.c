#include <STC15F2K60S2.H>
#include "onewire.h"
#define u8	unsigned char
#define u16 unsigned int

u8 code LED[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xbf,0xff};
u8 yi,er,san,si,wu,liu,qi,ba,shezhi=0,Tmin=20,Tmax=30,cnt=0,num,flag=0;
u16 timermax,f=0;

void delay_ms(u16 ms)
{
	u16 i,j;
	for(i=ms ; i>0 ; i--)
		for(j=845 ; j>0 ;j--);
	
}
void relay_open()
{
	P2=0XA0;
	P0=0X10;
}
void relay_close()
{
	P2=0XA0;
	P0=0X00;
}
void Init()
{
	P2=0x80;	//�ر�LED��
	P0=0xFF;
	
	P2=0XA0;	//�رշ������ͼ̵�����
	P0=0X00;
	
	P2=0XC0;	//�ر������λѡ
	P0=0XFF;
	
	P2=0XFF;	//�ر�����ܶ�ѡ
	P0=0XFF;
}

void Display()
{
	P2=0XC0;	
	P0=0X01;	//ѡ�������1
	P2=0XFF;	//����ܶ�ѡ
	P0=LED[yi];
	delay_ms(1);
	
	P2=0XC0;	
	P0=0X02;	//ѡ�������2
	P2=0XFF;	
	P0=LED[er];
	delay_ms(1);
	
	P2=0XC0;	
	P0=0X04;	//ѡ�������3
	P2=0XFF;	
	P0=LED[san];
	delay_ms(1);
	
	P2=0XC0;	
	P0=0X08;	//ѡ�������4
	P2=0XFF;	
	P0=LED[si];
	delay_ms(1);
	
	P2=0XC0;	
	P0=0X10;	//ѡ�������5
	P2=0XFF;	
	P0=LED[wu];
	delay_ms(1);
	
	P2=0XC0;	
	P0=0X20;	//ѡ�������6
	P2=0XFF;	
	P0=LED[liu];
	delay_ms(1);
	
	P2=0XC0;	
	P0=0X40;	//ѡ�������7
	P2=0XFF;	
	P0=LED[qi];
	delay_ms(1);
	
	P2=0XC0;	
	P0=0X80;	//ѡ�������8
	P2=0XFF;	
	P0=LED[ba];
	delay_ms(1);
	
}
void Key_KBD()	//Button(����)����д, KBDΪKeyboard(����)
{
	u8 temp;
	P3=0X7F;P44=0;P42=1;
	temp=P3;
	temp=temp&0x0f;
	if(temp!=0x0f){
		delay_ms(5);
		temp=P3;
		temp=temp&0x0f;
		if(temp!=0x0f){
			temp=P3;
			switch(temp){
				case 0x7e:num=0;cnt++;break;
				case 0x7d:num=3;cnt++;break;
				case 0x7b:num=6;cnt++;break;
				case 0x77:num=9;cnt++;break;
			}
		}
		while(temp!=0x0f){
			temp = P3;
			temp&=0x0f;
		}
	}
	
	P3=0XbF;P44=1;P42=0;
	temp=P3;
	temp=temp&0x0f;
	if(temp!=0x0f){
		delay_ms(5);
		temp=P3;
		temp=temp&0x0f;
		if(temp!=0x0f){
			temp=P3;
			switch(temp){
				case 0xbe:num=1;cnt++;break;
				case 0xbd:num=4;cnt++;break;
				case 0xbb:num=7;cnt++;break;
				case 0xb7:shezhi=!shezhi;EA=!EA;break;
			}
		}
		while(temp!=0x0f){
			temp = P3;
			temp&=0x0f;
		}
	}

	P3=0XdF;P44=1;P42=1;
	temp=P3;
	temp=temp&0x0f;
	if(temp!=0x0f){
		delay_ms(5);
		temp=P3;
		temp=temp&0x0f;
		if(temp!=0x0f){
			temp = P3;
			switch (temp){
				case 0xde:num=2;cnt++;break;
				case 0xdd:num=5;cnt++;break;
				case 0xdb:num=8;cnt++;break;
				case 0xd7:cnt=0;break;
			}
		}
		while(temp!=0x0f){
			temp = P3;
			temp&=0x0f;
		}
	}
	
	P3=0XeF;P44=1;P42=1;
	temp=P3;
	temp=temp&0x0f;
	if(temp!=0x0f){
		delay_ms(5);
		temp=P3;
		temp=temp&0x0f;
		if(temp!=0x0f){
			temp = P3;
			switch (temp){
				case 0xee:yi=1;er=9;break;
				case 0xed:yi=1;er=8;break;
				case 0xeb:yi=1;er=7;break;
				case 0xe7:yi=1;er=6;break;
			}
		}
		while(temp!=0x0f){
			temp = P3;
			temp&=0x0f;
		}
	}
}
void Timer0Init(void)		//1����@11.0592MHz
{
	AUXR |= 0x80;		//��ʱ��ʱ��1Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TL0 = 0xCD;		//���ö�ʱ��ֵ
	TH0 = 0xD4;		//���ö�ʱ��ֵ
	TF0 = 0;		//���TF0��־
	TR0 = 1;		//��ʱ��0��ʼ��ʱ
	ET0=1;
	EA=1;
}
void timer0()interrupt 1
{
	if(++f>timermax-1)
	{
		f=0;
		if(flag)
		{
			P2=0X80;
			P0=0Xfe;
		}
		else
		{
			P2=0X80;
			P0=0Xff;
		}
		flag=!flag;
	}
}
void main()
{
	u8 temp;
	Init();	//�Ƚ��������ʼ��
	Timer0Init();
	while(1){
		if(shezhi)
		{
			relay_close();
			yi=10;liu=10;si=11;wu=11;
			if(cnt==0)
			{
				er=11;san=11;qi=11;ba=11;
			}
			else if(cnt==1)
			{
				er=num;
			}
			else if(cnt==2)
			{
				san=num;
			}
			else if(cnt==3)
			{
				qi=num;
			}
			else if(cnt==4)
			{
				ba=num;
				Tmin=qi*10+ba;
				Tmax=er*10+san;
				if(Tmax<Tmin)
				{
					
					P2=0X80;
					P0=0Xfd;
				}
				else
				{
					P2=0X80;
					P0=0XFF;
				}
			}
		}
		else
		{
			cnt=0;
			yi=10;er=1;san=10;si=11;wu=11;liu=11;
			temp = Tempget();
			qi=temp/10;ba=temp%10;
			if(temp<Tmin)
			{
				er=0;
				relay_close();
				timermax=800;
			}				
			else if(temp>Tmax)
			{
				er=2;
				relay_open();
				timermax=200;
			}	
			else
			{
				er=1;
				relay_close();
				timermax=400;
			}	
		}
		Key_KBD();
		Display();
	}
}