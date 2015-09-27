/****************************************/
/* MES/Copyleft Yukio Mituiwa,2005	*/
/*					*/
/*  2005/6/1 first release		*/
/*					*/
/****************************************/
//#include <string.h>
//#include <macro.h>
//#include "mes.h"
//#include "id.h"
#include "usb.h"
#include "lib.h"

typedef struct {
	int	fd;
	void	(*handle)(int);
	char	minor;
	char	class;
	char	sub;
	char	use;
	char	epnum;
	char	addr[MAX_EP];
	char	attr[MAX_EP];
	char	epsiz[MAX_EP];
} USBInfo;

typedef struct {
	unsigned short	status __attribute__((packed));
	unsigned short	change __attribute__((packed));
} Status;

static char	gethubdesc[8] = {TRANS_TO_HOST+TYPE_CLASS,GET_DESCRIPTOR,PORT_CONNECT,0,0,0,71,0};
static char	setport[8] = {TRANS_TO_DEV+TYPE_CLASS+TRANS_TO_OTHER,SET_FEATURE,PORT_POWER,0,0,0,0,0};
static char	clearport[8] = {TRANS_TO_DEV+TYPE_CLASS+TRANS_TO_OTHER,CLEAR_FEATURE,C_PORT_CONNECT,0,0,0,0,0};
static char	getstatus[8] = {TRANS_TO_HOST+TYPE_CLASS+TRANS_TO_OTHER,GET_STATUS,PORT_CONNECT,0,0,0,4,0};
static char	portreset[8] = {TRANS_TO_DEV+TYPE_CLASS+TRANS_TO_OTHER,SET_FEATURE,PORT_RESET,0,0,0,0,0};
static char	devdesc[8] = {TRANS_TO_HOST,6,0,1,0,0,64,0};
static USBInfo  *info;
static char	*buf;
static char	chip_status[SLNUM];

int usb_setup(int fd, char *desc, char *data) {
	if(__ioctl(SuperTask, fd, (int)desc, USB_SETUP) == -1) return -1;
	__seek(SuperTask, fd, 0);
	if(desc[0] & TRANS_TO_HOST) {
		if(__read(SuperTask, fd, data, desc[6]) == -1) return -1;
		if(__write(SuperTask, fd, 0, 0) == -1) return -1;
	} else {
		if(__read(SuperTask, fd, 0, 0) == -1) return -1;
	}
	return 0;
}

int usb_read(int fd, char *data, int size, int ep) {
	__seek(SuperTask, fd, ep);
	return __read(SuperTask, fd, data, size);
}

int usb_write(int fd, char *data, int size, int ep) {
	__seek(SuperTask, fd, ep);
	return __write(SuperTask, fd, data, size);
}

static int set_info(int num) {
	int		i, fd, ifnum;
	char		name[6];
	CfgDesc		*config;
	IntfDesc	*interface;
	EPDesc		*endpoint;

	if(num < 10) {
		strcpy(name, "usb?");
		name[3] = '0' + num;
	} else {
		strcpy(name, "usb??");
		name[3] = '0' + num % 10;
		name[4] = '0' + num / 10;
	}		
	fd = __open(SuperTask, name, 0);
	if(fd == -1) return -1;
	info[num].fd = fd;
	__ioctl(SuperTask, info[num].fd, (int)buf, USB_INFO);
	config = (CfgDesc*)buf;
	ifnum = config->bNumIntf;
	interface = (IntfDesc*)&buf[sizeof(CfgDesc)];
	endpoint = (EPDesc*)&buf[sizeof(CfgDesc) + sizeof(IntfDesc) * ifnum];
	info[num].class = interface->iClass;
	info[num].sub = interface->iSub;
	info[num].epnum = (interface->bEndPoints > MAX_EP) ? MAX_EP : interface->bEndPoints;
	for(i = 0;i < info[num].epnum;i++) {
		info[num].addr[i] = endpoint[i].bEPAdd;
		info[num].attr[i] = endpoint[i].bAttr;
		info[num].epsiz[i] = le16toh(endpoint[i].wPayLoad);
	}
	return 0;
}

static int usb_start(int chip) {
	int		i, port, num, retry;
	Status		status;

	for(num = 1;num < USBNUM;num++) {
		info[num + chip * USBNUM].fd = 0;
		info[num + chip * USBNUM].use = 0;
		info[num + chip * USBNUM].handle = 0;
	}
	if(set_info(1 + chip * USBNUM) == -1) {
		info[1 + chip * USBNUM].fd = 0;
		info[1 + chip * USBNUM].use = 0;
		info[1 + chip * USBNUM].handle = 0;
		return -1;
	}
	if(info[1 + chip * USBNUM].class != HUBCLASS) return 0;
	usb_setup(info[1 + chip * USBNUM].fd, gethubdesc, buf);
	info[1 + chip * USBNUM].sub = buf[2];
	__ioctl(SuperTask, info[1 + chip * USBNUM].fd, 1, USB_SETCONF);
	for(port = 1;port <= info[1 + chip * USBNUM].sub;port++) {
		setport[4] = port;
		usb_setup(info[1 + chip * USBNUM].fd, setport, 0);
		clearport[4] = port;
		usb_setup(info[1 + chip * USBNUM].fd, clearport, 0);
	}
	num = 2;
	for(port = 1;port <= info[1 + chip * USBNUM].sub;port++) {
		getstatus[4] = port;
		usb_setup(info[1 + chip * USBNUM].fd, getstatus, (char*)&status);
		if(le16toh(status.status) & BIT_PORT_CONNECT) {
			portreset[4] = port;
			usb_setup(info[1 + chip * USBNUM].fd, portreset, 0);
			if(set_info(num + chip * USBNUM) == -1) continue;
			if(++num >= USBNUM) break;
		}
	}
	return 0;
}

