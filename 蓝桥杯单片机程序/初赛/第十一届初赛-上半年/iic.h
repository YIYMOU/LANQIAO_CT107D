#ifndef _IIC_H
#define _IIC_H

//º¯ÊıÉùÃ÷
void IIC_Start(void); 
void IIC_Stop(void);  
void IIC_Ack(bit ackbit); 
void IIC_SendByte(unsigned char byt); 
bit IIC_WaitAck(void);  
unsigned char IIC_RecByte(void); 
unsigned char read_at24c02(unsigned char add);
void write_at24c02(unsigned char add,unsigned char dat);
unsigned char read_pcf8591(unsigned char ch);
#endif