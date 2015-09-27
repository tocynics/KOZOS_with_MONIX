/*****************************************************************************
	オーダー	: USBライブラリ
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: usb.c
	接頭辞		: usb
	作成日時	: 2015-05-08
*****************************************************************************/
/*
	作成日時
*/

/*****************************************************************************
	インクルード
*****************************************************************************/
#include "reg3069.h"
#include "usblib.h"
#include "lib.h"

/*****************************************************************************
	定義
*****************************************************************************/

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/

typedef struct _get_desc_struct {
	int num;
	short type;
	char *data;
	int datasize;
	int epsize;
	char index;
	char cDummy;
	short lang;
} GET_DESCTYPE;

static SL811Info sl811[SL_NUM];

/*****************************************************************************
	外部変数
*****************************************************************************/

/*****************************************************************************
	内部変数
*****************************************************************************/

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/

static int regTest(int num){
	int	i, data, result = 0;
	char	buf[256];

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

static int USBReset(int num) {
	int	status;

	SL811Write(num, SL11H_CTLREG2, 0xae);
	SL811Write(num, SL11H_CTLREG1, 0x08);	// reset USB
	waitms(20);				// 20ms
	SL811Write(num, SL11H_CTLREG1, 0);	// remove SE0

	for(status = 0;status < 100;status++) {
		SL811Write(num, SL11H_INTSTATREG, 0xff); // clear all interrupt bits
	}
	status = SL811Read(num, SL11H_INTSTATREG);
	if(status & 0x40){  // Check if device is removed
		sl811[num].speed = 0;	// None
		SL811Write(num, SL11H_INTENBLREG, SL11H_INTMASK_XFERDONE | 
			   SL11H_INTMASK_SOFINTR | SL11H_INTMASK_INSRMV);
		return -1;
	}
	SL811Write(num, SL11H_BUFLNTHREG_B, 0);	//zero lenth
	SL811Write(num, SL11H_PIDEPREG_B, 0x50);	//send SOF to EP0
	SL811Write(num, SL11H_DEVADDRREG_B, 0x01);	//address0
	SL811Write(num, SL11H_SOFLOWREG, 0xe0);
	if(!(status & 0x80)) {
		sl811[num].speed = USB_LOW;	// Low
		SL811Write(num, SL11H_CTLREG1, 0x8);
		waitms(20);
		SL811Write(num, SL11H_SOFTMRREG, 0xee);
		SL811Write(num, SL11H_CTLREG1, 0x21);
		SL811Write(num, SL11H_HOSTCTLREG_B, 0x01);
		for(status = 0;status < 20;status++) {
			SL811Write(num, SL11H_INTSTATREG, 0xff);
		}
	} else {
		sl811[num].speed = USB_FULL;	// Full
		SL811Write(num, SL11H_CTLREG1, 0x8);
		waitms(20);
		SL811Write(num, SL11H_SOFTMRREG, 0xae);
		SL811Write(num, SL11H_CTLREG1, 0x01 );
		SL811Write(num, SL11H_HOSTCTLREG_B, 0x01);
		SL811Write(num, SL11H_INTSTATREG, 0xff);
	}
	SL811Write(num, SL11H_INTENBLREG, SL11H_INTMASK_XFERDONE | 
		   SL11H_INTMASK_SOFINTR|SL11H_INTMASK_INSRMV);
	return 0;
}

static int write_setup(int num, char* data) {
	char	pktstat, ret;

	SL811BufWrite(num, 0x10, data, 8);
	SL811Write(num, SL11H_BUFADDRREG, 0x10);
	SL811Write(num, SL11H_BUFLNTHREG, 8);
	SL811Write(num, SL11H_DEVADDRREG, sl811[num].addr);
	SL811Write(num, SL11H_PIDEPREG, PID_SETUP);
	SL811Write(num, SL11H_HOSTCTLREG, DATA0_WR);
	waitus(200);
	while(SL811Read(num, SL11H_INTSTATREG) & SL11H_INTMASK_XFERDONE == 0);
	pktstat = SL811Read(num, SL11H_PKTSTATREG);
	ret = 0;
	if(pktstat & SL11H_STATMASK_ACK) ret |= USB_ACK;
	if(pktstat & SL11H_STATMASK_NAK) ret |= USB_NAK;
	if(pktstat & SL11H_STATMASK_STALL) ret |= USB_STALL;
	return ret;
}

int write_sl811(int num, char* data, int size) {
	char	pktstat, ret;

	if(size < USB_HDRSIZ) return 0;
	SL811BufWrite(num, 0x10, &data[USB_HDRSIZ], size - USB_HDRSIZ);
	SL811Write(num, SL11H_BUFADDRREG, 0x10);
	SL811Write(num, SL11H_BUFLNTHREG, size - USB_HDRSIZ);
	SL811Write(num, SL11H_DEVADDRREG, sl811[num].addr);
	SL811Write(num, SL11H_PIDEPREG, PID_OUT | data[USB_EP]);
	SL811Write(num, SL11H_HOSTCTLREG, (data[USB_TOGGLE] == 0) ? DATA0_WR : DATA1_WR);
	waitus(200);
	while(SL811Read(num, SL11H_INTSTATREG) & SL11H_INTMASK_XFERDONE == 0);
	pktstat = SL811Read(num, SL11H_PKTSTATREG);
	ret = 0;
	if(pktstat & SL11H_STATMASK_ACK) ret |= USB_ACK;
	if(pktstat & SL11H_STATMASK_NAK) ret |= USB_NAK;
	if(pktstat & SL11H_STATMASK_STALL) ret |= USB_STALL;
	return ret;
}

int read_sl811(int num, char* data, int size) {
	char	pktstat, ret;
	int	timovr;

	if(size < 0) return 0;
	SL811Write(num, SL11H_BUFADDRREG, 0x10);
	SL811Write(num, SL11H_BUFLNTHREG, size);
	SL811Write(num, SL11H_DEVADDRREG, sl811[num].addr);
	SL811Write(num, SL11H_PIDEPREG, PID_IN | data[USB_EP]);
	for(timovr = 0;timovr < 200;timovr++) {
		SL811Write(num, SL11H_HOSTCTLREG, (data[USB_TOGGLE] == 0) ? DATA0_RD : DATA1_RD);
		waitus(200);
		while(SL811Read(num, SL11H_INTSTATREG) & SL11H_INTMASK_XFERDONE == 0);
		pktstat = SL811Read(num, SL11H_PKTSTATREG);
		if(!(pktstat & SL11H_STATMASK_NAK)) break;
	}
	if(pktstat & SL11H_STATMASK_ACK) SL811BufRead(num, 0x10, data, size);
	ret = 0;
	if(pktstat & SL11H_STATMASK_ACK) ret |= USB_ACK;
	if(pktstat & SL11H_STATMASK_NAK) ret |= USB_NAK;
	if(pktstat & SL11H_STATMASK_STALL) ret |= USB_STALL;
	return ret;
}

int seek_sl811(int num, int position) {
	if(num >= SL_NUM) return -1;
	sl811[num].addr = position;
	return 0;
}

int ioctl_sl811(int num, int data, int op) {
	static char	getdesc[8] = {0x80, 0x06, 0, 1, 0, 0, 64, 0};
	static char	setaddr[8] = {0x00, 0x05, 2, 0, 0, 0, 0, 0};
	char		buf[10];
	int		ret;

	if(num >= SL_NUM) return -1;
	ret = 0;
	switch(op) {
	case USB_RESET:
		ret = -1;
		sl811[num].addr = 0;
		if(USBReset(num) != 0) break;
		if((write_setup(num, getdesc) & USB_ACK) == 0) break;
		buf[USB_EP] = 0, buf[USB_TOGGLE] = 1;
		if((read_sl811(num, buf, 8) & USB_ACK) == 0) break;
		buf[USB_EP] = 0, buf[USB_TOGGLE] = 1;
		if((write_sl811(num, buf, 2) & USB_ACK) == 0) break;
		if((write_setup(num, setaddr) & USB_ACK) == 0) break;
		buf[USB_EP] = 0, buf[USB_TOGGLE] = 1;
		if((read_sl811(num, buf, 0) & USB_ACK) == 0) break;
		sl811[num].addr = data;
		ret = buf[7];
		break;
	case USB_GETSPEED:
		ret = sl811[num].speed;
		break;
	case USB_SETUP:
		ret = write_setup(num, (char*)data);
		break;
	default:
		ret = -1;
	}
	return ret;
}

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
	int		num, i, ep, cfgsiz, offset;
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
	num = 0;
	sl811[num].addr_reg = (volatile char *)0x200001;
	sl811[num].data_reg = (volatile char *)0x200003;
	sl811[num].addr = 0;
	if(regTest(num) != 0) {
		print("USB not found!\n");
		for(;;);
	}
	epsize = ioctl_sl811(num, 2, USB_RESET);
	if(epsize == -1) {
		print("Device not found!\n");
		for(;;);
	}
	print("Size of endpoint is %d[byte]", epsize);
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
	print("\nDevice desc : ");
	p = (char*)&device;
	for(i = 0;i < sizeof(DevDesc);i++) print("%02x ", (int)p[i] & 0xff);

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
	print("\nConfiguration desc : ");
	p = (char*)&config;
	for(i = 0;i < sizeof(CfgDesc);i++) print("%02x ", (int)p[i] & 0xff);
	if(device.iProduct != 0) {

		desctype.num = num;
		desctype.type = STRING_TYPE;
		desctype.data = (char*)&string;
		desctype.datasize = sizeof(StrDesc);
		desctype.epsize = epsize;
		desctype.index = 0;
		desctype.lang = 0;

		get_desc(&desctype);
		print("\nString desc : ");
		p = (char*)&string;
		for(i = 0;i < sizeof(StrDesc);i++) print("%02x ", (int)p[i] & 0xff);

		desctype.num = num;
		desctype.type = STRING_TYPE;
		desctype.data = buffer;
		desctype.datasize = 128;
		desctype.epsize = epsize;
		desctype.index = device.iProduct;
		desctype.lang = le16toh(string.wLang);

		get_desc(&desctype);
		print("\n[");
		p = (char*)&string;
		for(i = 2;i < ((int)buffer[0] & 0xff);i+=2) print("%c", buffer[i]);
		print("]");
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
	print("\nInterface desc : ");
	p = (char*)&interface;
	for(i = 0;i < sizeof(IntfDesc);i++) print("%02x ", (int)p[i] & 0xff);
	offset += sizeof(IntfDesc);
	epnum = interface.bEndPoints;
	class = interface.iClass;
	subclass = interface.iSub;
	protocol = interface.iProto;
	for(ep = 0;ep < epnum;ep++) {
		if(ep == MAX_EP) break;
		memcpy((char*)&endpoint[ep], &buffer[offset], sizeof(EPDesc));
		offset += sizeof(EPDesc);
		print("\nEndpoint desc(%d) : ", ep);
		p = (char*)&endpoint[ep];
		for(i = 0;i < sizeof(EPDesc);i++) print("%02x ", (int)p[i] & 0xff);
	}
	print("\nEndpoint number is %d\n", epnum);
	print("Vendor %d product %d Class %d Sub %d Proto %d\n",
		(int)vendor, (int)product, (int)class & 0xff, (int)subclass & 0xff, (int)protocol & 0xff);
//	for(;;);
}
