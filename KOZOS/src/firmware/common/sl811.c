/****************************************/
/* MES/Copyleft Yukio Mituiwa,2005	*/
/*					*/
/*  2006/6/16 first release		*/
/*					*/
/****************************************/
#ifdef _KERNEL_
  #include "mes.h"
  #include "conf.h"
  #define device_main(arg1,arg2) init_sl811(void)
  extern BUSConf usb_config[];
#endif
#include "usb.h"
#include "sl811.h"
#include "reg3069.h"
#include "lib.h"

//#if defined(__H8300H__)
#if 1
#define USB_WAIT	100
#define USB_LOW		1
#define USB_FULL	2
#define WAITCONST 1
#define USBNUM 8
#define SLNUM 2

#define USB_NO_ERROR	0
#define USB_WRITE_ERROR	1
#define USB_DEV_DOWN	2

#define DEV_INFO 0x01
#define GET_MAC 0x02
#define SET_HANDLE 0x03
#define HANDLE_ARG 0x04
#define MAXPACKET 0x05
#define DISK_CACHE 0x06
#define MEDIA_SIZE 0x07
#define DEV_RESET 0x08
#define DEV_INTERVAL 0x09
#define DEV_START 0x0a
#define DEV_STOP 0x0b
#define DEV_FLUSH 0x0c
#define DEV_BLOCK 0x0d
#define READ_BURST 0x0e
#define SCI_FREQ 0x101
#define SCI_SPEED 0x102
#define SCI_POLL 0x103
#define SCI_CONTINUE 0x104
#define WI_GETBSSID 0x201
#define WI_SETWEPKEY 0x202
#define WI_GETWEPKEY 0x203
#define WI_SETWEP64 0x204
#define WI_SETWEP128 0x205
#define WI_WEPON 0x206
#define WI_SETSSID 0x207
#define WI_GETSSID 0x208
#define WI_PMGTON 0x209
#define WI_SETWPA 0x20a
#define WI_USBDOWN 0x20b
#define MMC_CSD 0x301
#define MMC_CID 0x302
#define USB_EPSIZE 0x401
#define USB_INFO 0x402
#define USB_SETCONF 0x403
#define USB_SETUP 0x404
#define USB_RESET 0x405
#define USB_GETSTATUS 0x406
#define USB_TIMEOUT 0x407
#define USB_INTERRUPT 0x408
#define USB_VENDOR 0x409
#define USB_PRODUCT 0x40a
#define USB_SET_EPSIZE 0x40b
#define USB_INTR_EP 0x40c
#define USB_FAST_RETRY 0x40d
#define USB_WRITE_TIMEOUT 0x40e
#define USB_ERROR_STATUS 0x40f
#define LCD_CLEAR 0x501
#define LCD_SETFONT 0x502

#define setup_data(num, data) write_data(num, data, 8, 1)
#define status_data(num) write_data(num, 0, 0, 0)

#define DEFAULT_TIMEOUT	20
#define DEFAULT_WRITE_TIMEOUT	6
#define SL811_WAIT	12

#define	bswap16(x)	((((x) & 0xff00) >>  8) | (((x) & 0x00ff) <<  8))
#define	htole16(x)	bswap16((x))
#define	le16toh(x)	bswap16((x))
#define USB_EP		1
#define USB_TOGGLE	2
#define USB_HDRSIZ	2

static char SL811Read(int num, char offset);
static void SL811Write(int num, char offset, char data);
static void SL811BufRead(int num, short offset, char* buf, short size);
static void SL811BufWrite(int num, short offset, char* buf, short size);
static int regTest(int num);
static int USBStop(int num);
static void USBReset(int num);
static int USBStart(int num);
static int write_data(int num, char* data, int size, int flag);
static int read_data(int num, char* data, int size);
static int intr_data(int num, char* data, int size);
static int get_info(int minor, char* data);
static int dev_init1(int minor);
static void dev_init2(int minor);
static int open_sl811(int minor, int opt);
static int close_sl811(int minor);
static int write_sl811(int minor, char *buffer, int size);
static int read_sl811(int minor, char *buffer, int size);
static int read_sl811(int minor, char *buffer, int size);
static int seek_sl811(int minor, int position);
static int ioctl_sl811(int minor, int data, int op);

typedef struct _get_desc_struct {
	short num;
	short type;
	char *data;
	int datasize;
	int epsize;
	char index;
	char cDummy;
	short lang;
} GET_DESCTYPE;

typedef struct {
	short	epsize;
	short	retry, w_retry;
	short	vendor, product;
	short	error;
	char	intr;
	char	ep;
	char	rd_cmd;
	char	wr_cmd;
	char	status;
	char	pad;
	char	dummy[2];
} USBInfo;

typedef struct {
	volatile char	*sl811data, *sl811addr;
	char		speed;
	char		pad;
} SL811Info;

typedef struct {
	int (*open_dev)(int, int);
	int (*close_dev)(int);
	int (*write_dev)(int, char*, int);
	int (*read_dev)(int, char*, int);
	int (*seek_dev)(int, int);
	int (*ioctl_dev)(int, int, int);
	void (*poll_dev)();
	char name[8];
} Functions;


#define MAXCONF		128
static USBInfo	*usb;
static SL811Info sl811[4];
static char	devdesc[8] = {TRANS_TO_HOST, 6, 0, 1, 0, 0, 64, 0};
static char	confdesc[8] = {TRANS_TO_HOST, 6, 0, 2, 0, 0, MAXCONF, 0};
static char	setaddr[8] = {TRANS_TO_DEV, 5, 0, 0, 0, 0, 0, 0};
static char	setconf[8] = {TRANS_TO_DEV, 9, 1, 0, 0, 0, 0, 0};
static char	clrfeature[8] = {TRANS_TO_DEV + TRANS_TO_EP,1,0,0,0,0,0,0};

static int __mulsi3(int a, int b)
{
	int res = 0;
	int i;
	if (b > 0) {
		for (i = 0; i < b; i++) {
			res += a;
		}
	} else if (b < 0) {
		for (i = 0; i < -b; i++) {
			res -= a;
		}
	} else {
		return 0;
	}
	return res;
}

static char SL811Read(int num, char offset) {
	char data;

	*(sl811[num].sl811addr) = offset;
	data = *(sl811[num].sl811data);
	return data;
}

static void SL811Write(int num, char offset, char data){
	*(sl811[num].sl811addr) = offset;
	*(sl811[num].sl811data) = data;
}

static void SL811BufRead(int num, short offset, char* buf, short size){
	if(size <= 0) return;
	*(sl811[num].sl811addr) = (char)offset;
	while(size--) *buf++ = *(sl811[num].sl811data);
}

static void SL811BufWrite(int num, short offset, char* buf, short size) {
	if(size <= 0) return;
	*(sl811[num].sl811addr) = (char)offset;
	while(size--) {
		*(sl811[num].sl811data) = *buf;
		buf++;
	}
}

static int regTest(int num){
	int		i, result;
	unsigned char	data, buf[256];

	result = 0;
	for(i = 16;i < 256; i++) {
		buf[i] = (char)SL811Read(num, i);
		SL811Write(num, i, i);
	}
	for(i = 16;i < 256; i++) {
		data = SL811Read(num, i);
		if(data != i) {
			result = -1;
		}
	}
	for(i = 16;i < 256;i++) SL811Write(num, i, buf[i]);
	return result;
}

static int USBStop(int num) {
	SL811Write(num, SL811H_HOSTCTLREG_A, 0x00);
	SL811Write(num, SL811H_HOSTCTLREG_B, 0x00);
	SL811Write(num, SL811H_CTLREG1, 0);
	return 0;
}

static void USBReset(int num) {
	char	tmp;

	tmp = SL811Read(num, SL811H_CTLREG1);
	SL811Write(num, SL811H_CTLREG1, tmp | SL811H_CTL1VAL_RESET);
	waitms(500);
	SL811Write(num, SL811H_CTLREG1, tmp | SL811H_CTL1MASK_DSTATE );
	waitms(300);
	SL811Write(num, SL811H_CTLREG1, tmp | SL811H_CTL1VAL_RESET);
	waitms(20);
	SL811Write(num, SL811H_CTLREG1, tmp);
}

