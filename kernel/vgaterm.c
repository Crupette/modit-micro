#include "kernel/vgaterm.h"
#include "kernel/io.h"

static size_t vga_row;
static size_t vga_col;
static uint8_t vga_color;
static uint8_t *vga_buffer;

#define SERIAL_CMD(port)	(port + 1)
#define SERIAL_FIFO(port)	(port + 2)
#define SERIAL_LCR(port)	(port + 3)
#define SERIAL_MODEM(port)	(port + 4)
#define SERIAL_LINE_STAT(port)	(port + 5)
#define SERIAL_MODEM_STAT(port)	(port + 6)
#define SERIAL_SCRATCH(port)	(port + 7)

void vgaterm_init(void){
	vga_row = vga_col = 0;
	vga_color = 0x0F;
	vga_buffer = (uint8_t*)0xE00B8000;
	memset(vga_buffer, 0, VGA_WIDTH * VGA_HEIGHT * 2);

	//Debug serial
	outb(SERIAL_CMD(0x3F8), 0);
	outb(SERIAL_LCR(0x3F8), 0x80);
	outb(0x3F8, 0x03);
	outb(SERIAL_CMD(0x3F8), 0x0);
	outb(SERIAL_LCR(0x3F8), 0x03);
	outb(SERIAL_FIFO(0x3F8), 0xC7);
	outb(SERIAL_MODEM(0x3F8), 0x0B);
	outb(SERIAL_CMD(0x3F8), 0x01);
}

void vgaterm_setc(uint8_t c){
	vga_color = c;
}

void vgaterm_setent(uint8_t col, uint8_t row, int8_t c, uint8_t color){
	const uint16_t index = row * VGA_WIDTH + col;
	vga_buffer[index * 2] = c;
	vga_buffer[index * 2 + 1] = color;
}

static void scroll(){
	for(int i = 1; i < VGA_HEIGHT; i++){
		memcpy(vga_buffer + ((i - 1) * (VGA_WIDTH * 2)), vga_buffer + (i * (VGA_WIDTH * 2)), VGA_WIDTH*2);
	}
	memset(vga_buffer + ((VGA_HEIGHT - 1) * (VGA_WIDTH * 2)), 0, VGA_WIDTH * 2);
}

void vgaterm_putent(int8_t c, uint8_t color){
	//Debug serial
	if(c == '\n'){
		while((inb(SERIAL_LINE_STAT(0x3F8)) & 0x20) == 0);
		outb(0x3F8, '\r');
	}
	while((inb(SERIAL_LINE_STAT(0x3F8)) & 0x20) == 0);
	outb(0x3F8, c);
	//End

	//Special characters need to be handled differently
	switch(c){
		case '\n':
			vga_row++;
			vga_col = 0;
		break;
		case '\b':
			if(vga_col == 0) { vga_col = VGA_WIDTH - 1; vga_row--; }
		break;
		default:
			vgaterm_setent(vga_col, vga_row, c, color);
			vga_col++;
		break;
	}
	if(vga_col >= VGA_WIDTH) {
		vga_col = 0;
		vga_row++;
	}
	if(vga_row >= VGA_HEIGHT){
		vga_row--;
		scroll();
	}
}

void vgaterm_putc(int8_t c){
	vgaterm_putent(c, vga_color);
}

void vgaterm_putstr(char *str){
	while(*str != 0){
		vgaterm_putc(*str);
		str++;
	}
}
