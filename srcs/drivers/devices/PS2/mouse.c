/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mouse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 18:48:51 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/20 23:03:52 by mbirou           ###   ########.fr       */
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

void	poll_mouse()
{
	static	int	nb_recv = 0;
	size_t	old_x = get_x();
	size_t	old_y = get_y();
	if (nb_recv == 0)
	{
		clear_zone(70, 0, 10, 25);
		set_pos(70, 10);
		// write("mouse");	newline();
	}
	set_pos(70, 11 + nb_recv);
	uint8_t	packet = safe_read_port(0x60);
	// debug_byte(packet);

	if (nb_recv == 3)
	{
		if (packet == 1)
			scroll_down();
		else if (packet & 0xF)
			scroll_up();
	}
	nb_recv ++;
	if (nb_recv == 4)
		nb_recv = 0;

	set_pos(old_x, old_y);
}