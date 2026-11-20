/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   keyboard.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 23:24:49 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/20 23:17:53 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>

static uint8_t	keyboard_state = 0;

typedef enum	STATES
{
	OFF,
	ON
}	STATES;

typedef enum	STATE_KEY
{
	SHIFT,
	CAPS,
	NUM,
	CTRL,
	ALT
}	STATE_KEY;

static inline STATES	get_state_key_status(STATE_KEY key)
{
	return ((keyboard_state & (1 << key)) >> key);
}


void	setup_keyboard()
{
	// set scan code to 1 -> this changes the code made by keys
	safe_write_port(0x60, 0xF0);
	ACK_safe_write_port(0x60, 1);
}

// ### TODO
// key register -> call terminal to print key
// same for mouse inputs
//
// --> try to do a sort of MVC design patern



void	poll_keyboard()
{
	size_t	old_x = get_x();
	size_t	old_y = get_y();

	clear_zone(60, 0, 10, 25);
	set_pos(60, 10);
	// write("keyboard");	newline();
	set_pos(60, 11);
	uint8_t	scancode = safe_read_port(0x60);
	// debug_byte(scancode);	newline();
	set_pos(old_x, old_y);

	debug_byte(scancode);	newline();
}