static int USBStart(int num) {
	int	status;

	SL811Write(num, SL811H_CTLREG2, 0xae);
	SL811Write(num, SL811H_CTLREG1, SL811H_CTL1VAL_RESET);	// reset USB
	waitms(250);						// 20ms
	SL811Write(num, SL811H_CTLREG1, 0);			// remove SE0

	for(status = 0;status < 100;status++) {
		SL811Write(num, SL811H_INTSTATREG, 0xff); // clear all interrupt bits
	}
	status = SL811Read(num, SL811H_INTSTATREG);
	if(status & 0x40){ 				// Check if device is removed
		sl811[num].speed = 0;			// None
		SL811Write(num, SL811H_INTENBLREG,
			   SL811H_INTMASK_XFERDONE_A | SL811H_INTMASK_XFERDONE_B |
			   SL811H_INTMASK_SOFINTR | SL811H_INTMASK_INSRMV);
		return -1;
	}
	SL811Write(num, SL811H_BUFLNTHREG_B, 0);	//zero lenth
	SL811Write(num, SL811H_PIDEPREG_B, PID_SOF);	//send SOF to EP0
	SL811Write(num, SL811H_DEVADDRREG_B, 1);	//address0
	SL811Write(num, SL811H_SOFLOWREG, SL811H_SOFLOW_1MS);
	SL811Write(num, SL811H_CTLREG1, SL811H_CTL1VAL_RESET);
	waitms(400);
	if(!(status & 0x80)) {
		sl811[num].speed = USB_LOW;		// Low
		SL811Write(num, SL811H_CTLREG2, SL811H_CTL2MASK_HOSTMODE + SL811H_CTL2MASK_DSWAP + SL811H_SOFHIGH_1MS);
		SL811Write(num, SL811H_CTLREG1, SL811H_CTL1MASK_ENASOF + SL811H_CTL1MASK_LOWSPD);
		SL811Write(num, SL811H_HOSTCTLREG_B, SL811H_HCTLMASK_ARM);
		for(status = 0;status < 20;status++) {
			SL811Write(num, SL811H_INTSTATREG, 0xff);
		}
	} else {
		sl811[num].speed = USB_FULL;		// Full
		SL811Write(num, SL811H_CTLREG2, SL811H_CTL2MASK_HOSTMODE + SL811H_SOFHIGH_1MS);
		SL811Write(num, SL811H_CTLREG1, SL811H_CTL1MASK_ENASOF + SL811H_CTL1MASK_FULLSPD);
		SL811Write(num, SL811H_HOSTCTLREG_B, SL811H_HCTLMASK_ARM);
		SL811Write(num, SL811H_INTSTATREG, 0xff);
	}
	SL811Write(num, SL811H_INTENBLREG, SL811H_INTMASK_XFERDONE_A | SL811H_INTMASK_XFERDONE_B |
		   SL811H_INTMASK_SOFINTR|SL811H_INTMASK_INSRMV);
	return 0;
}

static int write_data(int num, char *data, int size, int flag) {
	int	remain, wsiz, timovr, slnum;
	char	pktstat, toggle, fin;

	toggle = 0;
	fin = 0;
	slnum = num / USBNUM;
	remain = size;
	SL811Write(slnum, SL811H_BUFADDRREG_A, 0x10);
	SL811Write(slnum, SL811H_DEVADDRREG_A, num % USBNUM);
	SL811Write(slnum, SL811H_PIDEPREG_A, (flag == 1) ? PID_SETUP : (PID_OUT | usb[num].ep));
	SL811Write(slnum, SL811H_BUFADDRREG_B, 0x10 + usb[num].epsize);
	SL811Write(slnum, SL811H_DEVADDRREG_B, num % USBNUM);
	SL811Write(slnum, SL811H_PIDEPREG_B, (flag == 1) ? PID_SETUP : (PID_OUT | usb[num].ep));

	wsiz = (remain > usb[num].epsize) ? usb[num].epsize : remain;
	SL811Write(slnum, SL811H_BUFLNTHREG_A, wsiz);
	if(data != 0 && size > 0) SL811BufWrite(slnum, 0x10, &data[size - remain], wsiz);
	remain -= wsiz;
	do {
		wsiz = (remain > usb[num].epsize) ? usb[num].epsize : remain;
		for(timovr = 0;timovr < usb[num].w_retry;timovr++) {
			SL811Write(slnum, SL811H_INTSTATREG, 0xff);
			if(toggle == 0) {
				SL811Write(slnum, SL811H_HOSTCTLREG_A, usb[num].wr_cmd);
				SL811Write(slnum, SL811H_BUFLNTHREG_B, wsiz);
				if(data != 0 && size > 0 && wsiz > 0) {
					SL811BufWrite(slnum, 0x10 + usb[num].epsize, &data[size - remain], wsiz);
				} else {
					waitus(USB_WAIT);
					fin = 1;
				}
				waitus(SL811_WAIT);
				while(SL811Read(slnum, SL811H_INTSTATREG) & SL811H_INTMASK_XFERDONE_A == 0);
				waitus(SL811_WAIT);
				pktstat = SL811Read(slnum, SL811H_PKTSTATREG_A);
			} else {
				SL811Write(slnum, SL811H_HOSTCTLREG_B, usb[num].wr_cmd);
				SL811Write(slnum, SL811H_BUFLNTHREG_A, wsiz);
				if(data != 0 && size > 0 && wsiz > 0) {
					SL811BufWrite(slnum, 0x10, &data[size - remain], wsiz);
				} else {
					waitus(USB_WAIT);
					fin = 1;
				}
				waitus(SL811_WAIT);
				while(SL811Read(slnum, SL811H_INTSTATREG) & SL811H_INTMASK_XFERDONE_B == 0);
				waitus(SL811_WAIT);
				pktstat = SL811Read(slnum, SL811H_PKTSTATREG_B);
			}
			if(pktstat & SL811H_STATMASK_ACK) {
				if(pktstat & SL811H_STATMASK_ERROR) {
					usb[num].error = USB_WRITE_ERROR;
#ifdef _KERNEL_		/* ----- Append 2010/01/26 A.Ohnishi ----- */
					__write(0,0,"e",1);
#endif
					waitms(100);
				}
				break;
			}
			waitus((timovr > 40) ? 500000 : 5000);
		}
		remain -= wsiz;
		if(!(pktstat & SL811H_STATMASK_ACK)) {
			return -1;
		}
		usb[num].wr_cmd = (usb[num].wr_cmd == DATA0_WR) ? DATA1_WR : DATA0_WR;
		toggle = (toggle == 0) ? 1 : 0;
	} while(fin == 0);
	return size;
}

