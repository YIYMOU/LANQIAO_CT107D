#include <STC15F2K60S2.H>
#include <IIC.H>

#define u8 unsigned char
#define u16 unsigned int

u8 code TAB[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0XBF,0XFF};
u8 disbuf[] = {0,1,2,3,4,5,6,7,8};

sbit TX = P1^0;
sbit RX = P1^1;

u8 key_buf = 0;

u8 smg_cnt = 0;

u8 distance;
u8 wave_tt = 0;
bit wave_flag = 0;

bit start_trans = 0;
bit alarm_stop = 0;

u16 LedFlicker_tt = 0;
bit LedFlicker_flag = 0;
bit LedFlicker_open = 0;
bit buzzer_flag = 0;
bit relay_flag = 0;
bit alarm_stop_flag = 0;
u16 Vo;

u16 smg_flicker_tt = 0;
bit smg_flicker_flag = 0;

u8 I_time = 2;
u8 II_time = 4;
u16 trans_tt = 0;

u8 remain_sec = 0;

u8 display_mode = 0;

u16 onesec_tt = 0;

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

void allinit(void)
{
	set_port(0x80,0xff);
	set_port(0xa0,0x00);
	set_port(0xc0,0xff);
	set_port(0xe0,0xff);
}

void display(void)
{
	if(display_mode == 0)
	{
		disbuf[1] = 1;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = distance / 10 % 10;
		disbuf[5] = distance % 10;
		disbuf[6] = 11;
		disbuf[7] = 11;
		if(distance <= 30)
			disbuf[8] = 1;
		else 
			disbuf[8] = 2;		
	}
	else if(display_mode == 1)
	{
		disbuf[1] = 2;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = 11;
		disbuf[5] = 11;
		disbuf[6] = 11;
		disbuf[7] = remain_sec / 10;
		disbuf[8] = remain_sec % 10;		
	}
	else if(display_mode == 2 || display_mode == 3 )
	{
		disbuf[1] = 3;
		disbuf[2] = 11;
		disbuf[3] = 11;
		disbuf[4] = I_time / 10;
		disbuf[5] = I_time % 10;
		disbuf[6] = 11;
		disbuf[7] = II_time / 10;
		disbuf[8] = II_time % 10;
	}

}

