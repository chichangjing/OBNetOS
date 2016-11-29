#include "halsw_mdio.h"


#if 0

#define	DIROUT	0xffffffff
#define	DIRIN	0x00000000
static void MD_delay(void)
{
	u8 i;
	for (i = 0; i < 10; i++);
};

void HalswMdioInit(void)
{
	GPIO_InitTypeDef gpioinit;
	RCC_APB2PeriphClockCmd(MDIOPPERIPH | MDCPERIPH, ENABLE);
	gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
	gpioinit.GPIO_Mode = GPIO_Mode_Out_OD;
	gpioinit.GPIO_Pin = MDIOPIN;
	GPIO_Init(MDIOPORT, &gpioinit);
	gpioinit.GPIO_Pin = MDCPIN;
	GPIO_Init(MDCPORT, &gpioinit);
	SET_MDIO;
	SET_MDC;
}

void HalMdioDir(unsigned int dir)
{
	GPIO_InitTypeDef gpioinit;
	gpioinit.GPIO_Pin = MDIOPIN;
	gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
	gpioinit.GPIO_Mode = dir ? GPIO_Mode_Out_OD : GPIO_Mode_IN_FLOATING;
	GPIO_Init(MDIOPORT, &gpioinit);
}



/*--------------------------- output_MDIO -----------------------------------*/

static void output_MDIO(u32 val, u32 n)
{
	/* Output a value to the MII PHY management interface. */
	for (val <<= (32 - n); n; val <<= 1, n--)
	{
		if (val & 0x80000000)
		{
			SET_MDIO;
		}
		else
		{
			CLR_MDIO;
		}
		MD_delay();
		SET_MDC;
		MD_delay();
		CLR_MDC;
	}
}


/*--------------------------- turnaround_MDIO -------------------------------*/

static void turnaround_MDIO(void)
{
	/* Turnaround MDO is tristated. */
	GPIO_InitTypeDef gpioinit;
	gpioinit.GPIO_Pin = MDIOPIN;
	gpioinit.GPIO_Speed = GPIO_Speed_10MHz;
	gpioinit.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(MDIOPORT, &gpioinit);
	SET_MDC;
	MD_delay();
	CLR_MDC;
	MD_delay();
}

/*--------------------------- input_MDIO ------------------------------------*/

static u32 input_MDIO(void)
{
	/* Input a value from the MII PHY management interface. */
	u32 i, val = 0;
	for (i = 0; i < 16; i++)
	{
		val <<= 1;
		SET_MDC;
		MD_delay();
		CLR_MDC;
		if (MDIOPORT->IDR & MDIOPIN)
		{
			val |= 1;
		}
	}
	return (val);
}


u16 mdio_read(u32 PHY_ADR, int PhyReg)
{
	u16 val;
	/* Configuring MDC on P2.8 and MDIO on P2.9 */
	//GPIO2->FIODIR |= MDIO;
	GPIO_InitTypeDef gpioinit;
	gpioinit.GPIO_Pin = MDIOPIN;
	gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
	gpioinit.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(MDIOPORT, &gpioinit);
	/* 32 consecutive ones on MDO to establish sync */
	output_MDIO(0xFFFFFFFF, 32);
	output_MDIO(0xFFFFFFFF, 32);
	/* start code (01), read command (10) */
	output_MDIO(0x06, 4);
	/* write PHY address */
	output_MDIO(PHY_ADR, 5);
	/* write the PHY register to write */
	output_MDIO(PhyReg, 5);
	/* turnaround MDO is tristated */
	turnaround_MDIO();
	/* read the data value */
	val = input_MDIO();
	/* turnaround MDIO is tristated */
	turnaround_MDIO();
	return (val);
}


