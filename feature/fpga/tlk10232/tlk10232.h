


#ifndef TLK10232_H
#define TLK10232_H

typedef unsigned short int subaddr_t;

/* Vendor Specific Device Registers */
#define	VSPDRADDR			((unsigned char)0x1e)

#define	GBL_CTL1			((subaddr_t)0x0000)
#define	CHAN_CTL1			((subaddr_t)0x0001)
#define	HSSDS_CTL1			((subaddr_t)0x0002)
#define	HSSDS_CTL2			((subaddr_t)0x0003)
#define	HSSDS_CTL3			((subaddr_t)0x0004)
#define	HSSDS_CTL4			((subaddr_t)0x0005)
#define	LSSDS_CTL1			((subaddr_t)0x0006)
#define	LSSDS_CTL2			((subaddr_t)0x0007)
#define	LSSDS_CTL3			((subaddr_t)0x0008)
#define	HSOLAY_CTL			((subaddr_t)0x0009)
#define	LSOLAY_CTL			((subaddr_t)0x000a)
#define	LBTP_CTL			((subaddr_t)0x000b)
#define	LSCFG_CTL			((subaddr_t)0x000c)
#define	CLK_CTL				((subaddr_t)0x000d)
#define	RESET_CTL			((subaddr_t)0x000e)
#define	CHAN_STU			((subaddr_t)0x000f)
#define	HSERR_CNT			((subaddr_t)0x0010)
#define	LSLN0ERR_CNT		((subaddr_t)0x0011)
#define	LSLN1ERR_CNT		((subaddr_t)0x0012)
#define	LSLN2ERR_CNT		((subaddr_t)0x0013)
#define	LSLN3ERR_CNT		((subaddr_t)0x0014)
#define	LS_STU1				((subaddr_t)0x0015)
#define	HS_STU1				((subaddr_t)0x0016)
#define	DST_CTL1			((subaddr_t)0x0017)
#define	DST_CTL2			((subaddr_t)0x0018)
#define	DSR_CTL1			((subaddr_t)0x0019)
#define	DSR_CTL2			((subaddr_t)0x001a)
#define	DATASWH_STU			((subaddr_t)0x001b)
#define	LSCH_CTL1			((subaddr_t)0x001c)
#define	HSCH_CTL1			((subaddr_t)0x001d)
#define	EXTSDDR_CTL			((subaddr_t)0x001e)
#define	EXTSDDR_DAT			((subaddr_t)0x001f)
//10G Registers
#define	VS10GLNALIGNACODE_P	((subaddr_t)0x8003)
#define	VS10GLNALIGNACODE_N	((subaddr_t)0x8004)
#define	MCAUTO_CTL			((subaddr_t)0x8021)
#define	DSTONCHAR_CTL		((subaddr_t)0x802a)
#define	DSTOFFCHAR_CTL		((subaddr_t)0x802b)
#define	DSTSTUFFCHAR_CTL	((subaddr_t)0x802c)
#define	DSRONCHAR_CTL		((subaddr_t)0x802d)
#define	DSROFFCHAR_CTL		((subaddr_t)0x802e)
#define	DSRSTUFFCHAR_CTL	((subaddr_t)0x802e)
#define	LATENCYMEASURE_CTL	((subaddr_t)0x8040)
#define	LATENCY_CNT2		((subaddr_t)0x8041)
#define	LATENCY_CNT1		((subaddr_t)0x8042)
#define	TRIGGERLOAD_CTL		((subaddr_t)0x8100)
#define	TRIGGEREN_CTL		((subaddr_t)0x8101)



/* PMA/PMD Registers */
#define	PAPDADDR			((unsigned char)0x01)

#define	PMA_CTL1			((subaddr_t)0x0000)
#define	PMA_STU1			((subaddr_t)0x0001)
#define	PMADEV_ID1			((subaddr_t)0x0002)
#define	PMADEV_ID2			((subaddr_t)0x0003)
#define	PMASPDABLT			((subaddr_t)0x0004)
#define	PMADEVPKG1			((subaddr_t)0x0005)
#define	PMADEVPKG2			((subaddr_t)0x0006)
#define	PMA_STU2			((subaddr_t)0x0008)
#define	PMARXSGLDET_STU		((subaddr_t)0x000a)
#define	PMAEXTABLT_CTL		((subaddr_t)0x000b)
#define	LTTRAIN_CTL			((subaddr_t)0x0096)
#define	LTTRAIN_STU			((subaddr_t)0x0097)
#define	LTLINKPTN_CTL		((subaddr_t)0x0098)
#define	LTLINKPTN_STU		((subaddr_t)0x0099)
#define	LTLOCALDEV_CTL		((subaddr_t)0x009a)
#define	LTLOCALDEV_STU		((subaddr_t)0x009b)
#define	KX_STU				((subaddr_t)0x00a1)
#define	KRFECABLT			((subaddr_t)0x00aa)
#define	KRFEC_CTL			((subaddr_t)0x00ab)
#define	KRFECC_CNT1			((subaddr_t)0x00ac)
#define	KRFECC_CNT2			((subaddr_t)0x00ad)
#define	KRFEUC_CNT1			((subaddr_t)0x00ae)
#define	KRFEUC_CNT2			((subaddr_t)0x00af)

