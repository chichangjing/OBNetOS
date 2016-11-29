




#include "stdio.h"
#include "fifo.h"




#if 0
static char Fifo_Active(fifo_p  fifop)
{
	return ((fifop->remain) ? 1 : 0);
}


void Fifo_Init(fifo_p  fifop)
{
#if (defined STDIO)
	memset(fifop, 0, sizeof(fifo_t));
#else
	unsigned int i = sizeof(fifo_t);
	unsigned char *p  = (unsigned char *)fifop;
	while (i--)
	{
		*p = 0;
		p++;
	}
#endif
}



void Fifo_Add_Value(fifo_p  fifop, unsigned char vlue)
{
	unsigned char temp;
	if (fifop->remain == 0)
	{
		fifop->buff[fifop->head] = vlue;
	}
	else
	{
		temp = (fifop->head + fifop->remain) % sizeof(fifop->buff);
		fifop->buff[temp] = vlue;
		if (fifop->head == temp)
		{
			fifop->head++;
			fifop->head %= sizeof(fifop->buff);
		}
	}
	if ((++fifop->remain) > sizeof(fifop->buff))
	{
		fifop->remain = sizeof(fifop->buff);
	}
}

void Fifo_write(fifo_p  fifop, unsigned char *buf, unsigned int size)
{
	unsigned char i = size;
	while (i--)
	{
		Fifo_Add_Value(fifop, *buf);
		buf++;
	}
}


unsigned char Fifo_Read(fifo_p  fifop, unsigned char *buf, unsigned int size)
{
	unsigned int i = 0;
	/* ¼ì²âÊÇ·ñÓÐ°´¼ü */
	if (!Fifo_Active(fifop))
	{
		return 0;
	}
	if (size == 0)
	{
		return 0;
	}
	if (size > fifop->remain)
	{
		size = fifop->remain;
	}
	for (i = 0; i < size; i++)
	{
		*(buf++) = fifop->buff[fifop->head++];
		if (fifop->head == sizeof(fifop->buff))
		{
			fifop->head = 0x00;
		}
	}
	fifop->remain -= size;
	return size;
}
#endif



void Fifo_Init(fifo_p  fifop)
{
#if (defined STDIO)
	memset(fifop, 0, sizeof(fifo_t));
#else
	unsigned int i = sizeof(fifo_t);
	unsigned char *p  = (unsigned char *)fifop;
	while (i--)
	{
		*p = 0;
		p++;
	}
#endif
}


void Fifo_Add_Value(fifo_p  fifop, unsigned char value)
{
	fifop->buff[fifop->tail++] = value;
	if (fifop->tail == MAXSIZE)
	{
		fifop->tail = 0;
	}
}


unsigned char Fifo_Read(fifo_p fifop, unsigned char *buf, unsigned int size)
{
	unsigned int i = 0;
	unsigned int acsize;
	if (size == 0)
	{
		return 0;
	}
	if (fifop->tail > fifop->head)
	{
		acsize = fifop->tail - fifop->head;
	}
	else
	{
		acsize = MAXSIZE - (fifop->tail - fifop->head);
	}
	size = (size >= acsize) ? acsize : size;
	for (i = 0; i < size; i++)
	{
		*(buf++) = fifop->buff[fifop->head++];
		if (fifop->head == MAXSIZE)
		{
			fifop->head = 0x00;
		}
	}
	return size;
}


void Fifo_Write(fifo_p  fifop, unsigned char *buf, unsigned int size)
{
	unsigned char i = size;
	while (i--)
	{
		Fifo_Add_Value(fifop, *buf);
		buf++;
	}
}
