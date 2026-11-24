/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kernel.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 18:26:55 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/24 18:12:53 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>

void	kmain(void)
{
	init_terminal();

	if (!init_devices())
	{
		write_color("Devices failed to initialise.", RED);
		return ;
	}
	int	status = poll();
	while (status)
		status = poll();
	if (status == 0)
		write_color("Devices error 6 | 7 happened.", RED);
	else if (status == -1)
		write_color("Timeout happened on a device.", RED);
}
