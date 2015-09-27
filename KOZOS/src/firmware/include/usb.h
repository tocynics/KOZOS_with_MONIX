#define TRANS_TO_DEV	0x00
#define TRANS_TO_HOST	0x80
#define TRANS_TO_IF	0x01
#define TRANS_TO_EP	0x02
#define TRANS_TO_OTHER	0x03
#define TYPE_STD	0x00
#define TYPE_CLASS	0x20
#define TYPE_VENDOR	0x40

#define GET_STATUS      0x00
#define CLEAR_FEATURE   0x01
#define SET_FEATURE     0x03
#define SET_ADDRESS     0x05
#define GET_DESCRIPTOR  0x06
#define SET_DESCRIPTOR  0x07
#define GET_CONFIG      0x08
#define SET_CONFIG      0x09
#define GET_INTERFACE   0x0a
#define SET_INTERFACE   0x0b
#define SYNCH_FRAME     0x0c

#define DEVICE_TYPE	0x01
#define CONFIG_TYPE	0x02
#define STRING_TYPE	0x03
#define INTERFACE_TYPE	0x04
#define ENDPOINT_TYPE	0x05

#define STDCLASS	0x00
#define HIDCLASS	0x03
#define MSSCLASS	0x08
#define HUBCLASS	0x09

#define BIT_PORT_CONNECT 0x0001
#define BIT_PORT_ENABLE	0x0002
#define BIT_PORT_RESET	0x0010
#define BIT_PORT_POWER	0x0100
#define PORT_CONNECT	0
#define PORT_ENABLE	1
#define PORT_RESET	4
#define PORT_POWER	8
#define C_PORT_CONNECT	16

#define MAX_EP		8

typedef struct {
	unsigned char	bmRequest;
	unsigned char	bRequest;
	unsigned short	wValue;
	unsigned short	wIndex;
	unsigned short	wLength;
} SetupPKG;

typedef struct {
	unsigned char	bLength;
	unsigned char	bDescriptorType;
	unsigned short	bcdUSB;
	unsigned char	bDeviceClass;
	unsigned char	bDeviceSubClass;
	unsigned char	bDeviceProtocol;
	unsigned char	bMaxPacketSize0;
	unsigned short	idVendor;
	unsigned short	idProduct;
	unsigned short	bcdDevice;
	unsigned char	iManufacturer;
	unsigned char	iProduct;
	unsigned char	iSerialNumber;
	unsigned char	bNumConfigurations;
} DevDesc;

// Standard Configuration Descriptor
typedef struct {
	unsigned char	bLength;	// Size of descriptor in unsigned char
	unsigned char	bType;		// Configuration
	unsigned short	wLength;	// Total length
	unsigned char	bNumIntf;	// Number of interface
	unsigned char	bCV;		// bConfigurationValue
	unsigned char	bIndex;		// iConfiguration
	unsigned char	bAttr;		// Configuration Characteristic
	unsigned char	bMaxPower;	// Power config
} CfgDesc;

// Standard Interface Descriptor
typedef struct {
	unsigned char	bLength;
	unsigned char	bType;
	unsigned char	iNum;
	unsigned char	iAltString;
	unsigned char	bEndPoints;
	unsigned char	iClass;
	unsigned char	iSub; 
	unsigned char	iProto;
	unsigned char	iIndex; 
} IntfDesc;

// Standard EndPoint Descriptor
typedef struct{
	unsigned char	bLength;
	unsigned char	bType;
	unsigned char	bEPAdd;
	unsigned char	bAttr;
	unsigned short	wPayLoad;	// low-speed this must be 0x08
	unsigned char	bInterval;
} EPDesc;

// Standard String Descriptor
typedef struct {
	unsigned char	bLength;
	unsigned char	bType;
	unsigned short	wLang;
} StrDesc;
