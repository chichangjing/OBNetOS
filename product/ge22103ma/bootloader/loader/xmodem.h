
#ifndef __XMODEM_H

/*  ASCII Constants  */
#define	SOH		001 
#define	STX		002
#define	ETX		003
#define	EOT		004
#define	ENQ		005
#define	ACK		006
#define	LF		012
#define	CR		015  
#define	NAK		025
#define	SYN		026
#define	CAN		030
#define	ESC		033

#define	XMODEM_SUCCESS		0
#define	XMODEM_FAILURE		-1
#define ERR_SEQ_NUM         -2
#define ERR_CRC             -3
#define ERR_TIMEOUT			-4
#define ERR_FLASH_WRITE		-5
#define ERR_CHECK_HEADER	-6

int XmodemWrite(unsigned int write_address, int *receive_size);
void CmdXmodem(int argc, char **argv);
void RegisterXmodemCommand(void);
#endif /* __XMODEM_H */

