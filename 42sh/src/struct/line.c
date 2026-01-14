#include "sh_header.h"

t_line	*set_line(t_memory *memory, t_env *env, t_alias *alias)
{
	char		*ln;
	const char	*prompt;
	t_line		*line;

	prompt = build_prompt(env);
	ln = readline(prompt);
	free((char *)prompt);
	if (!ln)
		ln = ft_strdup("exit\n");
	line = line_lstnew(ln, memory, env, alias);
	add_history(ln);
	free(ln);
	return (line);
}

t_line	*line_lstnew(char *line_str, t_memory *memory, t_env *env, t_alias *alias)
{
	// TODO : implement parsing logic to create a t_line from line_str
	return (NULL); // Placeholder
}

t_line	*line_lstadd_back(t_line **lst, t_line *new)
{
	t_line	*temp;

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

void	line_lstclear(t_line **lst)
{
	t_line	*temp;

	if (!lst)
		return;
	while (*lst)
	{
		if ((*lst)->parenthesis)
			line_lstclear(&(*lst)->parenthesis);
		if ((*lst)->line)
			free((*lst)->line);
		if ((*lst)->command_path)
			free((*lst)->command_path);
		temp = (*lst)->next;
		free(*lst);
		*lst = temp;
	}
}

