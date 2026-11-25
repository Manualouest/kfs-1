/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   terminal.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 17:52:43 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/25 21:01:53 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>
#include <art.h>

#define FULL_WIDTH		80
#define FULL_HEIGHT		100
#define NB_WINDOWS		5
#define VGA_ADDRESS		0xB8000
#define EMPTY_CHAR		0b0000111100100000	// ' ' in default color

static size_t				x;
static size_t				y;
static volatile uint16_t	*visible_terminal = (uint16_t *)VGA_ADDRESS;

static size_t				scroll_y;
static size_t				offset_y;
static size_t				curr_window;
static volatile uint16_t	buffer_terminal[NB_WINDOWS][FULL_HEIGHT * FULL_WIDTH] = {0};

static volatile uint16_t	clipboard[VGA_HEIGHT + 1][VGA_WIDTH + 1] = {0};

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
	return (((c & 0b1111000000000000) >> 4) | ((c & 0b0000111100000000) << 4) | (c & 0b11111111));
}

static size_t	find_last_char(size_t line)
{
	size_t	line_x = VGA_WIDTH;

	for (; line_x > 0; --line_x)
		if ((visible_terminal[line * VGA_WIDTH + line_x] & 0xFF) != ' ')
			break;
	return (line_x + (line_x != VGA_WIDTH - 1 && line_x != 0));
}

void	switch_window(int way)
{
	if (curr_window == 0 && way < 0)
		curr_window = NB_WINDOWS - 1;
	else
		curr_window += way;
	if (curr_window >= NB_WINDOWS)
		curr_window = 0;
	
	for (size_t index = 0; index < VGA_HEIGHT * VGA_WIDTH; ++index)
		visible_terminal[index] = encode_char(' ', DEFAULT_COLOR);

	set_pos(0, 0);
	scroll_y = 0;
	offset_y = 0;

	int	len = FULL_HEIGHT * FULL_WIDTH - 1;
	while (len >= 0 && (buffer_terminal[curr_window][len] & 0xFF) == ' ')
		len --;
	int	i = 0;
	while (i <= len)
	{
		putchar_color(buffer_terminal[curr_window][i], buffer_terminal[curr_window][i] >> 8);
		i ++;
	}
	set_pos(x, y);
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
		x = 0;
		y ++;
		set_cursor_pos(x, y);
	}
	if (y >= VGA_HEIGHT)
	{
		evil_scroll_down();
		y = VGA_HEIGHT - 1;
		set_cursor_pos(x, y);
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
		visible_terminal[i] = buffer_terminal[curr_window][buffer_offset + i];

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
		visible_terminal[i] = buffer_terminal[curr_window][buffer_offset + i];

	move_cursor(0, -1);
}

static void	evil_scroll_down(void)
{
	if (scroll_y == FULL_HEIGHT - VGA_HEIGHT)
	{
		for (size_t i = 0; i < FULL_HEIGHT * FULL_WIDTH - FULL_WIDTH; ++i)
			buffer_terminal[curr_window][i] = buffer_terminal[curr_window][i + FULL_WIDTH];
		clear_line(VGA_HEIGHT - 1);
		size_t	buffer_offset = scroll_y * FULL_WIDTH;
		for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; ++i)
			visible_terminal[i] = buffer_terminal[curr_window][buffer_offset + i];
		return ;
	}
	scroll_y ++;

	size_t	buffer_offset = scroll_y * FULL_WIDTH;
	for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; ++i)
		visible_terminal[i] = buffer_terminal[curr_window][buffer_offset + i];

	move_cursor(0, 1);
}

void	clipboard_to_cursor()
{
	for (size_t i = 0; i < VGA_HEIGHT; ++i)
	{
		for (size_t ii = 0; clipboard[i][ii]; ++ii)
			putchar_color(clipboard[i][ii], clipboard[i][ii] >> 8);
		if (clipboard[i + 1][0])
			newline();
	}
}

void	region_to_clipboard(int8_t highlight_start[2], int8_t highlight_end[2])
{
	int8_t	start[2] = {highlight_start[0], highlight_start[1]};
	int8_t	end[2] = {highlight_end[0], highlight_end[1]};
	if (start[1] == end[1] && start[0] > end[0])
	{
		uint8_t	tmp = start[0];
		start[0] = end[0];
		end[0] = tmp;
	}
	if (start[1] > end[1])
	{
		uint8_t	tmp = start[1];
		start[1] = end[1];
		end[1] = tmp;
		tmp = start[0];
		start[0] = end[0];
		end[0] = tmp;
	}

	for (size_t i = 0; i < VGA_HEIGHT; ++i)
		for (size_t ii = 0; ii < VGA_WIDTH; ++ii)
			clipboard[i][ii] = 0;

	int	clip_i = end[1] - start[1];
	int	clip_ii = end[0];
	if (start[1] == end[1])
		clip_ii = end[0] - start[0];
	int	in_empty = 1;
	for (int i = end[1] * VGA_WIDTH + end[0]; i >= start[1] * VGA_WIDTH + start[0]; --i)
	{
		if (in_empty && (visible_terminal[i] & 0xFF) == ' ')
			clipboard[clip_i][clip_ii] = 0;
		else
		{
			in_empty = 0;
			if (visible_terminal[i] >> 12)
				clipboard[clip_i][clip_ii] = invert_color(visible_terminal[i]);
			else
				clipboard[clip_i][clip_ii] = visible_terminal[i];
		}
		clip_ii --;
		if (clip_ii < 0)
		{
			clip_i --;
			clip_ii = VGA_WIDTH - 1;
			in_empty = 1;
		}
	}
}

