/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kernel.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 17:55:43 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/23 20:13:13 by eblondee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef KERNEL_H
# define KERNEL_H

# include <stdint.h>
# include <stddef.h>

// ##################################################
// #			klib/printk.c						#
// ##################################################

# include <stdarg.h>

void printk(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void printk_color(uint8_t color, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void vprintk(uint8_t color, const char *fmt, va_list args);


// ##################################################
// #			klib/kmem.c							#
// ##################################################

void			*memset(void *s, int c, size_t n);
void			bzero(void *s, size_t n);
volatile void	*memcpy(volatile void *dest, const volatile void *src, size_t n);


// ##################################################
// #			klib/cast.c							#
// ##################################################
int32_t	float_int_cast(float f);
#define int(f) float_int_cast(f)


// ##################################################
// #			drivers/terminal.c					#
// ##################################################
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
#define DEFAULT_COLOR   0b00001111			//(BLACK << 4 | WHITE)

uint16_t	invert_color(uint16_t c);
void		toggle_cursor(void);
void		set_cursor_pos(int x, int y);
uint16_t	get_cursor_pos(void);
void		move_cursor(int move_x, int move_y);
void		scroll_up(void);
void		scroll_down(void);
void		putchar_color(unsigned char c, uint8_t color);
void		putchar(unsigned char c);
void		write_color(const char *str, uint8_t color);
void		write(const char *str);
void		newline(void);
void		tabulation(void);
void		set_pos(size_t nx, size_t ny);
void		clear_line(size_t y);
void		clear_zone(size_t target_x, size_t target_y, size_t size_x, size_t size_y);
void		clear_screen(void);
void		erase_char(void);
uint16_t	get_curr_char(void);
size_t		get_x(void);
size_t		get_y(void);
void		init_terminal(void);


// ##################################################
// #			drivers/utils.c						#
// ##################################################
uint8_t			read_port(uint16_t port);
void			write_port(uint16_t port, uint8_t data);


// ##################################################
// #			drivers/devices/PS2/keyboard.c		#
// ##################################################
typedef enum	STATE
{
	OFF,
	ON
}	STATE;

typedef enum	STATE_KEY
{
	SHIFT,
	CAPS,
	NUM,
	CTRL,
	ALT,
	SCRL
}	STATE_KEY;

void	setup_keyboard(void);
void	poll_keyboard(void);
//STATE	get_state_key_status(STATE_KEY key); // TODO : DELETE ?
//void	set_state_key_status(STATE_KEY key, STATE status); // TODO : DELETE ?

// ##################################################
// #			drivers/devices/PS2/mouse.c			#
// ##################################################
void	update_mouse_pos(void);
void	activate_mouse(void);
void	poll_mouse(void);


// ##################################################
// #			drivers/devices/PS2/controller.c	#
// ##################################################
void	debug_byte(uint8_t data);
void	safe_write_port(uint16_t port, uint8_t data);
uint8_t	ACK_safe_write_port(uint16_t port, uint8_t data);
void	safe_write_port_second_device(uint16_t port, uint8_t data);
uint8_t	ACK_safe_write_port_second_device(uint16_t port, uint8_t data);
uint8_t	safe_read_port(uint16_t port);
void	init_devices(void);
void	poll(void);



#endif
