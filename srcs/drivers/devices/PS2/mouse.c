/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mouse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 18:48:51 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/23 18:04:36 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>

int				use_mouse;
static float	mouse_x;
static float	mouse_y;

static void	set_sample_rate(uint8_t rate)
{
	ACK_safe_write_port_second_device(0x60, 0xF3);
	ACK_safe_write_port_second_device(0x60, rate);
}

static void	set_resolution(uint8_t resolution)
{
	ACK_safe_write_port_second_device(0x60, 0xE8);
	ACK_safe_write_port_second_device(0x60, resolution);
}

static uint8_t get_mouseID()
{
	ACK_safe_write_port_second_device(0x60, 0xF2);
	uint8_t	mouseID = safe_read_port(0x60);
	return (mouseID);
}

void	activate_mouse(void)
{
	use_mouse = 0;
	mouse_x = 0;
	mouse_y = 0;

	// activate mouse
	get_mouseID();

	// this is the magic sequence to enable the scroll wheel
	set_sample_rate(0xC8); //set sample rate to 200
	set_sample_rate(0x64); // 100
	set_sample_rate(0x50); // 80
	get_mouseID();

	// slow down mouse
	set_sample_rate(0x16); // 80

	// enable automatic mouvement packet generation
	ACK_safe_write_port_second_device(0x60, 0xF4);

	// set scaling to 1:1
	ACK_safe_write_port_second_device(0x60, 0xE6);

	set_resolution(0x00);
}
// TODO --> all this is reused to do copy-paste
// static void	verify_mouse_pos()
// {
// 	if (mouse_x < 0)
// 		mouse_x = 0;
// 	if (mouse_x >= VGA_WIDTH)
// 		mouse_x = VGA_WIDTH - 1;
// 	if (mouse_y < 0)
// 		mouse_y = 0;
// 	if (mouse_y >= VGA_HEIGHT)
// 		mouse_y = VGA_HEIGHT - 1;
// }

// TODO --> all this is reused to do copy-paste
// static void	move_mouse(uint8_t bytes[4])
// {
// 	mouse_x += (int)(bytes[0] & (1 << 4) ? (bytes[1] ^ 0xFFFFFF00) : bytes[1]) / 40.f;
// 	mouse_y -= (int)(bytes[0] & (1 << 5) ? (bytes[2] ^ 0xFFFFFF00) : bytes[2]) / 40.f;

// 	verify_mouse_pos();
// 	set_cursor_pos(int(mouse_x), int(mouse_y));
// }

static void	process_inputs(uint8_t bytes[4])
{
	// TODO --> all this is reused to do copy-paste
	// if (bytes[0] & (1 << 0) && !get_state_key_status(SCRL))
	// {
	// 	use_mouse = !use_mouse;

	// 	uint16_t	raw_pos = get_cursor_pos();
	// 	int			pos_x = (int)(raw_pos % VGA_WIDTH);
	// 	int			pos_y = (int)(raw_pos / VGA_WIDTH);

	// 	mouse_x = pos_x;
	// 	mouse_y = pos_y;
	// }

	// if (use_mouse)
	// 	move_mouse(bytes);

	if (bytes[3] == 1)
		scroll_down();
	else if (bytes[3] & 0xF)
		scroll_up();
}

void	update_mouse_pos()
{
	uint16_t	raw_pos = get_cursor_pos();
	int			pos_x = (int)(raw_pos % VGA_WIDTH);
	int			pos_y = (int)(raw_pos / VGA_WIDTH);

	mouse_x = pos_x;
	mouse_y = pos_y;
}

void	poll_mouse()
{
	static	int		nb_recv = 0;
	static	uint8_t	bytes[4] = {0};

	uint8_t	packet = safe_read_port(0x60);

	bytes[nb_recv] = packet;
	if (nb_recv == 3)
		process_inputs(bytes);
	nb_recv ++;
	if (nb_recv == 4)
		nb_recv = 0;
}
