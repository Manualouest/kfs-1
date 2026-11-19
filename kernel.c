#include <stdint.h>
#include <stddef.h>


// ######################################################
// 			VGA PART
// ######################################################

enum vga_color {
	BLACK = 0,
	BLUE = 1,
	GREEN = 2,
	CYAN = 3,
	RED = 4,
	MAGENTA = 5,
	BROWN = 6,
	LIGHT_GREY = 7,
	DARK_GREY = 8,
	LIGHT_BLUE = 9,
	LIGHT_GREEN = 10,
	LIGHT_CYAN = 11,
	LIGHT_RED = 12,
	LIGHT_MAGENTA = 13,
	LIGHT_BROWN = 14,
	WHITE = 15,
};

#define VGA_WIDTH		80
#define VGA_HEIGHT		25
#define VGA_ADDRESS		0xB8000
#define DEFAULT_COLOR   (BLACK << 4 | WHITE)

static uint16_t	x; // TODO : size_t ?
static uint16_t	y; // TODO : size_t ?
static volatile uint16_t	*visible_terminal = (uint16_t *)VGA_ADDRESS;

uint16_t	encode_char(unsigned char c, uint8_t color)
{
	return (color << 8 | c);
}

uint8_t	setup_color(uint8_t front, uint8_t back)
{
	return (back << 4 | front);
}

void	init_terminal(void)
{
	x = 0;
	y = 0;

	for (size_t index = 0; index < VGA_HEIGHT * VGA_WIDTH; ++index)
		visible_terminal[index] = encode_char(' ', DEFAULT_COLOR);
}

void	putchar(uint16_t c)
{
	visible_terminal[y * VGA_WIDTH + x ++] = c;
	if (x >= VGA_WIDTH)
	{
		x = 0;
		y ++;
	}
	if (y >= VGA_HEIGHT)
		y = 0; // TODO : SCROLL
}

void	write(const char *str, uint8_t color)
{
	for (size_t i = 0; str[i]; ++i)
		putchar(encode_char(str[i], color));
}

void	newline(void)
{
	x = 0;
	y ++;
	if (y >= VGA_HEIGHT)
		y = 0;
}

void	set_pos(uint16_t nx, uint16_t ny)
{
	x = nx;
	y = ny;
	if (x >= VGA_WIDTH)
	{
		x = 0;
		y ++;
	}
	if (y >= VGA_HEIGHT)
		y = 0;
}

// ######################################################
// 			VGA PART
// ######################################################



// ######################################################
// 			KEYBOARD PART
// ######################################################

// port 0x64 is to read status or send commands
// port 0x60 is to read data received/return of command or send second part of command

uint8_t read_port(uint16_t port)
{
    uint8_t ret;
	__asm__ volatile ("inb %%dx, %%al"
						: "=a"(ret)
						: "d"(port));
	return (ret);
}

void write_port(uint16_t port, uint8_t data)
{
	__asm__ volatile ("outb %0, %1"
						:
						: "a"(data), "Nd"(port));
}

// waits for the command register to send something in the Data Port
void	wait_controller_input()
{
	while (!(read_port(0x64) & 1))
		;
}

// waits for the command register to be ready to receive input
void	wait_controller_ready()
{
	while (read_port(0x64) & 2)
		;
}

// ######################################################
// 			KEYBOARD PART
// ######################################################


void	write_byte(uint8_t byte)
{
	for (int i = 0; i < 8; ++i)
		putchar(encode_char(((byte >> (7 - i)) & 1) + '0', DEFAULT_COLOR));
}