static int read_data(int num, char *data, int size) {
	char	pktstat, epsv, rdsv, wrsv, toggle;
	int	timovr, remain, rsiz, rcnt, xfercnt;
	int	maxretry, minretry;
	int	slnum;
	char	*bufptr;
	int	bufsiz;

	toggle = 0;
	bufptr = 0;
	slnum = num / USBNUM;
	remain = size;
	maxretry = usb[num].retry;
	minretry = (maxretry <= 7) ? maxretry : maxretry / 3;
	if(minretry > 40) minretry = 40;
	SL811Write(slnum, SL811H_DEVADDRREG_A, num % USBNUM);
	SL811Write(slnum, SL811H_PIDEPREG_A, PID_IN | usb[num].ep);
	SL811Write(slnum, SL811H_DEVADDRREG_B, num % USBNUM);
	SL811Write(slnum, SL811H_PIDEPREG_B, PID_IN | usb[num].ep);
	SL811Write(slnum, SL811H_BUFADDRREG_A, 0x10);
	SL811Write(slnum, SL811H_BUFADDRREG_B, 0x10 + usb[num].epsize);
	do {
		rsiz = (remain > usb[num].epsize) ? usb[num].epsize : remain;
		SL811Write(slnum, SL811H_BUFLNTHREG_A, rsiz);
		SL811Write(slnum, SL811H_BUFLNTHREG_B, rsiz);
		for(timovr = 0;timovr < maxretry;timovr++) {
			SL811Write(slnum, SL811H_INTSTATREG, 0xff);
			if(toggle == 0) {
				SL811Write(slnum, SL811H_HOSTCTLREG_A, usb[num].rd_cmd);
				if(bufptr) {
					SL811BufRead(slnum, 0x10 + usb[num].epsize, bufptr, bufsiz);
					bufptr = 0;
				} else {
					waitus((timovr < minretry) ? USB_WAIT : 1500);
				}
				waitus(SL811_WAIT);
				while(SL811Read(slnum, SL811H_INTSTATREG) & SL811H_INTMASK_XFERDONE_A == 0);
				waitus(SL811_WAIT);
				pktstat = SL811Read(slnum, SL811H_PKTSTATREG_A);
				xfercnt = (int)SL811Read(slnum, SL811H_XFERCNTREG_A) & 0xff;
			} else {
				SL811Write(slnum, SL811H_HOSTCTLREG_B, usb[num].rd_cmd);
				if(bufptr) {
					SL811BufRead(slnum, 0x10, bufptr, bufsiz);
					bufptr = 0;
				} else {
					waitus((timovr < minretry) ? USB_WAIT : 1500);
				}
				waitus(SL811_WAIT);
				while(SL811Read(slnum, SL811H_INTSTATREG) & SL811H_INTMASK_XFERDONE_B == 0);
				waitus(SL811_WAIT);
				pktstat = SL811Read(slnum, SL811H_PKTSTATREG_B);
				xfercnt = (int)SL811Read(slnum, SL811H_XFERCNTREG_B) & 0xff;
			}
			if(xfercnt == 1 && pktstat & SL811H_STATMASK_SETUP) {
				usb[num].error = USB_DEV_DOWN;
#ifdef _KERNEL_		/* ----- Append 2010/01/26 A.Ohnishi ----- */
				__write(0,0,"X",1);
#endif
				return -1;
			}
			if(pktstat & SL811H_STATMASK_ACK) break;
			if(pktstat & (SL811H_STATMASK_ERROR | SL811H_STATMASK_TMOUT | SL811H_STATMASK_OVF | SL811H_STATMASK_NAK)) {
				waitus(500);
				continue;
			}
			if(pktstat & SL811H_STATMASK_STALL) {
				epsv = usb[num].ep;
				wrsv = usb[num].wr_cmd;
				rdsv = usb[num].rd_cmd;
				clrfeature[4] = TRANS_TO_HOST | usb[num].ep;
				usb[num].wr_cmd = DATA0_WR;
				usb[num].rd_cmd = DATA1_RD;
				setup_data(num, clrfeature);
				usb[num].ep = 0;
				if(read_data(num, 0, 0) > 0) break;
				usb[num].ep = epsv;
				usb[num].wr_cmd = wrsv;
				usb[num].rd_cmd = rdsv;
				usb[num].rd_cmd = (usb[num].rd_cmd == DATA0_RD) ? DATA1_RD : DATA0_RD;
				return -1;
			}
		}
		if(timovr >= maxretry) {
			return -1;
		}
		if(toggle == 0) {
		} else {
			xfercnt = (int)SL811Read(slnum, SL811H_XFERCNTREG_B) & 0xff;
		}
		rcnt = rsiz - xfercnt;
		if(pktstat & SL811H_STATMASK_ACK && data != 0 && size > 0) {
			bufptr = &data[size - remain];
			bufsiz = rsiz;
		}
		remain -= rcnt;
		usb[num].rd_cmd = (usb[num].rd_cmd == DATA0_RD) ? DATA1_RD : DATA0_RD;
		toggle = (toggle == 0) ? 1 : 0;
	} while(remain > 0 && rcnt == rsiz);
	if(bufptr) {
		if(toggle == 0) {
			SL811BufRead(slnum, 0x10 + usb[num].epsize, bufptr, bufsiz);
		} else {
			SL811BufRead(slnum, 0x10, bufptr, bufsiz);
		}
	}
	return size - remain;
}

static int intr_data(int num, char *data, int size) {
	char	pktstat;
	int	rcnt, slnum;

	slnum = num / USBNUM;
	SL811Write(slnum, SL811H_DEVADDRREG_A, num % USBNUM);
	SL811Write(slnum, SL811H_PIDEPREG_A, PID_IN | usb[num].ep);
	SL811Write(slnum, SL811H_BUFADDRREG_A, 0x10);
	SL811Write(slnum, SL811H_BUFLNTHREG_A, size);
	SL811Write(slnum, SL811H_HOSTCTLREG_A, usb[num].rd_cmd);
	waitus(30);
	while(SL811Read(slnum, SL811H_INTSTATREG) & SL811H_INTMASK_XFERDONE_A == 0);
	pktstat = SL811Read(slnum, SL811H_PKTSTATREG_A);
	rcnt = size - (int)SL811Read(slnum, SL811H_XFERCNTREG_A);
	if(pktstat & SL811H_STATMASK_ACK) {
		SL811BufRead(slnum, 0x10, data, size);
		usb[num].rd_cmd = (usb[num].rd_cmd == DATA0_RD) ? DATA1_RD : DATA0_RD;
	} else {
		rcnt = 0;
	}
	return rcnt;
}

static int get_info(int minor, char *data) {
	usb[minor].wr_cmd = DATA0_WR;
	usb[minor].rd_cmd = DATA1_RD;
	if(setup_data(minor, confdesc) == -1) return -1;
	if(read_data(minor, data, MAXCONF) == -1) return -1;
	if(status_data(minor) == -1) return -1;
	return data[2];
}

static int dev_init1(int minor) {
	char	retry;
	int	sl_minor;
	DevDesc	devinfo;

	sl_minor = minor / USBNUM;
	usb[sl_minor * USBNUM].wr_cmd = DATA0_WR;
	setup_data(sl_minor * USBNUM, devdesc);
	for(retry = 0;retry < 8;retry++) {
		usb[sl_minor * USBNUM].rd_cmd = DATA1_RD;
		if(read_data(sl_minor * USBNUM, (char*)&devinfo, 8) != -1) break;
		waitms(200);
		usb[sl_minor * USBNUM].wr_cmd = DATA0_WR;
		setup_data(sl_minor * USBNUM, devdesc);
	}
	if(retry == 8) return -1;
	if(status_data(sl_minor * USBNUM) == -1) return -1;
	setaddr[2] = minor % USBNUM;
	usb[sl_minor * USBNUM].wr_cmd = DATA0_WR;
	usb[sl_minor * USBNUM].rd_cmd = DATA1_RD;
	if(setup_data(sl_minor * USBNUM, setaddr) == -1) return -1;
	if(read_data(sl_minor * USBNUM, 0, 0) == -1) return -1;
	usb[minor].epsize = devinfo.bMaxPacketSize0;
	usb[minor].ep = 0;
	return 0;
}

static void dev_init2(int minor) {
	DevDesc	devinfo;

	usb[minor].retry = DEFAULT_TIMEOUT;
	usb[minor].wr_cmd = DATA0_WR;
	setup_data(minor, devdesc);
	usb[minor].rd_cmd = DATA1_RD;
	read_data(minor, (char*)&devinfo, sizeof(DevDesc));
	usb[minor].vendor = le16toh(devinfo.idVendor);
	usb[minor].product = le16toh(devinfo.idProduct);
	status_data(minor);
}

static int open_sl811(int minor, int option) {
	int	num, sl_minor;

	sl_minor = minor / USBNUM;
	if(minor >= USBNUM * SLNUM) return -1;
	if(sl811[sl_minor].sl811addr == 0 && sl811[sl_minor].sl811data == 0) return -1;
	if(minor == sl_minor * USBNUM) {
		for(num = 0;num < USBNUM;num++) {
			usb[minor + num].ep = 0;
			usb[minor + num].intr = 0;
			usb[minor + num].epsize = 0;
			usb[minor + num].error = 0;
			usb[minor + num].retry = DEFAULT_TIMEOUT;
			usb[minor + num].w_retry = DEFAULT_WRITE_TIMEOUT;
		}
		if(regTest(sl_minor) != -1) {
			if(USBStart(sl_minor) == 0) usb[minor].epsize = 8;
		}
		return -1;
	}
	if(usb[sl_minor * USBNUM].epsize == 0) return -1;
	if(option == -1) return 0;
	if(dev_init1(minor) == -1) return -1;
	dev_init2(minor);
	return 0;
}

static int close_sl811(int minor) {
	if(minor == 0 || minor >= USBNUM * SLNUM) return -1;
	if(usb[minor].epsize == 0) return -1;
	usb[minor].ep = 0;
	usb[minor].intr = 0;
	usb[minor].epsize = 0;
	usb[minor].retry = DEFAULT_TIMEOUT;
	if(minor == 1) USBStop(minor / USBNUM);
	return 0;
}

