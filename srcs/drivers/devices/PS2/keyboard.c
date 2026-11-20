/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   keyboard.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 23:24:49 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/20 17:21:50 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>

uint8_t	keys[11] = {0};

// keys is a bitfield -> we need 58 different bits (scan code set 1)
// key / 8 gives the bitfield group of the key and (key >> 8) gives the bit for the key itself

inline int	is_key_pressed(uint8_t key)
{
	return ((keys[key / 8] >> (key % 8)) & 1);
}

inline void	set_key_value(uint8_t key, uint8_t value)
{
	keys[key / 8] = (keys[key / 8] & (~(1 << (key % 8)))) | (value << (key % 8));
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

