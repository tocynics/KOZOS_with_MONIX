/*****************************************************************************
	オーダー	: USBドライバ
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: usb.c
	接頭辞		: usb
	作成日時	: 2014-06-26
*****************************************************************************/
/*
	改訂履歴
*/

/*****************************************************************************
	インクルード
*****************************************************************************/
#include "reg3069.h"
#include "usb.h"
#include "lib.h"
#include "sl811.h"

/*****************************************************************************
	定義
*****************************************************************************/

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/

/*****************************************************************************
	外部変数
*****************************************************************************/

/*****************************************************************************
	内部変数
*****************************************************************************/

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/

/*****************************************************************************
	テーブル
*****************************************************************************/

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-26
*****************************************************************************/
void get_desc(int num, short type, char *data, int datasize, int epsize, char index, short lang)
{
	SetupPKG	command;
	char		toggle, buf[2];
//	int		i, remain, size, offset;
	int		remain, size, offset;

	command.bmRequest = TRANS_TO_HOST;
	command.bRequest = GET_DESCRIPTOR;
	command.wValue = htole16((type << 8) | index);
	command.wIndex = lang;
	command.wLength = htole16(datasize);
//	ioctl_sl811(num, (long)&command, USB_SETUP);
	sl811Setup((char*)&command);
	offset = 0;
	toggle = 1;
	buf[USB_TOGGLE] = 1;
	for(remain = datasize;remain > 0;remain -= size) {
		size = (remain > epsize) ? epsize : remain;
		if(size < USB_HDRSIZ) {
			buf[USB_EP] = 0;
			read_sl811(num, buf, size);
			data[offset] = buf[0];
		} else {
			data[offset + USB_EP] = 0;
			data[offset + USB_TOGGLE] = toggle;
			read_sl811(num, &data[offset], size);
		}
		if(offset == 0 && data[1] != CONFIG_TYPE){
			remain = (int)data[0] & 0xff;
		}
		toggle = (toggle == 0) ? 1 : 0;
		offset += size;
	}
	buf[USB_EP] = 0;
	buf[USB_TOGGLE] = 1;
	write_sl811(num, buf, 2);
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-26
*****************************************************************************/
int usb_main(int argc, char** argv)
{
	int			num, i, ep, cfgsiz, offset;
	char		buffer[128], *p;
	DevDesc		device;
	CfgDesc		config;
	StrDesc		string;
	IntfDesc	interface;
	EPDesc		endpoint[MAX_EP];
	int			epsize, epnum;
	short		vendor, product;
	char		class, subclass, protocol;

	num = sl811Init();
	if(num != 0){
		puts("USB not found!\n");
		return -1;
	}

//	epsize = ioctl_sl811(num, 2, USB_RESET);
	epsize = sl811Reset(2);
	if(epsize == -1) {
		puts("Device not found!\n");
		return -2;
	}
	puts("Size of endpoint is ");
	printhex(epsize, 0, 1);
	get_desc(num, DEVICE_TYPE, (char*)&device, sizeof(DevDesc), epsize, 0, 0);
	vendor = le16toh(device.idVendor);
	product = le16toh(device.idProduct);
	puts("\nDevice desc : ");
	p = (char*)&device;
	for(i = 0;i < sizeof(DevDesc);i++){
		printhex((int)p[i] & 0xff, 2, 0);
		puts(" ");
	}
	get_desc(num, CONFIG_TYPE, (char*)&config, sizeof(CfgDesc), epsize, 0, 0);
	cfgsiz = le16toh(config.wLength);
	if(cfgsiz > 128){
		cfgsiz = 128;
	}
	puts("\nConfiguration desc : ");
	p = (char*)&config;
	for(i = 0;i < sizeof(CfgDesc);i++){
		printhex((int)p[i] & 0xff, 2, 0);
		puts(" ");
	}
	if(device.iProduct != 0){
		get_desc(num, STRING_TYPE, (char*)&string, sizeof(StrDesc), epsize, 0, 0);
		
		puts("\nString desc : ");
		p = (char*)&string;
		for(i = 0;i < sizeof(StrDesc);i++){
			printhex((int)p[i] & 0xff, 2, 0);
			puts(" ");
		}
		
		get_desc(num, STRING_TYPE, buffer, 128, epsize,
					device.iProduct, le16toh(string.wLang));

		puts("\n[");
		p = (char*)&string;
		for(i = 2;i < ((int)buffer[0] & 0xff);i+=2){
			putc(buffer[i]);
		}
		puts("]");
	}
	get_desc(num, CONFIG_TYPE, buffer, cfgsiz, epsize, 0, 0);
	offset = sizeof(CfgDesc);
	memcpy((char*)&interface, &buffer[offset], sizeof(IntfDesc));
	puts("\nInterface desc : ");
	p = (char*)&interface;
	for(i = 0;i < sizeof(IntfDesc);i++){
		printhex((int)p[i] & 0xff, 2, 0);
		puts(" ");
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
		puts("\nEndpoint desc(");
		printhex(ep, 0, 1);
		puts(") : ");
		p = (char*)&endpoint[ep];
		for(i = 0;i < sizeof(EPDesc);i++){
			printhex((int)p[i] & 0xff, 2, 0);
			puts(" ");
		}
	}
	puts("\nEndpoint number is ");
	printhex(epnum, 0, 1);
	puts("\n");
	
	puts("Vendor ");
	printhex(vendor, 0, 1);
	puts(" ");
	
	puts("product ");
	printhex(product, 0, 1);
	puts(" ");

	puts("Class ");
	printhex(class, 0, 1);
	puts(" ");
	
	puts("Sub ");
	printhex(subclass, 0, 1);
	puts(" ");
	
	puts("Proto ");
	printhex(protocol, 0, 1);
	puts("\n");
//	for(;;);
	return 0;
}

/***** End Of File *****/
