/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   terminal.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 17:52:43 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/20 18:32:51 by eblondee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>

#define VGA_WIDTH		80
#define VGA_HEIGHT		25
#define VGA_ADDRESS		0xB8000
#define EMPTY_CHAR		0b0000111100100000	// ' ' in default color

static size_t				x;
static size_t				y;
static volatile uint16_t	*visible_terminal = (uint16_t *)VGA_ADDRESS;

static inline uint16_t	encode_char(unsigned char c, uint8_t color)
{
	return (color << 8 | c);
}

static inline uint8_t	setup_color(uint8_t front, uint8_t back)
{
	return (back << 4 | front);
}

static void	verify_pos()
{
	if (x >= VGA_WIDTH)
	{
		x = 0;
		y ++;
	}
	if (y >= VGA_HEIGHT)
		y = 0; // TODO : SCROLL
}

void	putchar_color(unsigned char c, uint8_t color)
{
	visible_terminal[y * VGA_WIDTH + x ++] = encode_char(c, color);
	verify_pos();
}

void	putchar(unsigned char c)
{
	visible_terminal[y * VGA_WIDTH + x ++] = encode_char(c, DEFAULT_COLOR);
	verify_pos();
}

void	write_color(const char *str, uint8_t color)
{
	for (size_t i = 0; str[i]; ++i)
		putchar_color(str[i], color);
}

void	write(const char *str)
{
	for (size_t i = 0; str[i]; ++i)
		putchar(str[i]);
}

void	newline(void)
{
	x = 0;
	y ++;
	verify_pos();
}

void	set_pos(size_t nx, size_t ny)
{
	x = nx;
	y = ny;
	verify_pos();
}

void	clear_line(size_t target_y)
{
	size_t	offset = target_y * VGA_WIDTH;
	for (size_t i = 0; i < VGA_WIDTH; ++i)
		visible_terminal[offset + i] = EMPTY_CHAR;
	if (target_y == y)
		x = 0;
}

void	clear_screen()
{
	for (size_t index = 0; index < VGA_HEIGHT * VGA_WIDTH; ++index)
		visible_terminal[index] = encode_char(' ', DEFAULT_COLOR);

	x = 0;
	y = 0;
}

size_t	get_x()
{
	return (x);
}

size_t	get_y()
{
	return (y);
}

void	init_terminal(void)
{
	x = 0;
	y = 0;

	clear_screen();
}
