/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   controller.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 17:57:28 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/20 22:56:12 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>

int	second_device = 0;

void	debug_byte(uint8_t byte) // DEBUG
{
	for (int i = 0; i < 8; ++i)
		putchar(((byte >> (7 - i)) & 1) + '0');
}

// port 0x64 is to read status or send commands
// port 0x60 is to read data received/return of command or send second part of command

static inline uint8_t	did_controller_return()
{
	uint8_t	status = read_port(0x64);
	if (status & 1)
		return (status);
	return (0);
}

// waits for the command register to send something in the Data Port
static void	wait_controller_return()
{
	while (!did_controller_return())
		;
}

static inline uint8_t	is_controller_ready()
{
	return ((read_port(0x64) & (1 << 1)) >> 1);
}

// waits for the command register to be ready to receive input
static void	wait_controller_ready()
{
	while (is_controller_ready())
		;
}

void	safe_write_port(uint16_t port, uint8_t data)
{
	wait_controller_ready();
	write_port(port, data);
}

uint8_t	ACK_safe_write_port(uint16_t port, uint8_t data)
{
	wait_controller_ready();
	write_port(port, data);
	uint8_t	response = safe_read_port(0x60);
	return (response);
}

void	safe_write_port_second_device(uint16_t port, uint8_t data)
{
	wait_controller_ready();
	safe_write_port(0x64, 0xD4);
	safe_write_port(port, data);
}

uint8_t	ACK_safe_write_port_second_device(uint16_t port, uint8_t data)
{
	wait_controller_ready();
	safe_write_port(0x64, 0xD4);
	safe_write_port(port, data);
	uint8_t	response = safe_read_port(0x60);
	return (response);
}

uint8_t	safe_read_port(uint16_t port)
{
	wait_controller_return();
	uint8_t	data = read_port(port);
	return (data);
}

static uint8_t	get_config_byte()
{
	safe_write_port(0x64, 0x20);
	uint8_t	controller_config_byte = safe_read_port(0x60);
	return (controller_config_byte);
}

static void	write_data_to_ram(uint8_t data)
{
	safe_write_port(0x64, 0x60);
	safe_write_port(0x60, data);
}

static void	flush()
{
	read_port(0x60);
}

void	init_devices(void)
{
	// disabeling devices
	safe_write_port(0x64, 0xAD);
	safe_write_port(0x64, 0xA7);

	// flushing the output buffer
	flush();

	// setup controller config
	uint8_t	config_byte = get_config_byte();
	config_byte &= ~1 & ~(1 << 1) & ~(1 << 4) & ~(1 << 6);
	write_data_to_ram(config_byte);

	// controller self test
	safe_write_port(0x64, 0xAA);
	if (safe_read_port(0x60) != 0x55)
		return ;
	write_data_to_ram(config_byte);

	// check if there are two channels
	safe_write_port(0x64, 0xA8);
	config_byte = get_config_byte();
	if (!(config_byte & (1 << 5)))
	{
		safe_write_port(0x64, 0xA7);
		config_byte &= ~(1 << 1) & ~(1 << 4) & ~(1 << 5);
		write_data_to_ram(config_byte);
		second_device = 1;
	}

	// perform interface tests
	safe_write_port(0x64, 0xAB);
	if (safe_read_port(0x60) != 0)
		return ;
	if (second_device)
	{
		safe_write_port(0x64, 0xA9);
		if (safe_read_port(0x60) != 0)
			return ;
	}

	// enable the devices
	safe_write_port(0x64, 0xAE);
	if (second_device)
		safe_write_port(0x64, 0xA8);

	config_byte = get_config_byte();
	config_byte |= 1 | (1 << 1);
	write_data_to_ram(config_byte);


	// reset devices
	safe_write_port(0x60, 0xFF);
	if (safe_read_port(0x60) == 0xFC)
		return ;


	setup_keyboard();
	activate_mouse();
	
}

void	poll()
{
	uint8_t	controller_status = did_controller_return();
	while (controller_status)
	{
		if (controller_status & (1 << 5))
			poll_mouse();
		else
			poll_keyboard();

		controller_status = did_controller_return();
	}
}