
#include <stdint.h>


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

uint16_t	x;
uint16_t	y;
uint16_t	*visible_terminal = (uint16_t *)VGA_ADDRESS;

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

	for (uint16_t index = 0; index < VGA_HEIGHT * VGA_WIDTH + VGA_WIDTH; ++index)
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
		y = 0;
}

void	write(const char *str, uint8_t color)
{
	for (int i = 0; str[i]; ++i)
		putchar(encode_char(str[i], color));
}

void	newline()
{
	x = 0;
	y ++;
	if (y >= VGA_HEIGHT)
		y = 0;
}

// ######################################################
// 			VGA PART
// ######################################################

void	kmain(void)
{
	init_terminal();
	write("Hello", DEFAULT_COLOR);
}