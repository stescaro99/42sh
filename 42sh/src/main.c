/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: stescaro <stescaro@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/18 11:20:15 by stescaro          #+#    #+#             */
/*   Updated: 2024/05/09 15:21:01 by stescaro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "sh_header.h"

int	g_ctrl = 0;

int	main(int argc, char **argv, char **env)
{
	t_data data;
	short ret;

	(void)argv;
	if(init_data(&data, env))
		return (1);
	header();
	ret = 0 * argc;
	while (ret >= 0)
	{
		signal(SIGINT, signal_handler);
		signal(SIGQUIT, SIG_IGN);
		data.line = set_line(data.memory, data.env, data.alias);
		if (!data.line)
		{
			clear_all(&data);
			clear_history();
			return (1);
		}
		ret = shell(&data);
		line_lstclear(&data.line);
	}
	clear_all(&data);
	clear_history();
	return (0);
}
