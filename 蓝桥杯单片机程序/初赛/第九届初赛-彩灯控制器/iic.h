#ifndef _IIC_H
#define _IIC_H

void IIC_Start(void); 
void IIC_Stop(void);  
bit IIC_WaitAck(void);  
void IIC_SendByte(unsigned char byt); 
unsigned char IIC_RecByte(void); 
void WRITE_EEPROM(unsigned char add,unsigned char dat);
unsigned char READ_EEPROM(unsigned char add);
unsigned char READ_PCF8591(unsigned char ch);
#endif