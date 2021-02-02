#ifndef _IIC_H
#define _IIC_H

void IIC_Start(void); 
void IIC_Stop(void);  
bit IIC_WaitAck(void);  
void IIC_SendByte(unsigned char byt); 
unsigned char IIC_RecByte(void); 
unsigned char IIC_Read(unsigned char add);
#endif