#ifndef __DS1302_H
#define __DS1302_H

void Write_Ds1302_Byte(unsigned  char temp);
void Write_Ds1302( unsigned char address,unsigned char dat );
unsigned char Read_Ds1302 ( unsigned char address );
void Ds1302_Init(void);
void DS1302_Timeget(void);
#endif