void	kmain(void)
{
	init_terminal();

	// disabeling devices
	wait_controller_ready();	write_port(0x64, 0xAD);
	wait_controller_ready();	write_port(0x64, 0xA7);

	// flushing the output buffer
	read_port(0x60);

	// setup controller config
	wait_controller_ready();	write_port(0x64, 0x20);
	wait_controller_input();	uint8_t	controller_config_byte = read_port(0x60);
	controller_config_byte &= ~1 & ~(1 << 1) & ~(1 << 4) & ~(1 << 6);
	wait_controller_ready();	write_port(0x64, 0x60);
	wait_controller_ready();	write_port(0x60, controller_config_byte);

	// controller self test
	wait_controller_ready();	write_port(0x64, 0xAA);
	wait_controller_input();	write("return of 0xAA: ", DEFAULT_COLOR);	write_byte(read_port(0x60));	newline();
	wait_controller_ready();	write_port(0x64, 0x60); // need to restore controller config byte
	wait_controller_ready();	write_port(0x60, controller_config_byte);

	// check if there are two channels
	wait_controller_ready();	write_port(0x64, 0xA8);
	wait_controller_ready();	write_port(0x64, 0x20);
	wait_controller_input();	controller_config_byte = read_port(0x60);	write("return of 0x20: ", DEFAULT_COLOR);	write_byte(controller_config_byte);	newline();
	wait_controller_ready();	write_port(0x64, 0xA7);
	controller_config_byte &= ~(1 << 1) & ~(1 << 4) & ~(1 << 5);
	wait_controller_ready();	write_port(0x64, 0x60);
	wait_controller_ready();	write_port(0x60, controller_config_byte);

	// perform interface tests
	wait_controller_ready();	write_port(0x64, 0xAB);
	wait_controller_input();	write("return of 0xAB: ", DEFAULT_COLOR);	write_byte(read_port(0x60));	newline();
	wait_controller_ready();	write_port(0x64, 0xA9);
	wait_controller_input();	write("return of 0xA9: ", DEFAULT_COLOR);	write_byte(read_port(0x60));	newline();

	// enable the devices
	wait_controller_ready();	write_port(0x64, 0xAE);
	wait_controller_ready();	write_port(0x64, 0xA8);

	wait_controller_ready();	write_port(0x64, 0x20);
	wait_controller_input();	controller_config_byte = read_port(0x60) | 1 | (1 << 1);
	wait_controller_ready();	write_port(0x64, 0x60);
	wait_controller_ready();	write_port(0x60, controller_config_byte);

	// reset devices
	wait_controller_ready();	write_port(0x64, 0xFF);


	// activate mouse
	wait_controller_ready();	write_port(0x64, 0xD4);
	wait_controller_ready();	write_port(0x60, 0xF2);		wait_controller_input();	read_port(0x60);
	wait_controller_input();	write("return of 0xF2: ", DEFAULT_COLOR);	write_byte(read_port(0x60));	newline();

	wait_controller_ready();	write_port(0x64, 0xD4);
	wait_controller_ready();	write_port(0x60, 0xF3);		wait_controller_input();	read_port(0x60);
	wait_controller_ready();	write_port(0x64, 0xD4);
	wait_controller_ready();	write_port(0x60, 0xC8);		wait_controller_input();	read_port(0x60);

	wait_controller_ready();	write_port(0x64, 0xD4);
	wait_controller_ready();	write_port(0x60, 0xF3);		wait_controller_input();	read_port(0x60);
	wait_controller_ready();	write_port(0x64, 0xD4);
	wait_controller_ready();	write_port(0x60, 0x64);		wait_controller_input();	read_port(0x60);

	wait_controller_ready();	write_port(0x64, 0xD4);
	wait_controller_ready();	write_port(0x60, 0xF3);		wait_controller_input();	read_port(0x60);
	wait_controller_ready();	write_port(0x64, 0xD4);
	wait_controller_ready();	write_port(0x60, 0x50);		wait_controller_input();	read_port(0x60);

	wait_controller_ready();	write_port(0x64, 0xD4);
	wait_controller_ready();	write_port(0x60, 0xF2);		wait_controller_input();	read_port(0x60);
	wait_controller_input();	write("return of 0xF2: ", DEFAULT_COLOR);	write_byte(read_port(0x60));	newline();

	wait_controller_ready();	write_port(0x64, 0xD4);
	wait_controller_ready();	write_port(0x60, 0xF4);		wait_controller_input();	read_port(0x60);

	uint8_t	old_wheel = 0;
	while (1)
	{
		set_pos(0, 10);
		wait_controller_input();	write_byte(read_port(0x60));	newline();
		wait_controller_input();	write_byte(read_port(0x60));	newline();
		wait_controller_input();	write_byte(read_port(0x60));	newline();
		wait_controller_input();	uint8_t	mousewheel = read_port(0x60);
		if (mousewheel != 0)
			old_wheel = mousewheel;
		write_byte(old_wheel);	newline();
		newline();
		wait_controller_input();	write_byte(read_port(0x64));	newline();
	}
}
