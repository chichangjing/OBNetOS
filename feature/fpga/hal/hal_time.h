
#ifndef	_HAL_TIME_H
#define	_HAL_TIME_H




#define	U32TIMEOUT(ctime, htime)	(ctime = (ctime>= htime) ? (ctime - htime) : ((unsigned int)~(0) - htime + ctime + 1))
#define	U16TIMEOUT(ctime, htime)	(ctime = (ctime>= htime) ? (ctime - htime) : ((unsigned short int)~(0) - htime + ctime + 1))
#define	U8TIMEOUT(ctime, htime)		(ctime = (ctime>= htime) ? (ctime - htime) : ((unsigned char)~(0) - htime + ctime + 1))




void SysTickGet(unsigned int *time);
void SysTickSet(unsigned int *time);




#endif//end hal_time.h
