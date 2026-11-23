/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   terminal.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 17:52:43 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/23 20:32:25 by eblondee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>

#define FULL_WIDTH		80
#define FULL_HEIGHT		100
#define VGA_ADDRESS		0xB8000
#define EMPTY_CHAR		0b0000111100100000	// ' ' in default color

static size_t				x;
static size_t				y;
static volatile uint16_t	*visible_terminal = (uint16_t *)VGA_ADDRESS;

static size_t				scroll_y;
static size_t				offset_y;
static volatile uint16_t	buffer_terminal[FULL_HEIGHT * FULL_WIDTH] = {0};

static void	evil_scroll_down(void);

static int	cursos_status;

static inline uint16_t	encode_char(unsigned char c, uint8_t color)
{
	return (color << 8 | c);
}

static inline uint8_t	setup_color(uint8_t front, uint8_t back)
{
	return (back << 4 | front);
}

inline uint16_t	invert_color(uint16_t c)
{
	return (((c & 01111000000000000) >> 4) | ((c & 00000111100000000) << 4) | (c & 0b11111111));
}

void	toggle_cursor(void)
{
	cursos_status = !cursos_status;
	if (cursos_status)
	{
		write_port(0x3D4, 0x0A);
		uint8_t cursor_start = read_port(0x3D5);
		write_port(0x3D5, cursor_start & ~0x20);
	}
	else
	{
		write_port(0x3D4, 0x0A);
		uint8_t cursor_start = read_port(0x3D5);
		write_port(0x3D5, cursor_start | 0x20);
	}
}

void	set_cursor_pos(int x, int y)
{
	uint16_t pos = y * VGA_WIDTH + x;

	write_port(0x3D4, 0x0F);
	write_port(0x3D5, (pos & 0xFF));
	write_port(0x3D4, 0x0E);
	write_port(0x3D5, ((pos >> 8) & 0xFF));
}

uint16_t	get_cursor_pos(void)
{
    uint16_t pos = 0;
    write_port(0x3D4, 0x0F);
    pos |= read_port(0x3D5);
    write_port(0x3D4, 0x0E);
    pos |= ((uint16_t)read_port(0x3D5)) << 8;
    return pos;
}

void	move_cursor(int move_x, int move_y)
{
	uint16_t	raw_pos = get_cursor_pos();
	int			pos_x = (int)(raw_pos % VGA_WIDTH) + move_x;
	int			pos_y = (int)(raw_pos / VGA_WIDTH) + move_y;

	if (pos_x < 0 || pos_y < 0)
		return ;
	if (pos_x >= VGA_WIDTH)
		pos_x = VGA_WIDTH - 1;

	set_cursor_pos(pos_x, pos_y);
}

static void	verify_pos(void)
{
	if (x >= VGA_WIDTH)
	{
		move_cursor(-(x - 1), 1);
		x = 0;
		y ++;
	}
	if (y >= VGA_HEIGHT)
	{
		evil_scroll_down();
		y = VGA_HEIGHT - 1;
	}
}

void	scroll_up(void)
{
	if (scroll_y == 0)
		return ;
	scroll_y --;
	offset_y ++;

	size_t	buffer_offset = scroll_y * FULL_WIDTH;
	for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; ++i)
		visible_terminal[i] = buffer_terminal[buffer_offset + i];

	move_cursor(0, 1);
}

void	scroll_down(void)
{
	if (scroll_y == FULL_HEIGHT - VGA_HEIGHT || offset_y == 0)
		return ;
	scroll_y ++;
	offset_y --;

	size_t	buffer_offset = scroll_y * FULL_WIDTH;
	for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; ++i)
		visible_terminal[i] = buffer_terminal[buffer_offset + i];

	move_cursor(0, -1);
}