static int write_sl811(int minor, char *buffer, int size) {
	if(minor == 0 || minor >= USBNUM * SLNUM) return -1;
	if(usb[minor].epsize == 0) return -1;
	if(usb[minor].error == USB_DEV_DOWN) return -1;
	return write_data(minor, buffer, size, 0);
}

static int read_sl811(int minor, char *buffer, int size) {
	if(minor == 0 || minor >= USBNUM * SLNUM) return -1;
	if(usb[minor].epsize == 0) return -1;
	if(usb[minor].error == USB_DEV_DOWN) return -1;
	return read_data(minor, buffer, size);
}

static int seek_sl811(int minor, int position) {
	if(minor == 0 || minor >= USBNUM * SLNUM) return -1;
	if(usb[minor].epsize == 0) return -1;
	usb[minor].ep = position;
}

static int ioctl_sl811(int minor, int data, int op) {
	int	ret;
	char	epsv, buf[8];

	if(minor == 0 || minor >= USBNUM * SLNUM){
		puts("minor?\n");
		return -1;
	}
	if(usb[minor].epsize == 0){
		puts("epsize?\n");
		return -1;
	}
	ret = 0;
	switch(op) {
	case DEV_INFO:
		break;
	case DEV_RESET:
		USBStart(minor / USBNUM);
		dev_init1(minor);
		dev_init2(minor);
		break;
	case DEV_STOP:
		ret = USBStop(minor / USBNUM);
		break;
	case USB_EPSIZE:
		ret = usb[minor].epsize;
		break;
	case USB_SET_EPSIZE:
		usb[minor].epsize = data;
		break;
	case USB_SETUP:
		usb[minor].wr_cmd = DATA0_WR;
		usb[minor].rd_cmd = DATA1_RD;
		if(setup_data(minor, (char*)data) == -1) return -1;
		break;
	case USB_INFO:
		ret = get_info(minor, (char*)data);
		break;
	case USB_SETCONF:
		ret = -1;
		usb[minor].wr_cmd = DATA0_WR;
		usb[minor].rd_cmd = DATA1_RD;
		setconf[2] = data;
		if(setup_data(minor, setconf) == -1) break;
		if(read_data(minor, 0, 0) == -1) break;
		usb[minor].wr_cmd = DATA0_WR;
		ret = 0;
		break;
	case USB_RESET:
		USBReset(minor / USBNUM);
		break;
	case USB_TIMEOUT:
		usb[minor].retry = (data <= 0) ? DEFAULT_TIMEOUT : data;
		break;
	case USB_WRITE_TIMEOUT:
		usb[minor].w_retry = (data <= 0) ? DEFAULT_WRITE_TIMEOUT : data;
		break;
	case USB_INTR_EP:
		usb[minor].intr = data;
		break;
	case USB_INTERRUPT:
		if(usb[minor].intr != 0) {
			epsv = usb[minor].ep;
			usb[minor].ep = usb[minor].intr;
			ret = intr_data(minor, (char*)data, 8);
			usb[minor].ep = epsv;
		}
		break;
	case USB_ERROR_STATUS:
		ret = usb[minor].error;
		usb[minor].error = USB_NO_ERROR;
		break;
	case USB_GETSTATUS:
		break;
	case USB_VENDOR:
		ret = usb[minor].vendor;
		break;
	case USB_PRODUCT:
		ret = usb[minor].product;
		break;
	default:
		ret = -1;
	}
	return ret;
}

static Functions func;

void get_desc(GET_DESCTYPE* pDesc) {
	SetupPKG	command;
	char		toggle, buf[3];
	int		i, remain, size, offset;

	command.bmRequest = TRANS_TO_HOST;
	command.bRequest = GET_DESCRIPTOR;
	command.wValue = htole16((pDesc->type << 8) | pDesc->index);
	command.wIndex = pDesc->lang;
	command.wLength = htole16(pDesc->datasize);
	ioctl_sl811(pDesc->num, (int)&command, USB_SETUP);
	offset = 0;
	toggle = 1;
	buf[USB_TOGGLE] = 1;
	for(remain = pDesc->datasize; remain > 0; remain -= size) {
		size = (remain > pDesc->epsize) ? pDesc->epsize : remain;
		if(size < USB_HDRSIZ) {
			buf[USB_EP] = 0;
			read_sl811(pDesc->num, buf, size);
			pDesc->data[offset] = buf[0];
		} else {
			pDesc->data[offset + USB_EP] = 0;
			pDesc->data[offset + USB_TOGGLE] = toggle;
			read_sl811(pDesc->num, &pDesc->data[offset], size);
		}
		if((offset == 0) && (pDesc->data[1] != CONFIG_TYPE)){
			remain = (int)pDesc->data[0] & 0xff;
		}
		toggle = (toggle == 0) ? 1 : 0;
		offset += size;		
	}
	buf[USB_EP] = 0;
	buf[USB_TOGGLE] = 1;
	write_sl811(pDesc->num, buf, 2);
}

