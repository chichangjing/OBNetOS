#ifndef __FIFO_H
#define __FIFO_H



#define MAXSIZE	640

#if 0
typedef struct FIFO_STATCK
{
	unsigned int		head;
	unsigned int		remain;
	unsigned char 	buff[MAXSIZE];
} fifo_t, *fifo_p;
#endif


typedef struct FIFO_STATCK
{
	unsigned int		head;
	unsigned int		tail;
	unsigned char 	buff[MAXSIZE];
} fifo_t, *fifo_p;



void Fifo_Add_Value(fifo_p  fifop, unsigned char vlue);
void Fifo_Write(fifo_p  fifop, unsigned char *buf, unsigned int size);
unsigned char Fifo_Read(fifo_p  fifop, unsigned char *buf, unsigned int size);



#endif //fifo.h
