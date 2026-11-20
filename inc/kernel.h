/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kernel.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 17:55:43 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/20 23:22:23 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef KERNEL_H
# define KERNEL_H

# include <stdint.h>
# include <stddef.h>

// ##################################################
// #			lib/printk.c						#
// ##################################################

# include <stdarg.h>

void printk(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void printk_color(uint8_t color, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void vprintk(uint8_t color, const char *fmt, va_list args);

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

#define DEFAULT_COLOR   0b00001111			//(BLACK << 4 | WHITE)

void	scroll_up();
void	scroll_down();
void	putchar_color(unsigned char c, uint8_t color);
void	putchar(unsigned char c);
void	write_color(const char *str, uint8_t color);
void	write(const char *str);
void	newline(void);
void	set_pos(size_t nx, size_t ny);
void	clear_line(size_t y);
void	clear_zone(size_t target_x, size_t target_y, size_t size_x, size_t size_y);
void	clear_screen();
size_t	get_x();
size_t	get_y();
void	init_terminal(void);


// ##################################################
// #			drivers/utils.c						#
// ##################################################
uint8_t			read_port(uint16_t port);
void			write_port(uint16_t port, uint8_t data);
void			*memset(void *s, int c, size_t n);
void			bzero(void *s, size_t n);
volatile void	*memcpy(volatile void *dest, const volatile void *src, size_t n);


// ##################################################
// #			drivers/devices/PS2/keyboard.c		#
// ##################################################
void	setup_keyboard();
void	poll_keyboard();


// ##################################################
// #			drivers/devices/PS2/mouse.c			#
// ##################################################
void	activate_mouse(void);
void	poll_mouse();


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
void	poll();



#endif