int usb_main() {
	int		num = 1;
	int	i, ep, cfgsiz, offset;
	char		buffer[128], *p;
	DevDesc		device;
	CfgDesc		config;
	StrDesc		string;
	IntfDesc	interface;
	EPDesc		endpoint[MAX_EP];
	int		epsize, epnum;
	short		vendor, product;
	char		class, subclass, protocol;
	GET_DESCTYPE	desctype;

	/* BUS  initialize */
	puts("usb_main...\n");
	P1DDR = 0xff;	/* A0-7    is enable */
	P2DDR = 0xff;	/* A8-15   is enable */
	P5DDR = 0x01;	/* A16     is enable */
	P8DDR = 0x0e;	/* CS1-3 is enable */
//	sio_init();
//	sio_speed(57600, 25);
	num = 1;
	sl811[num].sl811addr = (volatile char *)0x200001;
	sl811[num].sl811data = (volatile char *)0x200003;
	sl811[num].pad = 0;
	
	if(regTest(num) != 0) {
		puts("USB not found!\n");
		for(;;);
	}
	ioctl_sl811(num, 2, DEV_RESET);
	epsize = ioctl_sl811(num, 2, USB_RESET);
	if(epsize == -1) {
		puts("Device not found!\n");
		for(;;);
	}
	puts("Size of endpoint is [byte]");
	printhex(epsize, 2 ,0);
	puts("\n");

	desctype.num = num;
	desctype.type = DEVICE_TYPE;
	desctype.data = (char*)&device;
	desctype.datasize = sizeof(DevDesc);
	desctype.epsize = epsize;
	desctype.index = 0;
	desctype.lang = 0;

	get_desc(&desctype);
	vendor = le16toh(device.idVendor);
	product = le16toh(device.idProduct);
	puts("\nDevice desc : ");
	p = (char*)&device;
	for(i = 0;i < sizeof(DevDesc);i++){
		printhex(p[i] & 0xFF, 2, 0);
		//print("%02x ", (int)p[i] & 0xff);
	}
	desctype.num = num;
	desctype.type = CONFIG_TYPE;
	desctype.data = (char*)&config;
	desctype.datasize = sizeof(CfgDesc);
	desctype.epsize = epsize;
	desctype.index = 0;
	desctype.lang = 0;

	get_desc(&desctype);
	cfgsiz = le16toh(config.wLength);
	if(cfgsiz > 128) cfgsiz = 128;
	puts("\nConfiguration desc : ");
	p = (char*)&config;
	for(i = 0;i < sizeof(CfgDesc);i++){
		printhex(p[i] & 0xFF, 2, 0);
		//print("%02x ", (int)p[i] & 0xff);
	}
	if(device.iProduct != 0) {

		desctype.num = num;
		desctype.type = STRING_TYPE;
		desctype.data = (char*)&string;
		desctype.datasize = sizeof(StrDesc);
		desctype.epsize = epsize;
		desctype.index = 0;
		desctype.lang = 0;

		get_desc(&desctype);
		puts("\nString desc : ");
		p = (char*)&string;
		for(i = 0;i < sizeof(StrDesc);i++){
			printhex(p[i] & 0xFF, 2, 0);
			//print("%02x ", (int)p[i] & 0xff);
		}

		desctype.num = num;
		desctype.type = STRING_TYPE;
		desctype.data = buffer;
		desctype.datasize = 128;
		desctype.epsize = epsize;
		desctype.index = device.iProduct;
		desctype.lang = le16toh(string.wLang);

		get_desc(&desctype);
		puts("\n[");
		p = (char*)&string;
		for(i = 2;i < ((int)buffer[0] & 0xff);i+=2){
			putc(buffer[i]);
			//print("%c", buffer[i]);
		}
		puts("]");
	}

		desctype.num = num;
		desctype.type = CONFIG_TYPE;
		desctype.data = buffer;
		desctype.datasize = cfgsiz;
		desctype.epsize = epsize;
		desctype.index = 0;
		desctype.lang = 0;

//	get_desc(num, CONFIG_TYPE, buffer, cfgsiz, epsize, 0, 0);
	get_desc(&desctype);
	offset = sizeof(CfgDesc);
	memcpy((char*)&interface, &buffer[offset], sizeof(IntfDesc));
	puts("\nInterface desc : ");
	p = (char*)&interface;
	for(i = 0;i < sizeof(IntfDesc);i++){
		printhex(p[i] & 0xFF, 2, 0);
		//print("%02x ", (int)p[i] & 0xff);
	}
	offset += sizeof(IntfDesc);
	epnum = interface.bEndPoints;
	class = interface.iClass;
	subclass = interface.iSub;
	protocol = interface.iProto;
	for(ep = 0;ep < epnum;ep++) {
		if(ep == MAX_EP) break;
		memcpy((char*)&endpoint[ep], &buffer[offset], sizeof(EPDesc));
		offset += sizeof(EPDesc);
		puts("\nEndpoint desc() : ");
		printhex(ep, 2, 0);
		puts("\nEndpoint desc() : ");
		p = (char*)&endpoint[ep];
		for(i = 0;i < sizeof(EPDesc);i++){
			printhex(p[i] & 0xFF, 2, 0);
			//print("%02x ", (int)p[i] & 0xff);
		}
	}
	puts("\nEndpoint number is ");
	printhex(epnum, 2, 0);
	puts("\n");
	puts("\nVendor");
	printhex(vendor, 2, 0);
	puts("\nproduct");
	printhex(product, 2, 0);
	puts("\nClass");
	printhex(class&0xFF, 2, 0);
	puts("\nSub");
	printhex(subclass&0xFF, 2, 0);
	puts("\nProto");
	printhex(protocol & 0xFF, 2, 0);

	//print("Vendor %d product %d Class %d Sub %d Proto %d\n",
	//	(int)vendor, (int)product, (int)class & 0xff, (int)subclass & 0xff, (int)protocol & 0xff);
//	for(;;);
}
#if 0
int device_main(int argc, char **argv) {
	unsigned int	num, n;
	char		*buf;

	buf = malloc(sizeof(USBInfo) * USBNUM * SLNUM + sizeof(SL811Info) * SLNUM);
	usb = (USBInfo*)buf;
	sl811 = (SL811Info*)&buf[sizeof(USBInfo) * USBNUM * SLNUM];
#ifndef _KERNEL_
	num = 0;
	for(n = 1;n < argc;n++) {
		if(memcmp(argv[n], "usb", 3) == 0) {
			num = stoi(&argv[n][3]);
			if(num >= SLNUM) num = 0;
		} else if(memcmp(argv[n], "ar=", 3) == 0) {
			sl811[num].sl811addr = (volatile char *)stoh(&argv[n][3]);
		} else if(memcmp(argv[n], "dr=", 3) == 0) {
			sl811[num].sl811data = (volatile char *)stoh(&argv[n][3]);
		}
	}
#endif
	for(num = 0;num < SLNUM;num++) {
#ifdef _KERNEL_
		sl811[num].sl811addr = (volatile char *)usb_config[num].addr_reg;
		sl811[num].sl811data = (volatile char *)usb_config[num].data_reg;
#endif
		set_buswidth((int)sl811[num].sl811addr, 8, 64);
	}
	for(num = 0;num < USBNUM * SLNUM;num++) {
		usb[num].ep = 0;
		usb[num].intr = 0;
		usb[num].epsize = 0;
		usb[num].retry = DEFAULT_TIMEOUT;
	}
	func.open_dev = open_sl811;
	func.close_dev = close_sl811;
	func.write_dev = write_sl811;
	func.read_dev = read_sl811;
	func.seek_dev = seek_sl811;
	func.ioctl_dev = ioctl_sl811;
	func.poll_dev = 0;
	strcpy(&(func.name[0]), "usb");
	device_return(&func);
}
#endif

#else

#if defined(__H8300H__)
  #define USB_WAIT	20
  #define USB_MIN_WAIT	25
  #define SL811_WAIT	30
#endif
#if defined(__sh2__)
  #define USB_WAIT	12
  #define USB_MIN_WAIT	15
  #define SL811_WAIT	10
#endif
#if defined(__sh3__)
  #define USB_WAIT	20
  #define USB_MIN_WAIT	25
  #define SL811_WAIT	40
#endif

#define USB_LOW		1
#define USB_FULL	2

#define setup_data(num, data) write_data(num, data, 8, 1)
#define status_data(num) write_data(num, 0, 0, 0)

#define DEFAULT_TIMEOUT	20
#define DEFAULT_WRITE_TIMEOUT	6

static char SL811Read(int num, char offset);
static void SL811Write(int num, char offset, char data);
static void SL811BufRead(int num, short offset, char* buf, short size);
static void SL811BufWrite(int num, short offset, char* buf, short size);
static int regTest(int num);
static int USBStop(int num);
static void USBReset(int num);
static int USBStart(int num);
static int write_data(int num, char* data, int size, int flag);
static int read_data(int num, char* data, int size);
static int intr_data(int num, char* data, int size);
static int get_info(int minor, char* data);
static int dev_init1(int minor);
static void dev_init2(int minor);
static int open_sl811(int minor, int opt);
static int close_sl811(int minor);
static int write_sl811(int minor, char *buffer, int size);
static int read_sl811(int minor, char *buffer, int size);
static int seek_sl811(int minor, int position);
static int ioctl_sl811(int minor, int data, int op);

typedef struct {
	short	epsize;
	short	retry, w_retry;
	short	vendor, product;
	short	error;
	char	intr;
	char	ep;
	char	rd_cmd;
	char	wr_cmd;
	char	status;
	char	pad;
	char	fast;
} USBInfo;

typedef struct {
	volatile char	*sl811data, *sl811addr;
	volatile short *sl811data_w, *sl811addr_w;
	char		speed;
	char		pad;
	char		toggle;
	char		buswidth;
} SL811Info;

#define MAXCONF		128
static USBInfo	*usb;
static SL811Info *sl811;
static char	devdesc[8] = {TRANS_TO_HOST, 6, 0, 1, 0, 0, 64, 0};
static char	confdesc[8] = {TRANS_TO_HOST, 6, 0, 2, 0, 0, MAXCONF, 0};
static char	setaddr[8] = {TRANS_TO_DEV, 5, 0, 0, 0, 0, 0, 0};
static char	setconf[8] = {TRANS_TO_DEV, 9, 1, 0, 0, 0, 0, 0};
static char	clrfeature[8] = {TRANS_TO_DEV + TRANS_TO_EP,1,0,0,0,0,0,0};

static void waitus(unsigned int times) {
	volatile int w;
	for(w = 0;w < times * WAITCONST;w++);
}

static char SL811Read(int num, char offset) {
	char data;

	if(sl811[num].buswidth == 16) {
		*(sl811[num].sl811addr_w) = offset;
		data = *(sl811[num].sl811data_w);
	} else {
		*(sl811[num].sl811addr) = offset;
		data = *(sl811[num].sl811data);
	}
	return data;
}

static void SL811Write(int num, char offset, char data){
	if(sl811[num].buswidth == 16) {
		*(sl811[num].sl811addr_w) = offset;
		*(sl811[num].sl811data_w) = data;
	} else {
		*(sl811[num].sl811addr) = offset;
		*(sl811[num].sl811data) = data;
	}
}

static void SL811BufRead(int num, short offset, char* buf, short size){
	if(size <= 0) return;
	if(sl811[num].buswidth == 16) {
		*(sl811[num].sl811addr_w) = offset;
		while(size--) *buf++ = *(sl811[num].sl811data_w);
	} else {
		*(sl811[num].sl811addr) = (char)offset;
		while(size--) *buf++ = *(sl811[num].sl811data);
	}
}

