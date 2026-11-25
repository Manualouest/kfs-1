/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kmem.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/21 00:05:00 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/25 21:03:24 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>

void	*memcpy(void *dest, const void *src, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
	return (dest);
}

void	*memset(void *s, int c, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		((char *)s)[i] = c;
	return (s);
}

void	bzero(void *s, size_t n)
{
	s = memset(s, 0, n);
}

void	swap(void *a, void *b)
{
	uint32_t	tmp = *(uint32_t *)a;
	*(uint32_t *)a = *(uint32_t *)b;
	*(uint32_t *)b = tmp;
}
