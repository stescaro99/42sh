#include "sh_header.h"

t_env	*set_envlist(char **env)
{
	t_env	*envlist;
	t_env	*temp;
	int		i;

	i = 0;
	envlist = NULL;
	while (env[i])
	{
		temp = env_lstnew(env[i], env);
		if (!temp)
			return (env_lstclear(envlist), NULL);
		env_lstadd_back(&envlist, temp);
		i++;
	}
	return (envlist);
}

t_env	*env_lstnew(char *line, char **envp)
{
	t_env	*new;
	char	*equal_pos;

	new = malloc(sizeof(t_env));
	if (!new)
		return (NULL);
	equal_pos = ft_strchr(line, '=');
	if (equal_pos)
	{
		new->name = ft_substr(line, 0, equal_pos - line);
		new->value = ft_strdup(equal_pos + 1);
	}
	else
	{
		new->name = ft_strdup(line);
		new->value = NULL;
	}
	new->unsetted = false;
	new->envp = envp;
	new->next = NULL;
	return (new);
}

t_env	*env_lstadd_back(t_env **lst, t_env *new)
{
	t_env	*temp;

	if (!lst || !new)
		return (NULL);
	if (!*lst)
	{
		*lst = new;
		return (*lst);
	}
	temp = *lst;
	while (temp->next)
		temp = temp->next;
	temp->next = new;
	return (*lst);
}

void	env_lstclear(t_env *lst)
{
	t_env	*temp;

	while (lst)
	{
		temp = lst->next;
		if (lst->name)
			free(lst->name);
		if (lst->value)
			free(lst->value);
		free(lst);
		lst = temp;
	}
}