#define	KRVSFIFO_CTL1		((subaddr_t)0x8001)
#define	KRVSTPGEN_CTL		((subaddr_t)0x8002)
#define	KRVSTPVER_CTL		((subaddr_t)0x8003)
#define	KRVSCTCERRCODE_LN0	((subaddr_t)0x8005)
#define	KRVSCTCERRCODE_LN1	((subaddr_t)0x8006)
#define	KRVSCTCERRCODE_LN2	((subaddr_t)0x8007)
#define	KRVSCTCERRCODE_LN3	((subaddr_t)0x8008)
#define	KRVSLN0EOPERR_CNT	((subaddr_t)0x8010)
#define	KRVSLN1EOPERR_CNT	((subaddr_t)0x8011)
#define	KRVSLN2EOPERR_CNT	((subaddr_t)0x8012)
#define	KRVSLN3EOPERR_CNT	((subaddr_t)0x8013)
#define	KRVSTXCTCDROP_CNT	((subaddr_t)0x8014)
#define	KRVSTXCTCIST_CNT	((subaddr_t)0x8015)
#define	KRVSRXCTCDROP_CNT	((subaddr_t)0x8016)
#define	KRVSRXCTCIST_CNT	((subaddr_t)0x8017)
#define	KRVS_STU1			((subaddr_t)0x8018)
#define	KRVSTXCRCJERR_CNT1	((subaddr_t)0x8019)
#define	KRVSTXCRCJERR_CNT2	((subaddr_t)0x801a)
#define	KRVSTXLN0HLMERR_CNT	((subaddr_t)0x801b)
#define	KRVSTXLN1HLMERR_CNT	((subaddr_t)0x801c)
#define	KRVSTXLN2HLMERR_CNT	((subaddr_t)0x801d)
#define	KRVSTXLN3HLMERR_CNT	((subaddr_t)0x801e)
#define	LTVS_CTL2			((subaddr_t)0x9001)


/* Pcs Registers */
#define	PCSADDR				((unsigned char)0x03)

#define	PCS_CTL				((subaddr_t)0x0000)
#define	PCS_STU1			((subaddr_t)0x0001)
#define	PCS_STU2			((subaddr_t)0x0002)
#define	KRPCS_STU1			((subaddr_t)0x0020)
#define	KRPCS_STU2			((subaddr_t)0x0021)
#define	PCSTPSEED_A0		((subaddr_t)0x0022)
#define	PCSTPSEED_A1		((subaddr_t)0x0023)
#define	PCSTPSEED_A2		((subaddr_t)0x0024)
#define	PCSTPSEED_A3		((subaddr_t)0x0025)
#define	PCSTPSEED_B0		((subaddr_t)0x0026)
#define	PCSTPSEED_B1		((subaddr_t)0x0027)
#define	PCSTPSEED_B2		((subaddr_t)0x0028)
#define	PCSTPSEED_B3		((subaddr_t)0x0029)
#define	PCSTP_CTL			((subaddr_t)0x002a)
#define	PCSTPERR_CNT		((subaddr_t)0x002b)
#define	PCSVS_CTL			((subaddr_t)0x8000)
#define	PCSVS_STU			((subaddr_t)0x8001)

/* Auto_Negotitation */
#define	ANADDR				((unsigned char)0x07)

#define	AN_CTL				((subaddr_t)0x0000)
#define	AN_STU				((subaddr_t)0x0001)
#define	ANDEV_PKG			((subaddr_t)0x0005)
#define	AN_AD1				((subaddr_t)0x0010)
#define	AN_AD2				((subaddr_t)0x0011)
#define	AN_AD3				((subaddr_t)0x0012)
#define	ANLP_AD1			((subaddr_t)0x0013)
#define	ANLP_AD2			((subaddr_t)0x0014)
#define	ANLP_AD3			((subaddr_t)0x0015)
#define	ANXNP_TMT1			((subaddr_t)0x0016)
#define	ANXNP_TMT2			((subaddr_t)0x0017)
#define	ANXNP_TMT3			((subaddr_t)0x0018)
#define	ANLPXNP_ABLT1		((subaddr_t)0x0019)
#define	ANLPXNP_ABLT2		((subaddr_t)0x001a)
#define	ANLPXNP_ABLT3		((subaddr_t)0x001b)
#define	ANBP_STU			((subaddr_t)0x0030)


#define	PHYCH1	0x0000
#define	PHYCH2	0x0001


void TlkValueWrite(unsigned int phyadd, unsigned int devadd, unsigned int  regadd, unsigned int value);
unsigned short int TlkValueRead(unsigned int phyadd, unsigned int devadd, unsigned int  regadd);
unsigned short int TlkValueIncRead(unsigned int phyadd, unsigned int devadd, unsigned int  regadd);


void TlkSwReset(unsigned int phyadd);
void TlkHwReset(void);

void MannualModeConf(unsigned char chan, unsigned char isauto, void (*pFunction)(void));
void Read_status(int chan);









#endif  // end tlk10232.h
