/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   printk.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/20 18:22:40 by eblondee          #+#    #+#             */
/*   Updated: 2026/11/26 10:10:22 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "kernel.h"

static void print_unsigned(uint32_t n, uint32_t base, int uppercase, uint8_t color)
{
	const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
	char       buf[32];
	size_t     i = 0;

	if (n == 0)
	{
		putchar_color('0', color);
		return;
	}
	while (n > 0)
	{
		buf[i++] = digits[n % base];
		n /= base;
	}
	while (i > 0)
		putchar_color((unsigned char)buf[--i], color);
}

static void print_number(int n, uint8_t color)
{
	uint32_t u;

	if (n < 0)
	{
		putchar_color('-', color);
		u = (uint32_t)(-(n + 1)) + 1;
	}
	else
		u = (uint32_t)n;
	print_unsigned(u, 10, 0, color);
}

static void print_str(const char *str, uint8_t color)
{
	if (!str)
		str = "(null)";
	write_color(str, color);
}

void vprintk(uint8_t color, const char *fmt, va_list args)
{
	for (size_t i = 0; fmt[i]; ++i)
	{
		if (fmt[i] != '%')
		{
			putchar_color((unsigned char)fmt[i], color);
			continue;
		}
		i++;
		if (!fmt[i])
			break;
		switch (fmt[i])
		{
			case '%':
				putchar_color('%', color);
				break;
			case 'c':
				putchar_color((unsigned char)va_arg(args, int), color);
				break;
			case 's':
				print_str(va_arg(args, const char *), color);
				break;
			case 'd':
			case 'i':
				print_number(va_arg(args, long int), color);
				break;
			case 'u':
				print_unsigned(va_arg(args, unsigned int), 10, 0, color);
				break;
			case 'x':
				print_unsigned(va_arg(args, unsigned int), 16, 0, color);
				break;
			case 'X':
				print_unsigned(va_arg(args, unsigned int), 16, 1, color);
				break;
			case 'p':
				write_color("0x", color);
				print_unsigned((uint32_t)(uintptr_t)va_arg(args, void *), 16, 0, color);
				break;
			default:
				putchar_color('%', color);
				putchar_color((unsigned char)fmt[i], color);
				break;
		}
	}
}

void printk(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vprintk(DEFAULT_COLOR, fmt, args);
	va_end(args);
}

void printk_color(uint8_t color, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vprintk(color, fmt, args);
	va_end(args);
}
