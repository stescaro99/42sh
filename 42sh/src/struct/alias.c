#include "sh_header.h"

t_alias	*alias_lstnew(char *name, char *value)
{
	t_alias	*new;

	new = malloc(sizeof(t_alias));
	if (!new)
		return (NULL);
	new->name = ft_strdup(name);
	new->value = ft_strdup(value);
	new->next = NULL;
	return (new);
}

t_alias	*alias_lstadd_back(t_alias **lst, t_alias *new)
{
	t_alias	*temp;

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

void	alias_lstclear(t_alias *lst)
{
	t_alias	*temp;

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