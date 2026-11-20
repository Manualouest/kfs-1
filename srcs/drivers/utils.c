/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 18:04:51 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/21 00:04:24 by mbirou           ###   ########.fr       */
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
