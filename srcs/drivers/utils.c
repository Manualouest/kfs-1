/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 18:04:51 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/20 21:45:06 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>

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

volatile void	*memcpy(volatile void *dest, const volatile void *src, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
	return (dest);
}

void	*memset(void *s, int c, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		((char *)s)[i] = c;
	return (s);
}

void	bzero(void *s, size_t n)
{
	s = memset(s, 0, n);
}
