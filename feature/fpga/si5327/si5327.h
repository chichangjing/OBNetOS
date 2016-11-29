/******************************************************************************
* @file    clkout_5324.c
* @author  wangjingwei
* @version V1.0.0
* @date    2014.03.20
  *************************************************************/

#ifndef _SI5324_H_
#define _SI5324_H_

#if (defined SI5324)
typedef struct SI5324_CONFREG
{
	unsigned char reg[43];
} si5324confreg, *si5324confregp;
#elif (defined SI5327)
typedef struct SI5324_CONFREG
{
	unsigned char reg[41];
} si5324confreg, *si5324confregp;
#endif

/* 像素时钟配置 输出模式为lvds OUT2 输出 */
extern const si5324confreg cfg_7425;
extern const si5324confreg cfg_15625;
extern const si5324confreg cfg_7425_1485;
extern const si5324confreg cfg_7425_1485_PECL;
extern const si5324confreg cfg_7425_1485_7425;
extern const si5324confreg cfg_7425_1485_1485;

unsigned char Si5324GetReg(unsigned char devid, unsigned char devaddr, unsigned char regadd);
void Si5324AllGetReg(unsigned char devid, unsigned char devaddr, si5324confregp cregp);
void Si5324Conf(unsigned char devid, unsigned char devaddr, si5324confregp cregp, const unsigned char *name);
int Si5324LockCheck(unsigned char devid, unsigned char devaddr);

#endif /* _SI5324_H_ */
