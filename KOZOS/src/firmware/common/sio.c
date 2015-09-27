#include "reg3067.h"

#define TDRE	0x80
#define RDRF	0x40
#define ORER	0x20
#define FER	0x10
#define PER	0x08
#define TEND	0x04
#define TIE	0x80
#define RIE	0x40
#define TE	0x20
#define RE	0x10

void sio_init() {
	volatile int	w;

	SMR1 = 0;
	SCR1 = 0;
	BRR1 = 10;
	for(w = 0;w < 280;w++);
	SCR1 = RE | TE;
	w = SSR1;
	SSR1 = TDRE;
}

int sio_write(char *buffer, int size) {
	int	i;

	for(i = 0;i < size;i++) {
		if(buffer[i] == '\n') {
			while(!(SSR1 & TDRE));
			TDR1 = '\r';
			SSR1 &= ~TDRE;
		}
		while(!(SSR1 & TDRE));
		TDR1 = buffer[i];
		SSR1 &= ~TDRE;
	}
	while(!(SSR1 & TEND));
	return 0;
}

int sio_read(char *buffer, int size) {
	int	i, n;
	char	c;

	n = 0;
	for(i = 0;i < size;i++) {
		if(SSR1 & (ORER | FER | PER)) {
			SSR1 &= ~(ORER | FER | PER);
			return 0;
		}
		if(SSR1 & RDRF) {
			buffer[n++] = RDR1;
			SSR1 &= ~RDRF;
		} else {
			return n;
		}
	}
	return n;
}

int sio_speed(int speed, int freq) {
	unsigned int	i, j, x, y, b, s;

	s = 32;
	for(i = 0;i < 4;i++) {
	//	x = freq * 10000000;
	//	y = s * speed;
	//	b = x / y + 5;
	//	b = b / 10 - 1;
		for(j = 0; j < freq; j++){
			x += 10000000;
		}
		for(j = 0; j < s; j++){
			y += speed;
		}
		for(b = 0; x < y; b++){
			x -= y;
		}
		b += 5;
		for(j = 0; b < 10; j++){
			b -= 10;
		}
		b -= 1;
		
		if(b <= 255) break;
		s <<= 2;
	}
	if(i == 4) return -1;
	SMR1 &= ~0x03;
	SMR1 |= i;
	BRR1 = (unsigned char)b;
	return 0;
}