int usb_get_desc(int num, int *fd, int *class, int *sub) {
	if(num <= 0 || num >= USBNUM * SLNUM) return 0;
	if(info[num].fd == 0) return 0;
	*fd = info[num].fd;
	*class = info[num].class;
	*sub = info[num].sub;
	return (info[num].use == 0) ? 1 : 0;
}

int usb_get_conf(int num, int *epnum, char *addr, char *attr, char *epsiz) {
	if(num <= 0 || num >= USBNUM * SLNUM) return 0;
	if(info[num].fd == 0) return 0;
	*epnum = info[num].epnum;
	memcpy(addr, &(info[num].addr[0]), *epnum);
	memcpy(attr, &(info[num].attr[0]), *epnum);
	memcpy(epsiz, &(info[num].epsiz[0]), *epnum);
	return (info[num].use == 0) ? 1 : 0;
}

int usb_product(int num, int *vendor, int *product) {
	if(num <= 0 || num >= USBNUM * SLNUM) return 0;
	if(info[num].fd == 0) return 0;
	*vendor = __ioctl(SuperTask, info[num].fd, 0, USB_VENDOR);
	*product = __ioctl(SuperTask, info[num].fd, 0, USB_PRODUCT);
	return (info[num].use == 0) ? 1 : 0;
}

int usb_alloc(int num) {
	if(num <= 0 || num >= USBNUM * SLNUM) return -1;
	if(info[num].fd == 0) return -1;
	if(info[num].use != 0) return -1;
	info[num].use = 1;
	return 0;
}

int usb_free(int num) {
	if(num <= 0 || num >= USBNUM * SLNUM) return -1;
	if(info[num].fd == 0) return -1;
	if(info[num].use == 0) return -1;
	info[num].use = 0;
	return 0;
}

int usb_handle(int num, int minor, void (*handle)(int)) {
	if(num <= 0 || num >= USBNUM * SLNUM) return -1;
	if(info[num].fd == 0) return -1;
	if(info[num].use == 0) return -1;
	info[num].handle = handle;
	info[num].minor = minor;
	return 0;
}

int usb_mount(int chip) {
	int	ret, num, fd;
	char	name[6];

	num = chip * USBNUM;
	if(num < 10) {
		strcpy(name, "usb?");
		name[3] = '0' + num;
	} else {
		strcpy(name, "usb??");
		name[3] = '0' + num % 10;
		name[4] = '0' + num / 10;
	}		
	__open(SuperTask, name, 0);
	ret = usb_start(chip);
	if(ret != 0) return -1;
	fd = __open(SuperTask, "spc0", -1);
	__close(SuperTask, fd);
	mount("spc0");
	mount("spc1");
	mount("spc2");
	mount("spc3");
	chip_status[chip] = 1;
	return 0;
}

int usb_allfree(int chip) {
	int	num;

	chip_status[chip] = 0;
	for(num = 1;num < USBNUM;num++) {
		if(info[num + chip * USBNUM].fd != 0) __close(SuperTask, info[num + chip * USBNUM].fd);
		info[num + chip * USBNUM].fd = 0;
		info[num + chip * USBNUM].use = 0;
		info[num + chip * USBNUM].handle = 0;
		info[num + chip * USBNUM].class = 0;
		info[num + chip * USBNUM].sub = 0;
		info[num + chip * USBNUM].epnum = 0;
	}
	return 0;
}

int usb_eject(int chip) {
	int	num;

	if(info[1 + chip * USBNUM].fd == 0) return -1;
	chip_status[chip] = 0;
	eject("spc0");
	eject("spc1");
	eject("spc2");
	eject("spc3");
	usb_allfree(chip);
	return 0;
}

int usb_status(int num) {
	if(num <= 0 || num >= USBNUM * SLNUM) return -1;
	if(info[num].fd == 0) return -1;
	if(info[num].use != 0) return -1;
	return info[num].use;
}

void usb_init(void) {
	int	num;

	buf = __malloc(SuperTask, 256 + sizeof(USBInfo) * USBNUM * SLNUM);
	info = (USBInfo*)&buf[256];
	for(num = 0;num < USBNUM * SLNUM;num++) {
		if((num % USBNUM) == 0) chip_status[num / USBNUM] = 0;
		info[num].fd = 0;
		info[num].use = 0;
		info[num].handle = 0;
		info[num].class = 0;
		info[num].sub = 0;
		info[num].epnum = 0;
	}
}

/*****************************************/
/************** USB TASK *****************/
/*****************************************/
void usb_interval(void) {
	int		diff, num, chip;
//	static	int	time = 0;

	for(chip = 0;chip < SLNUM;chip++) {
		if(chip_status[chip] == 0) continue;
		for(num = 1;num < USBNUM;num++) {
			if(info[num + chip * USBNUM].use == 1) {
//				command_trap(_ioctl_, info[num + chip * USBNUM].fd, 0, USB_INTERRUPT);
				if(info[num + chip * USBNUM].handle != 0) (*info[num + chip * USBNUM].handle)(info[num + chip * USBNUM].minor);
			}
		}
	}
}