static void SL811BufWrite(int num, short offset, char* buf, short size) {
	if(size <= 0) return;
	if(sl811[num].buswidth == 16) {
		*(sl811[num].sl811addr_w) = offset;
		while(size--) {
			*(sl811[num].sl811data_w) = *buf;
			buf++;
		}
	} else {
		*(sl811[num].sl811addr) = (char)offset;
		while(size--) {
			*(sl811[num].sl811data) = *buf;
			buf++;
		}
	}
}

static int regTest(int num){
	int		i, result;
	unsigned char	data, buf[256];

	result = 0;
	for(i = 16;i < 256; i++) {
		buf[i] = (char)SL811Read(num, i);
		SL811Write(num, i, i);
	}
	for(i = 16;i < 256; i++) {
		data = SL811Read(num, i);
		if(data != i) {
			result = -1;
		}
	}
	for(i = 16;i < 256;i++) SL811Write(num, i, buf[i]);
	return result;
}

static int USBStop(int num) {
	SL811Write(num, SL811H_HOSTCTLREG_A, 0x00);
	SL811Write(num, SL811H_HOSTCTLREG_B, 0x00);
	SL811Write(num, SL811H_CTLREG1, 0);
}

static void USBReset(int num) {
	char	tmp;

	tmp = SL811Read(num, SL811H_CTLREG1);
	SL811Write(num, SL811H_CTLREG1, tmp | SL811H_CTL1VAL_RESET);
	waitms(500);
	SL811Write(num, SL811H_CTLREG1, tmp | SL811H_CTL1MASK_DSTATE);
	waitms(300);
	SL811Write(num, SL811H_CTLREG1, tmp | SL811H_CTL1VAL_RESET);
	waitms(20);
	SL811Write(num, SL811H_CTLREG1, tmp);
}

static int USBStart(int num) {
	int	status;

	SL811Write(num, SL811H_CTLREG2, 0xae);
	SL811Write(num, SL811H_CTLREG1, 0x08);	// reset USB
	waitms(50);				// 20ms
	SL811Write(num, SL811H_CTLREG1, 0);		// remove SE0

	for(status = 0;status < 100;status++) {
		SL811Write(num, SL811H_INTSTATREG, 0xff); // clear all interrupt bits
	}
	status = SL811Read(num, SL811H_INTSTATREG);
	if(status & 0x40){ 			// Check if device is removed
		sl811[num].speed = 0;			// None
		SL811Write(num, SL811H_INTENBLREG,
			   SL811H_INTMASK_XFERDONE_A | SL811H_INTMASK_XFERDONE_B |
			   SL811H_INTMASK_SOFINTR | SL811H_INTMASK_INSRMV);
		return -1;
	}
	SL811Write(num, SL811H_BUFLNTHREG_B, 0);	//zero lenth
	SL811Write(num, SL811H_PIDEPREG_B, 0x50);	//send SOF to EP0
	SL811Write(num, SL811H_DEVADDRREG_B, 0x01);	//address0
	SL811Write(num, SL811H_SOFLOWREG, 0xe0);
	SL811Write(num, SL811H_CTLREG1, 0x8);
	waitms(300);
	if(!(status & 0x80)) {
		sl811[num].speed = USB_LOW;		// Low
		SL811Write(num, SL811H_SOFTMRREG, 0xee);
		SL811Write(num, SL811H_CTLREG1, 0x21);
		SL811Write(num, SL811H_HOSTCTLREG_B, 0x01);
		for(status = 0;status < 20;status++) {
			SL811Write(num, SL811H_INTSTATREG, 0xff);
		}
	} else {
		sl811[num].speed = USB_FULL;		// Full
		SL811Write(num, SL811H_SOFTMRREG, 0xae);
		SL811Write(num, SL811H_CTLREG1, 0x01);
		SL811Write(num, SL811H_HOSTCTLREG_B, 0x01);
		SL811Write(num, SL811H_INTSTATREG, 0xff);
	}
	SL811Write(num, SL811H_INTENBLREG, SL811H_INTMASK_XFERDONE_A | SL811H_INTMASK_XFERDONE_B |
		   SL811H_INTMASK_SOFINTR|SL811H_INTMASK_INSRMV);
	sl811[num].toggle = 0;
	return 0;
}

static int write_data(int num, char *data, int size, int flag) {
	int	remain, wsiz, timovr, slnum;
	char	pktstat, fin, epsv, rdsv, wrsv;

	fin = 0;
	slnum = num / USBNUM;
	remain = size;
	SL811Write(slnum, SL811H_BUFADDRREG_A, 0x10);
	SL811Write(slnum, SL811H_DEVADDRREG_A, num % USBNUM);
	SL811Write(slnum, SL811H_PIDEPREG_A, (flag == 1) ? PID_SETUP : (PID_OUT | usb[num].ep));
	SL811Write(slnum, SL811H_BUFADDRREG_B, 0x10 + usb[num].epsize);
	SL811Write(slnum, SL811H_DEVADDRREG_B, num % USBNUM);
	SL811Write(slnum, SL811H_PIDEPREG_B, (flag == 1) ? PID_SETUP : (PID_OUT | usb[num].ep));

	wsiz = (remain > usb[num].epsize) ? usb[num].epsize : remain;
	if(sl811[slnum].toggle == 0) {
		SL811Write(slnum, SL811H_BUFLNTHREG_A, wsiz);
		if(data != 0 && size > 0) SL811BufWrite(slnum, 0x10, data, wsiz);
	} else {
		SL811Write(slnum, SL811H_BUFLNTHREG_B, wsiz);
		if(data != 0 && size > 0) SL811BufWrite(slnum, 0x10 + usb[num].epsize, data, wsiz);
	}
	remain -= wsiz;
	do {
		wsiz = (remain > usb[num].epsize) ? usb[num].epsize : remain;
		for(timovr = 0;timovr < usb[num].retry;timovr++) {
			if(sl811[slnum].toggle == 0) {
				sl811[slnum].toggle = 1;
				SL811Write(slnum, SL811H_INTSTATREG, 0xff);
				SL811Write(slnum, SL811H_HOSTCTLREG_A, usb[num].wr_cmd);
				SL811Write(slnum, SL811H_BUFLNTHREG_B, wsiz);
				if(data != 0 && size > 0 && wsiz > 0) {
					SL811BufWrite(slnum, 0x10 + usb[num].epsize, &data[size - remain], wsiz);
				} else {
					waitus(USB_WAIT);
					fin = 1;
				}
				waitus(SL811_WAIT);
				while(SL811Read(slnum, SL811H_INTSTATREG) & SL811H_INTMASK_XFERDONE_A == 0);
				waitus(SL811_WAIT);
				pktstat = SL811Read(slnum, SL811H_PKTSTATREG_A);
			} else {
				sl811[slnum].toggle = 0;
				SL811Write(slnum, SL811H_INTSTATREG, 0xff);
				SL811Write(slnum, SL811H_HOSTCTLREG_B, usb[num].wr_cmd);
				SL811Write(slnum, SL811H_BUFLNTHREG_A, wsiz);
				if(data != 0 && size > 0 && wsiz > 0) {
					SL811BufWrite(slnum, 0x10, &data[size - remain], wsiz);
				} else {
					waitus(USB_WAIT);
					fin = 1;
				}
				waitus(SL811_WAIT);
				while(SL811Read(slnum, SL811H_INTSTATREG) & SL811H_INTMASK_XFERDONE_B == 0);
				waitus(SL811_WAIT);
				pktstat = SL811Read(slnum, SL811H_PKTSTATREG_B);
			}
			if(pktstat & SL811H_STATMASK_ACK) {
				if(pktstat & SL811H_STATMASK_ERROR) {
					usb[num].error = USB_WRITE_ERROR;
#ifdef _KERNEL_		/* ----- Append 2010/01/26 A.Ohnishi ----- */
					__write(0,0,"e",1);
#endif
					waitms(100);
				}
				break;
			}
			if(pktstat & SL811H_STATMASK_STALL) {
				epsv = usb[num].ep;
				wrsv = usb[num].wr_cmd;
				rdsv = usb[num].rd_cmd;
				clrfeature[4] = TRANS_TO_HOST | usb[num].ep;
				usb[num].wr_cmd = DATA0_WR;
				usb[num].rd_cmd = DATA1_RD;
				setup_data(num, clrfeature);
				usb[num].ep = 0;
				if(read_data(num, 0, 0) > 0) {
					usb[num].ep = epsv;
					break;
				}
				usb[num].ep = epsv;
				usb[num].wr_cmd = wrsv;
				usb[num].rd_cmd = rdsv;
				usb[num].wr_cmd = (usb[num].wr_cmd == DATA0_WR) ? DATA1_WR : DATA0_WR;
				return -1;
			}
			sl811[slnum].toggle = (sl811[slnum].toggle == 0) ? 1 : 0;
			waitus((timovr < 40) ? 8000 : 100000);
		}
		remain -= wsiz;
		if(!(pktstat & SL811H_STATMASK_ACK)) {
			return -1;
		}
		usb[num].wr_cmd = (usb[num].wr_cmd == DATA0_WR) ? DATA1_WR : DATA0_WR;
	} while(fin == 0);
	return size;
}

