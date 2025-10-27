/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sfabi <sfabi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/18 11:23:38 by stescaro          #+#    #+#             */
/*   Updated: 2024/05/09 10:58:35 by sfabi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h>
#include <sys/mman.h>
#include "libft.h"

typedef struct s_env
{
	char			**envp;
	char			*name;
	char			*value;
	bool			unsetted;
	struct s_env	*next;
}	t_env;

typedef struct s_memory
{
	char			*name;
	char			*value;
	struct s_memory	*next;
}	t_memory;

typedef struct s_alias
{
    char			*name;
    char			*value;
    struct s_alias	*next;
}	t_alias;

typedef struct s_line
{
	struct s_line	*parenthesis;
	short			type;
	char			*line;
	short			logic;
	short			exit;
	struct s_line	*next;
}	t_line;

typedef struct s_data
{
	struct s_env	*env;
	struct s_line	*line;
	struct s_memory	*memory;
	struct s_alias	*alias;
	int				*fds;
}	t_data;

int	change_envopwd(t_env **env, char *opwd);
int	change_envpwd(t_env **env);
t_env	*env_lstnew(char *env, char **envp);
t_env	*env_lstlast(t_env *lst);
bool	env_lstclear(t_env *lst);



#endif
