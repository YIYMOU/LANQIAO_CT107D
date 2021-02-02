#ifndef _IIC_H
#define _IIC_H

void IIC_Start(void); 
void IIC_Stop(void);  
bit IIC_WaitAck(void);  
void IIC_SendAck(bit ackbit); 
void IIC_SendByte(unsigned char byt); 
unsigned char IIC_RecByte(void); 
unsigned char read_eeprom(unsigned char add);
void write_eeprom(unsigned char add,unsigned char dat);
void write_pcf8591(unsigned char dat);

#endif