void BTN(void)
{
	u8 key_temp;
	key_temp = P3 & 0X0F;
	if(key_temp != 0x0f && key_buf == 0)
	{
		Delay5ms();
		key_temp = P3 & 0X0F;
		if(key_temp != 0x0f && key_buf == 0)
		{
			key_buf = key_temp;
		}
	}
	else if(key_temp == 0x0f && key_buf)
	{
		Delay5ms();
		key_temp = P3 & 0X0F;
		if(key_temp == 0x0f && key_buf != 0)
		{
			switch(key_buf)
			{
				case 0x0e: 
					if(display_mode == 2)
					{
						I_time++;
						if(I_time >= 11) I_time = 1;
					}
					else if(display_mode == 3)
					{
						II_time++;
						if(II_time >= 11) II_time = 1;
					}
				break;
				case 0x0d: 
					if(display_mode == 0 || display_mode == 1)
						display_mode = 2; 
					else if(display_mode == 2)
						display_mode = 3; 
					else
					{
						display_mode = 0; 
						write_eeprom(0x00,I_time);
						Delay5ms();
						write_eeprom(0x01,II_time);
						Delay5ms();
					}
				break;
				case 0x0b: 
					if(start_trans)
					{
						alarm_stop = !alarm_stop;
						if(alarm_stop)
							relay_flag = 0;
						else
							relay_flag = 1;
					}
				break;
				case 0x07: 
					if(alarm_stop == 0 && start_trans == 0 && Vo < 400 && Vo >= 100)
					{
						display_mode = 1;
						start_trans = 1; 
						onesec_tt = 0;
						relay_flag = 1;
						if(distance <= 30)
						{
							remain_sec = I_time;
						}
						else
						{
							remain_sec = II_time;
						}
					}
				break;
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
	u8 led_state = 0xff;
	set_port(0xc0, 0x01 << smg_cnt);
	if(++smg_flicker_tt >= 500)
	{
		smg_flicker_tt = 0;
		smg_flicker_flag = !smg_flicker_flag;
	}
	if(display_mode == 2 && (smg_cnt == 3 || smg_cnt == 4))
	{
		if(smg_flicker_flag)
		{
			set_port(0xe0, TAB[disbuf[smg_cnt + 1]]);
		}
		else 
		{
			set_port(0xe0, TAB[11]);
		}
	}
	else if(display_mode == 3 && (smg_cnt == 6 || smg_cnt == 7))
	{
		if(smg_flicker_flag)
		{
			set_port(0xe0, TAB[disbuf[smg_cnt + 1]]);
		}
		else 
		{
			set_port(0xe0, TAB[11]);
		}
	}
	else 
	{
		if(Vo < 100 && display_mode == 0)
			set_port(0xe0, TAB[11]);
		else 
			set_port(0xe0, TAB[disbuf[smg_cnt + 1]]);
	}
	if(++smg_cnt >= 8) smg_cnt = 0;
	
	if(alarm_stop == 0 && start_trans && ++onesec_tt >= 1000)
	{
		onesec_tt = 0;
		remain_sec--;
		if(remain_sec == 0)
		{
			start_trans = 0;
			relay_flag = 0;
			display_mode = 0;
		}
	}
	
	if(++wave_tt >= 200)
	{
		wave_tt = 0;
		wave_flag = 1;
	}
	
	if(Vo >= 400)
	{
		LedFlicker_open = 1;
	}
	else if(Vo < 100)
	{
		led_state &= 0xfe;
		LedFlicker_open = 0;
	}
	else 
	{
		led_state &= 0xfd;
		LedFlicker_open = 0;
	}
	
	if(++LedFlicker_tt >= 500)
	{
		LedFlicker_tt = 0;
		if(LedFlicker_open)
		{
			LedFlicker_flag = !LedFlicker_flag;
		}
		
		if(alarm_stop)
		{
			alarm_stop_flag = !alarm_stop_flag;
		}
	}
	if(LedFlicker_flag == 0 || Vo < 400)
	{
		led_state |= 0x04;
	}
	else
	{
		led_state &= 0xfb;
	}
	if(alarm_stop_flag == 0 || alarm_stop == 0)
	{
		led_state |= 0x08;
	}
	else
	{
		led_state &= 0xf7;
	}
	
	set_port(0x80,led_state);
	
	if(Vo >= 400)
		buzzer_flag = 1;
	else 
		buzzer_flag = 0;
	
	if(buzzer_flag && relay_flag)
	{
		set_port(0xa0,0x40 | 0x10);
	}
	else if(buzzer_flag)
	{
		set_port(0xa0,0x40);
	}
	else if(relay_flag)
	{
		set_port(0xa0,0x10);
	}
	else
	{
		set_port(0xa0,0x00);
	}
}

void Timer1Init(void)		//12微秒@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0xF4;		//设置定时初值
	TH1 = 0xFF;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 0;		//定时器1关闭
}

void wave_rec()
{
	if(wave_flag)
	{
		u8 num = 10;
		wave_flag = 0;
		TX = 0;
		TL1 = 0xF4;		//设置定时初值
		TH1 = 0xFF;		//设置定时初值
		TR1 = 1;
		while(num--)
		{
			while(!TF1);
			TF1 = 0;
			TX ^= 1;
		}
		TR1 = 0;
		TL1 = 0;		//设置定时初值
		TH1 = 0;		//设置定时初值
		TR1 = 1;
		while(!TF1 && RX);
		TR1 = 0;
		if(TF1)
		{
			TF1 = 0;
			distance = 99;
		}
		else
		{
			distance = ((TH1 << 8) | TL1) * 0.017;
			if(distance > 99) distance = 99;
		}
	}
}


void main(void)
{
	allinit();
	Timer1Init();
	Timer2Init();
	IE2 |= 0X04;
	EA = 1;
	I_time = read_eeprom(0x00);
	if(I_time > 10) I_time = 2;
	II_time = read_eeprom(0x01);
	if(II_time > 10) II_time = 2;
	while(1)
	{
		Vo = read_pcf8591(3) * 1.96;
		Vo = read_pcf8591(3) * 1.96;
		wave_rec();
		BTN();
		display();
	}
}