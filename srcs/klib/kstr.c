/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kstr.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/24 18:35:03 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/24 18:39:57 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>

size_t	strlen(const char *str)
{
	size_t	i = 0;
	while (str[i])
		i ++;
	return (i);
}

int	strcmp(const char *str1, const char *str2)
{
	size_t	i = 0;
	while (str1[i] && str1[i] == str2[i])
		i ++;
	return (str1[i] - str2[i]);
}
