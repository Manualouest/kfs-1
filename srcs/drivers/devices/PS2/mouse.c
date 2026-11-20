/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mouse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 18:48:51 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/20 17:21:50 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>

static void	set_sample_rate(uint8_t rate)
{
	ACK_safe_write_port_second_device(0x60, 0xF3);
	ACK_safe_write_port_second_device(0x60, rate);
}

static uint8_t get_mouseID()
{
	ACK_safe_write_port_second_device(0x60, 0xF2);
	uint8_t	mouseID = safe_read_port(0x60);
	return (mouseID);
}

void	activate_mouse(void)
{
	// activate mouse
	get_mouseID();

	// this is the magic sequence to enable the scroll wheel
	set_sample_rate(0xC8); //set sample rate to 200
	set_sample_rate(0x64); // 100
	set_sample_rate(0x50); // 80
	get_mouseID();

	// slow down mouse
	set_sample_rate(0x50); // 80

	// enable automatic mouvement packet generation
	ACK_safe_write_port_second_device(0x60, 0xF4);
}
