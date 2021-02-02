#ifndef _IIC_H
#define _IIC_H

void IIC_Start(void); 
void IIC_Stop(void);  
bit IIC_WaitAck(void);  
void IIC_SendAck(bit ackbit); 
void IIC_SendByte(unsigned char byt); 
unsigned char IIC_RecByte(void); 
void init_ds1302(void);
unsigned char read_at24c02(unsigned char add);
void write_at24c02(unsigned char add,unsigned char dat);
void write_pcf8591(unsigned char dat);
unsigned char read_pcf8591(unsigned char add);

#endif