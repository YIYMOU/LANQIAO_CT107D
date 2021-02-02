#ifndef _IIC_H
#define _IIC_H

//º¯ÊýÉùÃ÷
void IIC_Start(void); 
void IIC_Stop(void);  
void IIC_Ack(bit ackbit); 
void IIC_SendByte(unsigned char byt); 
bit IIC_WaitAck(void);  
unsigned char IIC_RecByte(void); 
unsigned char Read_PCF8591(unsigned char add);
void Write_AT24C02(unsigned char dat,unsigned char add);
unsigned char Read_AT24C02(unsigned char add);
#endif