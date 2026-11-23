/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cast.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 18:20:21 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/22 18:20:43 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>

int32_t	float_int_cast(float f)
{
	union {
		float		f;
		uint32_t	i;
	}	bitfield = {.f = f};

	uint32_t	sign		= bitfield.i >> 31;
	int32_t		exp			= ((bitfield.i >> 23) & 0xFF) - 127;
	uint32_t	mantissa	= (bitfield.i & 0x7FFFFF) | 0x800000;

	if (exp < 0)
		return (0);
	if (exp >= 31)
		return (sign ? INT32_MIN : INT32_MAX);
	
	int32_t	result;
	if (exp >= 23)
		result = (int32_t)(mantissa << (exp - 23));
	else
		result = (int32_t)(mantissa >> (23 - exp));
	
	return (sign ? -result : result);
}