static void	evil_scroll_down(void)
{
	if (scroll_y == FULL_HEIGHT - VGA_HEIGHT)
	{
		for (size_t i = 0; i < FULL_HEIGHT * FULL_WIDTH - FULL_WIDTH; ++i)
			buffer_terminal[i] = buffer_terminal[i + FULL_WIDTH];
		clear_line(VGA_HEIGHT - 1);
		size_t	buffer_offset = scroll_y * FULL_WIDTH;
		for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; ++i)
			visible_terminal[i] = buffer_terminal[buffer_offset + i];
		return ;
	}
	scroll_y ++;

	size_t	buffer_offset = scroll_y * FULL_WIDTH;
	for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; ++i)
		visible_terminal[i] = buffer_terminal[buffer_offset + i];

	move_cursor(0, 1);
}

void	putchar_color(unsigned char c, uint8_t color)
{
	while (offset_y > 0)
		scroll_down();
	if (c == '\n')
		newline();

	visible_terminal[y * VGA_WIDTH + x] = encode_char(c, color);
	buffer_terminal[(y + scroll_y) * FULL_WIDTH + x] = encode_char(c, color);
	x ++;
	move_cursor(1, 0);
	verify_pos();
}

void	putchar(unsigned char c)
{
	while (offset_y > 0)
		scroll_down();
	if (c == '\n')
		newline();

	visible_terminal[y * VGA_WIDTH + x] = encode_char(c, DEFAULT_COLOR);
	buffer_terminal[(y + scroll_y) * FULL_WIDTH + x] = encode_char(c, DEFAULT_COLOR);
	x ++;
	move_cursor(1, 0);
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
	set_cursor_pos(x, y);
}

void	tabulation(void)
{
	x = (x + 4) & ~3;
	verify_pos();
	set_cursor_pos(x, y);
}

void	set_pos(size_t nx, size_t ny)
{
	x = nx;
	y = ny;
	verify_pos();
	set_cursor_pos(x, y);
}

void	clear_line(size_t target_y)
{
	size_t	offset = target_y * VGA_WIDTH;
	size_t	buffer_offset = offset + (scroll_y * FULL_WIDTH);
	for (size_t i = 0; i < VGA_WIDTH; ++i)
	{
		visible_terminal[offset + i] = EMPTY_CHAR;
		buffer_terminal[buffer_offset + i] = EMPTY_CHAR;
	}
	if (target_y == y)
		x = 0;
}

void	clear_zone(size_t target_x, size_t target_y, size_t size_x, size_t size_y)
{
	for (size_t index_y = 0; index_y < size_y; ++index_y)
	{
		for (size_t index_x = 0; index_x < size_x; ++index_x)
		{
			visible_terminal[(target_y + index_y) * VGA_WIDTH + (target_x + index_x)] = EMPTY_CHAR;
			buffer_terminal[(target_y + index_y + scroll_y) * FULL_WIDTH + (target_x + index_x)] = EMPTY_CHAR;
		}
	}
}

void	clear_screen(void)
{
	for (size_t index = 0; index < VGA_HEIGHT * VGA_WIDTH; ++index)
		visible_terminal[index] = encode_char(' ', DEFAULT_COLOR);

	for (size_t index = 0; index < FULL_HEIGHT * FULL_WIDTH; ++index)
		buffer_terminal[index] = encode_char(' ', DEFAULT_COLOR);

	x = 0;
	y = 0;
}

void	erase_char(void)
{
	if (x == 0) // TODO : BETTER
	{
		if (y == 0)
			return ;
		x = FULL_WIDTH - 1;
		y --;
		move_cursor(x, -1);
		visible_terminal[y * VGA_WIDTH + x] = EMPTY_CHAR;
		buffer_terminal[(y + scroll_y) * FULL_WIDTH + x] = EMPTY_CHAR;
		return ;
	}

	move_cursor(-1, 0);
	x --;

	visible_terminal[y * VGA_WIDTH + x] = EMPTY_CHAR;
	buffer_terminal[(y + scroll_y) * FULL_WIDTH + x] = EMPTY_CHAR;
}

uint16_t	get_curr_char(void)
{
	return (visible_terminal[y * VGA_HEIGHT + x]);
}

size_t	get_x(void)
{
	return (x);
}

size_t	get_y(void)
{
	return (y);
}

void	init_terminal(void)
{
	x = 0;
	y = 0;
	scroll_y = 0;
	offset_y = 0;
	cursos_status = 1;

	clear_screen();
	set_cursor_pos(0, 0);
}