void mdio_write(u32 PHY_ADR, int PhyReg, int Value)
{
	/* Configuring MDC on P2.8 and MDIO on P2.9 */
	//GPIO2->FIODIR |= MDIO;
	GPIO_InitTypeDef gpioinit;
	gpioinit.GPIO_Pin = MDIOPIN;
	gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
	gpioinit.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(MDIOPORT, &gpioinit);
	/* 32 consecutive ones on MDO to establish sync */
	output_MDIO(0xFFFFFFFF, 32);
	output_MDIO(0xFFFFFFFF, 32);
	/* start code (01), write command (01) */
	output_MDIO(0x05, 4);
	/* write PHY address */
	output_MDIO(PHY_ADR, 5);
	/* write the PHY register to write */
	output_MDIO(PhyReg, 5);
	/* turnaround MDIO (1,0)*/
	output_MDIO(0x02, 2);
	/* write the data value */
	output_MDIO(Value, 16);
	/* turnaround MDO is tristated */
	turnaround_MDIO();
}


void MdioCL45WriteA(unsigned int phyadd, unsigned int devadd, unsigned  int regadd)
{
	HalMdioDir(DIROUT);
	/* 32 consecutive ones on MDO to establish sync */
	output_MDIO(0xFFFFFFFF, 32);
	output_MDIO(0xFFFFFFFF, 32);
	/* start code (01), write command (01) */
	output_MDIO(0x00, 4);
	/* write PHY address */
	output_MDIO(phyadd, 5);
	/* write the PHY register to write */
	output_MDIO(devadd, 5);
	/* turnaround MDIO (1,0)*/
	output_MDIO(0x02, 2);
	/* write the data value */
	output_MDIO(regadd, 16);
	/* turnaround MDO is tristated */
	turnaround_MDIO();
}

void MdioCL45WriteV(unsigned int phyadd, unsigned int devadd, unsigned  int value)
{
	HalMdioDir(DIROUT);
	/* 32 consecutive ones on MDO to establish sync */
	output_MDIO(0xFFFFFFFF, 32);
	output_MDIO(0xFFFFFFFF, 32);
	/* start code (01), write command (01) */
	output_MDIO(0x01, 4);
	/* write PHY address */
	output_MDIO(phyadd, 5);
	/* write the PHY register to write */
	output_MDIO(devadd, 5);
	/* turnaround MDIO (1,0)*/
	output_MDIO(0x02, 2);
	/* write the data value */
	output_MDIO(value, 16);
	/* turnaround MDO is tristated */
	turnaround_MDIO();
}

unsigned short int MdioCL45Read(unsigned char phyadd, unsigned char devadd)
{
	unsigned short int val;
	HalMdioDir(DIROUT);
	/* 32 consecutive ones on MDO to establish sync */
	output_MDIO(0xFFFFFFFF, 32);
	output_MDIO(0xFFFFFFFF, 32);
	/* start code (01), read command (10) */
	output_MDIO(0x03, 4);
	/* write PHY address */
	output_MDIO(phyadd, 5);
	/* write the PHY register to write */
	output_MDIO(devadd, 5);
	/* turnaround MDO is tristated */
	turnaround_MDIO();
	/* read the data value */
	val = input_MDIO();
	/* turnaround MDIO is tristated */
	turnaround_MDIO();
	return (val);
}

unsigned short int MdioCL45IncRead(unsigned char phyadd, unsigned char devadd)
{
	unsigned short int val = 0;
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = MDIOPIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(MDIOPORT, &GPIO_InitStructure);
	/* 32 consecutive ones on MDO to establish sync */
	output_MDIO(0xFFFFFFFF, 32);
	output_MDIO(0xFFFFFFFF, 32);
	/* start code (01), read command (10) */
	output_MDIO(0x02, 4);
	/* write PHY address */
	output_MDIO(phyadd, 5);
	/* write the PHY register to write */
	output_MDIO(devadd, 5);
	/* turnaround MDO is tristated */
	turnaround_MDIO();
	/* read the data value */
	val = input_MDIO();
	/* turnaround MDIO is tristated */
	turnaround_MDIO();
	return (val);
}





u16 Indirect_mdio_read(u32 PHY_ADR, int PhyReg)
{
	mdio_write(PHY_ADR, 0x1E, PhyReg);
	return mdio_read(PHY_ADR, 0x1F);
}


void Indirect_mdio_write(u32 PHY_ADR, int PhyReg, int Value)
{
	mdio_write(PHY_ADR, 0x1E, PhyReg);
	mdio_write(PHY_ADR, 0x1F, Value);
}


#endif



