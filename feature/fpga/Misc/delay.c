


#include "delay.h"

#include "stdio.h"




void Delay_5Us(unsigned int s)
{
	unsigned int i;
	while (s--)
	{
		for (i = 0; i < 90; i++);
	}
}


void Delay_Ms(unsigned int s)
{
	unsigned int i;
	while (s--)
	{
		for (i = 0; i < 20970; i++);
	}
}