static int read_data(int num, char *data, int size) {
	char	pktstat, epsv, wrsv, rdsv;
	int	timovr, remain, rsiz, rcnt, xfercnt;
	int	maxretry, minretry;
	int	slnum;
	char	*bufptr;
	int	bufsiz;

	pktstat = 0;
	bufptr = 0;
	slnum = num / USBNUM;
	remain = size;
	maxretry = usb[num].retry;
	minretry = (maxretry <= 7) ? maxretry : maxretry / 3;
	if(minretry > 40) minretry = 40;
	if(usb[num].fast) minretry = maxretry;
	SL811Write(slnum, SL811H_DEVADDRREG_A, num % USBNUM);
	SL811Write(slnum, SL811H_PIDEPREG_A, PID_IN | usb[num].ep);
	SL811Write(slnum, SL811H_DEVADDRREG_B, num % USBNUM);
	SL811Write(slnum, SL811H_PIDEPREG_B, PID_IN | usb[num].ep);
	SL811Write(slnum, SL811H_BUFADDRREG_A, 0x10);
	SL811Write(slnum, SL811H_BUFADDRREG_B, 0x10 + usb[num].epsize);
	do {
		rsiz = (remain > usb[num].epsize) ? usb[num].epsize : remain;
		SL811Write(slnum, SL811H_BUFLNTHREG_A, rsiz);
		SL811Write(slnum, SL811H_BUFLNTHREG_B, rsiz);
		for(timovr = 0;timovr < maxretry;timovr++) {
			if(sl811[slnum].toggle == 0) {
				sl811[slnum].toggle = 1;
				SL811Write(slnum, SL811H_INTSTATREG, 0xff);
				SL811Write(slnum, SL811H_HOSTCTLREG_A, usb[num].rd_cmd);
				if(bufptr) {
					SL811BufRead(slnum, 0x10 + usb[num].epsize, bufptr, bufsiz);
					bufptr = 0;
				} else {
					waitus((timovr < minretry) ? USB_MIN_WAIT : 2000);
				}
				waitus(SL811_WAIT);
				while(SL811Read(slnum, SL811H_INTSTATREG) & SL811H_INTMASK_XFERDONE_A == 0);
				waitus(SL811_WAIT);
				pktstat = SL811Read(slnum, SL811H_PKTSTATREG_A);
				xfercnt = (int)SL811Read(slnum, SL811H_XFERCNTREG_A);
			} else {
				sl811[slnum].toggle = 0;
				SL811Write(slnum, SL811H_INTSTATREG, 0xff);
				SL811Write(slnum, SL811H_HOSTCTLREG_B, usb[num].rd_cmd);
				if(bufptr) {
					SL811BufRead(slnum, 0x10, bufptr, bufsiz);
					bufptr = 0;
				} else {
					waitus((timovr < minretry) ? USB_MIN_WAIT : 2000);
				}
				waitus(SL811_WAIT);
				while(SL811Read(slnum, SL811H_INTSTATREG) & SL811H_INTMASK_XFERDONE_B == 0);
				waitus(SL811_WAIT);
				pktstat = SL811Read(slnum, SL811H_PKTSTATREG_B);
				xfercnt = (int)SL811Read(slnum, SL811H_XFERCNTREG_B);
			}
			if(xfercnt == 1 && pktstat & SL811H_STATMASK_SETUP) {
				usb[num].error = USB_DEV_DOWN;
#ifdef _KERNEL_		/* ----- Append 2010/01/26 A.Ohnishi ----- */
				__write(0,0,"X",1);
#endif
				return -1;
			}
			if(pktstat & SL811H_STATMASK_ACK) break;
			if(pktstat & (SL811H_STATMASK_ERROR | SL811H_STATMASK_TMOUT | SL811H_STATMASK_OVF | SL811H_STATMASK_NAK)) {
				waitus(500);
				continue;
			}
			if(pktstat & SL811H_STATMASK_STALL) {
				epsv = usb[num].ep;
				wrsv = usb[num].wr_cmd;
				rdsv = usb[num].rd_cmd;
				clrfeature[4] = TRANS_TO_HOST | usb[num].ep;
				usb[num].wr_cmd = DATA0_WR;
				usb[num].rd_cmd = DATA1_RD;
				setup_data(num, clrfeature);
				usb[num].ep = 0;
				if(read_data(num, 0, 0) > 0) {
					usb[num].ep = epsv;
					break;
				}
				usb[num].ep = epsv;
				usb[num].wr_cmd = wrsv;
				usb[num].rd_cmd = rdsv;
				usb[num].rd_cmd = (usb[num].rd_cmd == DATA0_RD) ? DATA1_RD : DATA0_RD;
				return -1;
			}
			waitus(2500);
		}
		if(timovr >= maxretry) {
			return -1;
		}
		rcnt = rsiz - xfercnt;
		if(pktstat & SL811H_STATMASK_ACK && data != 0 && size > 0) {
			bufptr = &data[size - remain];
			bufsiz = rsiz;
		}
		remain -= rcnt;
		usb[num].rd_cmd = (usb[num].rd_cmd == DATA0_RD) ? DATA1_RD : DATA0_RD;
	} while(remain > 0 && rcnt == rsiz);
	if(bufptr) {
		if(sl811[slnum].toggle == 0) {
			SL811BufRead(slnum, 0x10 + usb[num].epsize, bufptr, bufsiz);
		} else {
			SL811BufRead(slnum, 0x10, bufptr, bufsiz);
		}
	}
	return size - remain;
}

static int intr_data(int num, char *data, int size) {
	char	pktstat;
	int	rcnt, slnum;

	slnum = num / USBNUM;
	SL811Write(slnum, SL811H_DEVADDRREG_A, num % USBNUM);
	SL811Write(slnum, SL811H_PIDEPREG_A, PID_IN | usb[num].ep);
	SL811Write(slnum, SL811H_BUFADDRREG_A, 0x10);
	SL811Write(slnum, SL811H_BUFLNTHREG_A, size);
	SL811Write(slnum, SL811H_HOSTCTLREG_A, usb[num].rd_cmd);
	waitus(30);
	while(SL811Read(slnum, SL811H_INTSTATREG) & SL811H_INTMASK_XFERDONE_A == 0);
	pktstat = SL811Read(slnum, SL811H_PKTSTATREG_A);
	rcnt = size - (int)SL811Read(slnum, SL811H_XFERCNTREG_A);
	if(pktstat & SL811H_STATMASK_ACK) {
		SL811BufRead(slnum, 0x10, data, size);
		usb[num].rd_cmd = (usb[num].rd_cmd == DATA0_RD) ? DATA1_RD : DATA0_RD;
	} else {
		rcnt = 0;
	}
	return rcnt;
}

static int get_info(int minor, char *data) {
	usb[minor].wr_cmd = DATA0_WR;
	usb[minor].rd_cmd = DATA1_RD;
	if(setup_data(minor, confdesc) == -1) return -1;
	if(read_data(minor, data, MAXCONF) == -1) return -1;
	if(status_data(minor) == -1) return -1;
	return data[2];
}

static int dev_init1(int minor) {
	char	retry;
	int	sl_minor;
	DevDesc	devinfo;

	sl_minor = minor / USBNUM;
	usb[sl_minor * USBNUM].wr_cmd = DATA0_WR;
	setup_data(sl_minor * USBNUM, devdesc);
	for(retry = 0;retry < 8;retry++) {
		usb[sl_minor * USBNUM].rd_cmd = DATA1_RD;
		if(read_data(sl_minor * USBNUM, (char*)&devinfo, 8) != -1) break;
		waitms(200);
		usb[sl_minor * USBNUM].wr_cmd = DATA0_WR;
		setup_data(sl_minor * USBNUM, devdesc);
	}
	if(retry == 8) return -1;
	if(status_data(sl_minor * USBNUM) == -1) return -1;
	setaddr[2] = minor % USBNUM;
	usb[sl_minor * USBNUM].wr_cmd = DATA0_WR;
	usb[sl_minor * USBNUM].rd_cmd = DATA1_RD;
	if(setup_data(sl_minor * USBNUM, setaddr) == -1) return -1;
	if(read_data(sl_minor * USBNUM, 0, 0) == -1) return -1;
	usb[minor].epsize = devinfo.bMaxPacketSize0;
	usb[minor].ep = 0;
	return 0;
}

