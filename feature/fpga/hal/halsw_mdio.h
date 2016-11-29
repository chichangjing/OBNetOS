/******************************************************************************
 * @file:    mdio.h
 * @purpose: CMSIS Cortex-M3 Core Peripheral Access Layer Header File for
 *           NXP LPC17xx Device Series
 * @version: V1.1
 * @date:    14th May 2009
 *---------------------------------------------------------------------------- */

#ifndef _HALSW_MDIO_H
#define _HALSW_MDIO_H

#include "stm32f2xx_gpio.h"
#include "stm32f2xx_rcc.h"

//#define MDIO    0x00000200
//#define MDC     0x00000100


#define	MDIOPIN		GPIO_Pin_11
#define	MDIOPORT		GPIOA
#define	MDIOPPERIPH	RCC_APB2Periph_GPIOA
#define	MDCPIN			GPIO_Pin_12
#define	MDCPORT		GPIOA
#define	MDCPERIPH		RCC_APB2Periph_GPIOA


#define SET_MDIO		(MDIOPORT->BSRR = MDIOPIN)//GPIO_WriteBit(MDIOPORT, MDIOPIN, (BitAction)(1));
#define CLR_MDIO		(MDIOPORT->BRR = MDIOPIN)//GPIO_WriteBit(MDIOPORT, MDIOPIN, (BitAction)(0));

//#define SET_MDIO  GPIO_SetBits(MDIO_PROT, MDIO_PIN)
//#define CLR_MDIO  GPIO_ResetBits(MDIO_PROT, MDIO_PIN)
#define SET_MDC			(MDCPORT->BSRR = MDCPIN)//GPIO_SetBits(MDCPORT, MDCPIN)
#define CLR_MDC			(MDCPORT->BRR = MDCPIN)//GPIO_ResetBits(MDCPORT, MDCPIN)

/* mdio msg fmt */
#define	CLAUSE22		0x01
#define	CL22READOP		0x02
#define	CL22WRITEOP	0x01

#define	CLAUSE45		0x00
#define	CL45ADDROP		0x00
#define	CL45READOP		0x01
#define	CL45WRITEOP	0x03
#define	CL45RDLNCOP	0x02

#define	CLXFMT(start, opcode)	(((start & 0x0003) << 2) | (opcode & 0x0003))




#define	A_ADR		0x0000
#define	B_ADR		0x0001

void	HalswMdioInit(void);

u16 mdio_read(u32 PHY_ADR, int PhyReg);
void mdio_write(u32 PHY_ADR, int PhyReg, int Value);

u16 Indirect_mdio_read(u32 PHY_ADR, int PhyReg);
void Indirect_mdio_write(u32 PHY_ADR, int PhyReg, int Value);

void MdioCL45WriteA(unsigned int phyadd, unsigned int devadd, unsigned  int regadd);
void MdioCL45WriteV(unsigned int phyadd, unsigned int devadd, unsigned  int value);
unsigned short int MdioCL45Read(unsigned char phyadd, unsigned char devadd);
unsigned short int MdioCL45IncRead(unsigned char phyadd, unsigned char devadd);







#endif  // __mdio_H__
