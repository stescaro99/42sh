#include "sh_header.h"

t_memory	*memory_lstnew(char *name, char *value)
{
	t_memory	*new;

	new = malloc(sizeof(t_memory));
	if (!new)
		return (NULL);
	new->name = ft_strdup(name);
	new->value = ft_strdup(value);
	new->next = NULL;
	return (new);
}

t_memory	*memory_lstadd_back(t_memory **lst, t_memory *new)
{
	t_memory	*temp;

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

void	memory_lstclear(t_memory *lst)
{
	t_memory	*temp;

	while (lst)
	{
		if (lst->name)
			free(lst->name);
		if (lst->value)
			free(lst->value);
		temp = lst->next;
		free(lst);
		lst = temp;
	}
}