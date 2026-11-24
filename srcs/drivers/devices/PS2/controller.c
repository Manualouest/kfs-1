/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   controller.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 17:57:28 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/24 18:17:12 by mbirou           ###   ########.fr       */
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

static inline uint8_t	did_controller_return(void)
{
	uint8_t	status = read_port(0x64);
	if (status & 1)
		return (status);
	return (0);
}

// waits for the command register to send something in the Data Port
static int	wait_controller_return(void)
{
	volatile uint32_t	timeout_iter = 10000;
	while (!did_controller_return())
		if (--timeout_iter <= 0)
			return (0);
	return (1);
}

static inline uint8_t	is_controller_ready(void)
{
	return ((read_port(0x64) & (1 << 1)) >> 1);
}

// waits for the command register to be ready to receive input
static int	wait_controller_ready(void)
{
	volatile uint32_t	timeout_iter = 10000;
	while (is_controller_ready())
		if (--timeout_iter <= 0)
			return (0);
	return (1);
}

int	safe_write_port(uint16_t port, uint8_t data)
{
	if (wait_controller_ready())
		write_port(port, data);
	else
		return (-1);
	return (1);
}

int	ACK_safe_write_port(uint16_t port, uint8_t data)
{
	if (wait_controller_ready())
	{
		write_port(port, data);
		uint8_t	response = safe_read_port(0x60);
		return (response);
	}
	return (-1);
}

int	ACK_safe_write_port_second_device(uint16_t port, uint8_t data)
{
	if (wait_controller_ready())
	{
		if (safe_write_port(0x64, 0xD4) == -1)
			return (-1);
		if (safe_write_port(port, data) == -1)
			return (-1);
		uint8_t	response = safe_read_port(0x60);
		return (response);
	}
	return (-1);
}

int	safe_read_port(uint16_t port)
{
	if (wait_controller_return())
	{
		uint8_t	data = read_port(port);
		return (data);
	}
	return (-1);
}

static int	get_config_byte(void)
{
	if (safe_write_port(0x64, 0x20) == -1)
		return (-1);
	uint8_t	controller_config_byte = safe_read_port(0x60);
	return (controller_config_byte);
}

static int	write_data_to_ram(uint8_t data)
{
	if (safe_write_port(0x64, 0x60) == -1)
		return (-1);
	if (safe_write_port(0x60, data) == -1)
		return (-1);
	return (1);
}

void	flush(void)
{
	while (did_controller_return())
		read_port(0x60);
}

int	init_devices(void)
{
	// disabeling devices
	if (safe_write_port(0x64, 0xAD) == -1)
		return (0);
	if (safe_write_port(0x64, 0xA7) == -1)
		return (0);

	// flushing the output buffer
	flush();

	// setup controller config
	int	config_byte = get_config_byte();
	if (config_byte == -1)
		return (0);
	config_byte &= ~1 & ~(1 << 1) & ~(1 << 4) & ~(1 << 6);
	if (write_data_to_ram(config_byte) == -1)
		return (0);

	// controller self test
	if (safe_write_port(0x64, 0xAA) == -1)
		return (0);
	if (safe_read_port(0x60) != 0x55)
		return (0);

	if (write_data_to_ram(config_byte) == -1)
		return (0);

	// check if there are two channels
	if (safe_write_port(0x64, 0xA8) == -1)
		return (0);
	config_byte = get_config_byte();
	if (config_byte == -1)
		return (0);
	if (!(config_byte & (1 << 5)))
	{
		if (safe_write_port(0x64, 0xA7) == -1)
			return (0);
		config_byte &= ~(1 << 1) & ~(1 << 4) & ~(1 << 5);
		if (write_data_to_ram(config_byte) == -1)
			return (0);
		second_device = 1;
	}

	// perform interface tests
	if (safe_write_port(0x64, 0xAB) == -1)
		return (0);
	if (safe_read_port(0x60) != 0)
		return (0);
	if (second_device)
	{
		if (safe_write_port(0x64, 0xA9) == -1)
			return (0);
		if (safe_read_port(0x60) != 0)
			return (0);
	}

	// enable the devices
	if (safe_write_port(0x64, 0xAE) == -1)
		return (0);
	if (second_device)
		if (safe_write_port(0x64, 0xA8) == -1)
			return (0);

	// reset devices
	if (safe_write_port(0x60, 0xFF) == -1)
		return (0);
	int	ret = safe_read_port(0x60);
	if (ret == 0xFC || ret == -1)
		return (0);


	if (!setup_keyboard())
		return (0);
	if (second_device)
		if (!activate_mouse())
			return (0);
	return (1);
}

int	poll(void)
{
	uint8_t	controller_status = did_controller_return();
	while (controller_status)
	{
		if (controller_status & (1 << 6) || controller_status & (1 << 7))
			return (0);

		if ((controller_status & (1 << 5)) && second_device)
		{
			if (!poll_mouse())
				return (-1);
		}
		else if (!(controller_status & (1 << 5)))
		{
			if (!poll_keyboard())
				return (-1);
		}

		controller_status = did_controller_return();
	}
	return (1);
}
