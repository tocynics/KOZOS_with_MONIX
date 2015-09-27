/****************************************/
/* CoreOS/Copyleft Yukio Mituiwa,2003	*/
/*					*/
/*  2003/12/12 first release		*/
/*					*/
/****************************************/
#define DATA0_WR    0x07   // (Arm+Enable+tranmist to Host+DATA0)
#define DATA1_WR    0x47   // (Arm+Enable+tranmist to Host on DATA1)
#define ZDATA0_WR   0x05   // (Arm+Transaction Ignored+tranmist to Host+DATA0)
#define ZDATA1_WR   0x45   // (Arm+Transaction Ignored+tranmist to Host+DATA1)
#define DATA0_RD    0x03   // (Arm+Enable+received from Host+DATA0)
#define DATA1_RD    0x43   // (Arm+Enable+received from Host+DATA1)

#define PID_SETUP   0xd0
#define PID_SOF     0x50
#define PID_IN      0x90
#define PID_OUT     0x10

#define SL811H_HOSTCTLREG        0
#define SL811H_BUFADDRREG        1
#define SL811H_BUFLNTHREG        2
#define SL811H_PKTSTATREG        3	/* read */
#define SL811H_PIDEPREG          3	/* write */
#define SL811H_XFERCNTREG        4	/* read */
#define SL811H_DEVADDRREG        4	/* write */

#define SL811H_HOSTCTLREG_A      0
#define SL811H_BUFADDRREG_A      1
#define SL811H_BUFLNTHREG_A      2
#define SL811H_PKTSTATREG_A      3	/* read */
#define SL811H_PIDEPREG_A        3	/* write */
#define SL811H_XFERCNTREG_A      4	/* read */
#define SL811H_DEVADDRREG_A      4	/* write */

#define SL811H_HOSTCTLREG_B      8
#define SL811H_BUFADDRREG_B      9
#define SL811H_BUFLNTHREG_B      10
#define SL811H_PKTSTATREG_B      11	/* read */
#define SL811H_PIDEPREG_B        11	/* write */
#define SL811H_XFERCNTREG_B      12	/* read */
#define SL811H_DEVADDRREG_B      12	/* write */

#define SL811H_CTLREG1           5
#define SL811H_INTENBLREG        6
#define SL811H_INTSTATREG        13	/* write clears bitwise */
#define SL811H_HWREVREG          14	/* read */
#define SL811H_SOFLOWREG         14	/* write */
#define SL811H_SOFTMRREG         15	/* read */
#define SL811H_CTLREG2           15	/* write */
#define SL811H_DATA_START        16

/* Host control register bits (addr 0) */

#define SL811H_HCTLMASK_ARM      1
#define SL811H_HCTLMASK_ENBLEP   2
#define SL811H_HCTLMASK_WRITE    4
#define SL811H_HCTLMASK_ISOCH    0x10
#define SL811H_HCTLMASK_AFTERSOF 0x20
#define SL811H_HCTLMASK_SEQ      0x40
#define SL811H_HCTLMASK_PREAMBLE 0x80

/* Packet status register bits (addr 3) */

#define SL811H_STATMASK_ACK      1
#define SL811H_STATMASK_ERROR    2
#define SL811H_STATMASK_TMOUT    4
#define SL811H_STATMASK_SEQ      8
#define SL811H_STATMASK_SETUP    0x10
#define SL811H_STATMASK_OVF      0x20
#define SL811H_STATMASK_NAK      0x40
#define SL811H_STATMASK_STALL    0x80

/* Control register 1 bits (addr 5) */
#define SL811H_CTL1MASK_ENASOF	1
#define SL811H_CTL1MASK_NOTXEOF2 4
#define SL811H_CTL1MASK_DSTATE   0x18
#define SL811H_CTL1MASK_LOWSPD   0x20
#define SL811H_CTL1MASK_FULLSPD  0x00
#define SL811H_CTL1MASK_SUSPEND  0x40
#define SL811H_CTL1MASK_CLK12    0x80

#define SL811H_CTL1VAL_RESET     8

/* Interrut enable (addr 6) and interrupt status register bits (addr 0xD) */

#define SL811H_INTMASK_XFERDONE_A  1
#define SL811H_INTMASK_XFERDONE_B  2
#define SL811H_INTMASK_SOFINTR   0x10
#define SL811H_INTMASK_INSRMV    0x20
#define SL811H_INTMASK_USBRESET  0x40
#define SL811H_INTMASK_NODEVICE  0x40
#define SL811H_INTMASK_DSTATE    0x80	/* only in status reg */
#define SL811H_INTMASK_DATAPLUS  0x80

/* HW rev and SOF lo register bits (addr 0xE) */

#define SL811H_HWRMASK_HWREV     0xF0
#define	SL811H_SOFLOW_1MS        0xE0

/* SOF counter and control reg 2 (addr 0xF) */

#define SL811H_CTL2MASK_SOFHI    0x3F
#define SL811H_CTL2MASK_DSWAP    0x40
#define SL811H_CTL2MASK_HOSTMODE 0x80

#define	SL811H_SOFHIGH_1MS       0x2E