static void dev_init2(int minor) {
	DevDesc	devinfo;

	usb[minor].retry = DEFAULT_TIMEOUT;
	usb[minor].wr_cmd = DATA0_WR;
	setup_data(minor, devdesc);
	usb[minor].rd_cmd = DATA1_RD;
	read_data(minor, (char*)&devinfo, sizeof(DevDesc));
	usb[minor].vendor = le16toh(devinfo.idVendor);
	usb[minor].product = le16toh(devinfo.idProduct);
	status_data(minor);
}

static int open_sl811(int minor, int option) {
	char	retry, buf[8];
	int	num, sl_minor;
	DevDesc	devinfo;

	sl_minor = minor / USBNUM;
	if(minor >= USBNUM * SLNUM) return -1;
	if(sl811[sl_minor].sl811addr == 0 && sl811[sl_minor].sl811data == 0) return -1;
	if(minor == sl_minor * USBNUM) {
		for(num = 0;num < USBNUM;num++) {
			usb[minor + num].ep = 0;
			usb[minor + num].intr = 0;
			usb[minor + num].epsize = 0;
			usb[minor + num].fast = 0;
			usb[minor + num].retry = DEFAULT_TIMEOUT;
			usb[minor + num].w_retry = DEFAULT_WRITE_TIMEOUT;
		}
		if(regTest(sl_minor) != -1) {
			if(USBStart(sl_minor) == 0) usb[minor].epsize = 8;
		}
		return -1;
	}
	if(usb[sl_minor * USBNUM].epsize == 0) return -1;
	if(option == -1) return 0;
	if(dev_init1(minor) == -1) return -1;
	dev_init2(minor);
	return 0;
}

static int close_sl811(int minor) {
	if(minor == 0 || minor >= USBNUM * SLNUM) return -1;
	if(usb[minor].epsize == 0) return -1;
	usb[minor].ep = 0;
	usb[minor].intr = 0;
	usb[minor].epsize = 0;
	usb[minor].retry = DEFAULT_TIMEOUT;
	if(minor == 1) USBStop(minor / USBNUM);
	return 0;
}

static int write_sl811(int minor, char *buffer, int size) {
	if(minor == 0 || minor >= USBNUM * SLNUM) return -1;
	if(usb[minor].epsize == 0) return -1;
	return write_data(minor, buffer, size, 0);
}

static int seek_sl811(int minor, int position) {
	if(minor == 0 || minor >= USBNUM * SLNUM) return -1;
	if(usb[minor].epsize == 0) return -1;
	usb[minor].ep = position;
}

static int ioctl_sl811(int minor, int data, int op) {
	int	ret;
	char	epsv, buf[8];

	if(minor == 0 || minor >= USBNUM * SLNUM) return -1;
	if(usb[minor].epsize == 0) return -1;
	ret = 0;
	switch(op) {
	case DEV_INFO:
		break;
	case DEV_RESET:
		USBStart(minor / USBNUM);
		dev_init1(minor);
		dev_init2(minor);
		break;
	case USB_RESET:
		USBReset(minor / USBNUM);
		break;
	case USB_EPSIZE:
		ret = usb[minor].epsize;
		break;
	case USB_SET_EPSIZE:
		usb[minor].epsize = data;
		break;
	case USB_SETUP:
		usb[minor].wr_cmd = DATA0_WR;
		usb[minor].rd_cmd = DATA1_RD;
		if(setup_data(minor, (char*)data) == -1) return -1;
		break;
	case USB_INFO:
		ret = get_info(minor, (char*)data);
		break;
	case USB_SETCONF:
		ret = -1;
		usb[minor].wr_cmd = DATA0_WR;
		usb[minor].rd_cmd = DATA1_RD;
		setconf[2] = data;
		if(setup_data(minor, setconf) == -1) break;
		if(read_data(minor, 0, 0) == -1) break;
		usb[minor].wr_cmd = DATA0_WR;
		ret = 0;
		break;
	case USB_TIMEOUT:
		usb[minor].retry = (data <= 0) ? DEFAULT_TIMEOUT : data;
		break;
	case USB_WRITE_TIMEOUT:
		usb[minor].w_retry = (data <= 0) ? DEFAULT_WRITE_TIMEOUT : data;
		break;
	case DEV_STOP:
		ret = USBStop(minor / USBNUM);
		break;
	case USB_INTR_EP:
		usb[minor].intr = data;
		break;
	case USB_INTERRUPT:
		if(usb[minor].intr != 0) {
			epsv = usb[minor].ep;
			usb[minor].ep = usb[minor].intr;
			ret = intr_data(minor, (char*)data, 8);
			usb[minor].ep = epsv;
		}
		break;
	case USB_GETSTATUS:
		break;
	case USB_VENDOR:
		ret = usb[minor].vendor;
		break;
	case USB_PRODUCT:
		ret = usb[minor].product;
		break;
	case USB_FAST_RETRY:
		usb[minor].fast = data;
		break;
	case USB_ERROR_STATUS:
		ret = usb[minor].error;
		usb[minor].error = USB_NO_ERROR;
		break;
	default:
		ret = -1;
	}
	return ret;
}

static Functions func;

int device_main(int argc, char **argv) {
	unsigned int	num, n, wait[SLNUM];
	char		*buf;

	buf = malloc(sizeof(USBInfo) * USBNUM * SLNUM + sizeof(SL811Info) * SLNUM);
	usb = (USBInfo*)buf;
	sl811 = (SL811Info*)&buf[sizeof(USBInfo) * USBNUM * SLNUM];
#ifndef _KERNEL_
	num = 0;
	for(n = 1;n < argc;n++) {
		if(memcmp(argv[n], "usb", 3) == 0) {
			num = stoi(&argv[n][3]);
			if(num >= SLNUM) num = 0;
		} else if(memcmp(argv[n], "ar=", 3) == 0) {
			sl811[num].sl811addr = (volatile char *)stoh(&argv[n][3]);
			sl811[num].sl811addr_w = (volatile short *)stoh(&argv[n][3]);
		} else if(memcmp(argv[n], "dr=", 3) == 0) {
			sl811[num].sl811data = (volatile char *)stoh(&argv[n][3]);
			sl811[num].sl811data_w = (volatile short *)stoh(&argv[n][3]);
		} else if(memcmp(argv[n], "bw=", 3) == 0) {
			sl811[num].buswidth = stoi(&argv[n][3]);
		} else if(memcmp(argv[n], "w=", 2) == 0) {
			wait[num] = stoi(&argv[n][2]);
		}
	}
#endif
	for(num = 0;num < SLNUM;num++) {
#ifdef _KERNEL_
		sl811[num].sl811addr = (volatile char *)usb_config[num].addr_reg;
		sl811[num].sl811addr_w = (volatile short *)usb_config[num].addr_reg;
		sl811[num].sl811data = (volatile char *)usb_config[num].data_reg;
		sl811[num].sl811data_w = (volatile short *)usb_config[num].data_reg;
		sl811[num].buswidth = usb_config[num].width;
		wait[num] = usb_config[num].wait;
#endif
		set_buswidth((int)sl811[num].sl811addr, sl811[num].buswidth, wait[num]);
	}
	for(num = 0;num < USBNUM * SLNUM;num++) {
		usb[num].ep = 0;
		usb[num].intr = 0;
		usb[num].epsize = 0;
		usb[num].fast = 0;
		usb[num].retry = DEFAULT_TIMEOUT;
	}
	func.open_dev = open_sl811;
	func.close_dev = close_sl811;
	func.write_dev = write_sl811;
	func.read_dev = read_data;
	func.seek_dev = seek_sl811;
	func.ioctl_dev = ioctl_sl811;
	func.poll_dev = 0;
	strcpy(&(func.name[0]), "usb");
	device_return(&func);
}

#endif
