
#include "si5327.h"
#include "halsw_i2c.h"
#include "delay.h"


#if (defined SI5324)
const unsigned char cregaddr[43] = {  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
                                      0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1f, 0x20, 0x21, 0x22, 0x23,
                                      0x24, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x37, 0x83,
                                      0x84, 0x89, 0x8a, 0x8b, 0x8e, 0x8f, 0x88
                                   };
#elif (defined SI5327)
const unsigned char cregaddr[41] = {  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08, 0x09, 0x0a, 0x0b, 0x13, 0x14,
                                      0x16, 0x17, 0x18, 0x19, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x28, 0x29,
                                      0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x80, 0x81, 0x82, 0x83, 0x84,
                                      0x89, 0x8a, 0x8b, 0x88
                                   };
#endif




unsigned char Si5324GetReg(unsigned char devid, unsigned char devaddr, unsigned char regadd)
{
	unsigned char ret;
	SWI2cRandomRead(devid, devaddr, regadd, &ret);
	return ret;
}

void Si5324AllGetReg(unsigned char devid, unsigned char devaddr, si5324confregp cregp)
{
	unsigned char i;
	for (i = 0; i < sizeof(cregaddr); i++)
	{
		SWI2cRandomRead(devid, devaddr, cregaddr[i], &cregp->reg[i]);
		Delay_5Us(10);
	}
}


void Si5324Conf(unsigned char devid, unsigned char devaddr, si5324confregp cregp, const unsigned char *name)
{
	unsigned char i;

	//added for test
	unsigned int err5324;
#if DEBUG_PRINTF
	printf("Seting Si5327 clock frequency: %s\r\n", name);
#endif

	for (i = 0; i < sizeof(cregaddr); i++)
	{
		err5324 = SWI2cRandomWrite(devid, devaddr, cregaddr[i], &cregp->reg[i]);
		Delay_5Us(2);
	}
}


int Si5324LockCheck(unsigned char devid, unsigned char devaddr)
{
	unsigned char value[2];
	int lockstate = 0;
	value[0] = Si5324GetReg(devid, devaddr, 0x80);
	value[1] = Si5324GetReg(devid, devaddr, 0x82);
	if (!(value[1] & 0x01))
	{
		switch (value[0] & 0x03)
		{
			case 0x01:
				lockstate = 1;
				break;
			case 0x02:
				lockstate = 2;
				break;
			default:
				lockstate = 0;
				break;
		}
	}
	return lockstate;
}

