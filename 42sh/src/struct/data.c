#include "42sh.h"

short	init_data(t_data *data, char **env)
{
	data->env = set_envlist(env);
	if (!data->env)
		return (1);
	data->memory = NULL;
	data->alias = NULL;
	data->line = NULL;
	data->fds = malloc(sizeof(int) * 3);
	if (!data->fds)
	{
		env_lstclear(data->env);
		return (1);
	}
	if (data->fds)
	{
		data->fds[0] = dup(0);
		data->fds[1] = dup(1);
		data->fds[2] = dup(2);
	}
	return (0);
}

void	clear_all(t_data *data)
{
	if (data->env)
		env_lstclear(data->env);
	if (data->memory)
		memory_lstclear(data->memory);
	if (data->alias)
		alias_lstclear(data->alias);
	if (data->line)
		line_lstclear(&data->line);
	if (data->fds)
	{
		close(data->fds[0]);
		close(data->fds[1]);
		close(data->fds[2]);
		free(data->fds);
	}
}