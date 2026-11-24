/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mouse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 18:48:51 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/24 18:20:30 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>

int				highlight_status;
static float	mouse_x;
static float	mouse_y;

static int	set_sample_rate(uint8_t rate)
{
	if (ACK_safe_write_port_second_device(0x60, 0xF3) == -1)
		return (0);
	if (ACK_safe_write_port_second_device(0x60, rate) == -1)
		return (0);
	return (1);
}

static int	set_resolution(uint8_t resolution)
{
	if (ACK_safe_write_port_second_device(0x60, 0xE8) == -1)
		return (0);
	if (ACK_safe_write_port_second_device(0x60, resolution) == -1)
		return (0);
	return (1);
}

static int get_mouseID(void)
{
	if (ACK_safe_write_port_second_device(0x60, 0xF2) != -1)
	{
		int	mouseID = safe_read_port(0x60);
		return (mouseID);
	}
	return (-1);
}

int	activate_mouse(void)
{
	highlight_status = 0;
	mouse_x = 0;
	mouse_y = 0;

	// activate mouse
	if (get_mouseID() == -1)
		return (0);

	// this is the magic sequence to enable the scroll wheel
	if (!set_sample_rate(0xC8)) //set sample rate to 200
		return (0);
	if (!set_sample_rate(0x64)) // 100
		return (0);
	if (!set_sample_rate(0x50)) // 80
		return (0);
	if (get_mouseID() == -1)
		return (0);

	// slow down mouse
	if (!set_sample_rate(0x16)) // 80
		return (0);

	// enable automatic mouvement packet generation
	if (ACK_safe_write_port_second_device(0x60, 0xF4) == -1)
		return (0);

	// set scaling to 1:1
	if (ACK_safe_write_port_second_device(0x60, 0xE6) == -1)
		return (0);

	if (!set_resolution(0x00))
		return (0);
	return (1);
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
	// 	highlight_status = !highlight_status;

	// 	uint16_t	raw_pos = get_cursor_pos();
	// 	int			pos_x = (int)(raw_pos % VGA_WIDTH);
	// 	int			pos_y = (int)(raw_pos / VGA_WIDTH);

	// 	mouse_x = pos_x;
	// 	mouse_y = pos_y;
	// }

	// if (highlight_status)
	// 	move_mouse(bytes);

	if (bytes[3] == 1 && get_state_key_status(SCRL) == OFF)
		scroll_down();
	else if (bytes[3] & 0xF && get_state_key_status(SCRL) == OFF)
		scroll_up();
}

void	update_mouse_pos(void)
{
	uint16_t	raw_pos = get_cursor_pos();
	int			pos_x = (int)(raw_pos % VGA_WIDTH);
	int			pos_y = (int)(raw_pos / VGA_WIDTH);

	mouse_x = pos_x;
	mouse_y = pos_y;
}

int	poll_mouse(void)
{
	static	int		nb_recv = 0;
	static	uint8_t	bytes[4] = {0};

	int	packet = safe_read_port(0x60);

	if (packet == -1)
		return (0);

	bytes[nb_recv] = (uint8_t)packet;
	if (nb_recv == 3)
		process_inputs(bytes);
	nb_recv ++;
	if (nb_recv == 4)
		nb_recv = 0;
	return (1);
}