void	highlight_char(int8_t pos[2], STATE state)
{
	if (state && !(visible_terminal[pos[1] * VGA_WIDTH + pos[0]] >> 12))
		visible_terminal[pos[1] * VGA_WIDTH + pos[0]] = invert_color(visible_terminal[pos[1] * VGA_WIDTH + pos[0]]);
	else if (!state && (visible_terminal[pos[1] * VGA_WIDTH + pos[0]] >> 12))
		visible_terminal[pos[1] * VGA_WIDTH + pos[0]] = invert_color(visible_terminal[pos[1] * VGA_WIDTH + pos[0]]);
}

void	highlight_region(int8_t highlight_start[2], int8_t highlight_end[2], STATE state)
{
	int8_t	start[2] = {highlight_start[0], highlight_start[1]};
	int8_t	end[2] = {highlight_end[0], highlight_end[1]};
	if (start[1] == end[1] && start[0] > end[0])
	{
		uint8_t	tmp = start[0];
		start[0] = end[0];
		end[0] = tmp;
	}
	if (start[1] > end[1])
	{
		uint8_t	tmp = start[1];
		start[1] = end[1];
		end[1] = tmp;
		tmp = start[0];
		start[0] = end[0];
		end[0] = tmp;
	}
	for (int i = start[1] * VGA_WIDTH + start[0]; i <= end[1] * VGA_WIDTH + end[0]; ++i)
	{
		if (state && !(visible_terminal[i] >> 12))
			visible_terminal[i] = invert_color(visible_terminal[i]);
		else if (!state && (visible_terminal[i] >> 12))
			visible_terminal[i] = invert_color(visible_terminal[i]);
	}
}

static int	is_special_char(unsigned char c)
{
	switch (c)
	{
		case '\n':
			newline();
			break;
		case '\t':
			tabulation();
			break;
		
		default:
			return (0);
	}
	return (1);
}

void	putchar_color(unsigned char c, uint8_t color)
{
	while (offset_y > 0)
		scroll_down();
	if (is_special_char(c))
		return ;

	visible_terminal[y * VGA_WIDTH + x] = encode_char(c, color);
	buffer_terminal[curr_window][(y + scroll_y) * FULL_WIDTH + x] = encode_char(c, color);
	x ++;
	move_cursor(1, 0);
	verify_pos();
}

void	putchar(unsigned char c)
{
	while (offset_y > 0)
		scroll_down();
	if (is_special_char(c))
		return ;

	visible_terminal[y * VGA_WIDTH + x] = encode_char(c, DEFAULT_COLOR);
	buffer_terminal[curr_window][(y + scroll_y) * FULL_WIDTH + x] = encode_char(c, DEFAULT_COLOR);
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
		buffer_terminal[curr_window][buffer_offset + i] = EMPTY_CHAR;
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
			buffer_terminal[curr_window][(target_y + index_y + scroll_y) * FULL_WIDTH + (target_x + index_x)] = EMPTY_CHAR;
		}
	}
}

void	clear_screen(void)
{
	for (size_t index = 0; index < VGA_HEIGHT * VGA_WIDTH; ++index)
		visible_terminal[index] = encode_char(' ', DEFAULT_COLOR);

	for (size_t window = 0; window < NB_WINDOWS; ++window)
		for (size_t index = 0; index < FULL_HEIGHT * FULL_WIDTH; ++index)
			buffer_terminal[window][index] = encode_char(' ', DEFAULT_COLOR);

	for (size_t i = 0; i < VGA_HEIGHT + 1; ++i)
		for (size_t ii = 0; ii < VGA_WIDTH + 1; ++ii)
			clipboard[i][ii] = encode_char(0, 0);

	x = 0;
	y = 0;
}

void	erase_char(void)
{
	if (x == 0)
	{
		if (y == 0)
			return ;
		y --;
		x = find_last_char(y);
		move_cursor(x, -1);
		visible_terminal[y * VGA_WIDTH + x] = EMPTY_CHAR;
		buffer_terminal[curr_window][(y + scroll_y) * FULL_WIDTH + x] = EMPTY_CHAR;
		return ;
	}

	move_cursor(-1, 0);
	x --;

	visible_terminal[y * VGA_WIDTH + x] = EMPTY_CHAR;
	buffer_terminal[curr_window][(y + scroll_y) * FULL_WIDTH + x] = EMPTY_CHAR;
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
	curr_window = 0;

	clear_screen();
	set_cursor_pos(0, 0);

	printk_color(RED, forty_two1);
	printk("\n\nthis is window 1");
	switch_window(1);
	printk_color(GREEN, forty_two2);
	printk("\n\nthis is window 2");
	switch_window(1);
	printk_color(CYAN, forty_two3);
	printk("\n\nthis is window 3");
	switch_window(1);
	printk_color(MAGENTA, forty_two4);
	printk("\n\nthis is window 4");
	switch_window(1);
	printk_color(BROWN, forty_two5);
	printk("\n\nthis is window 5");
	switch_window(1);
}
