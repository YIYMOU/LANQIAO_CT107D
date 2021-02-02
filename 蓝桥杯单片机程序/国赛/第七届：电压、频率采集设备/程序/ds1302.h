#ifndef __DS1302_H
#define __DS1302_H

void Write_Ds1302(unsigned char temp);
void Write_Ds1302_Byte( unsigned char address,unsigned char dat );
unsigned char Read_Ds1302_Byte( unsigned char address );
void init_ds1302(void);
void read_ds1302(void);
#endif
