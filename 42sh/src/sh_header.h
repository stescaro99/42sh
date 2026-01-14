#ifndef SH_HEADER_H
# define SH_HEADER_H

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
	char			*command_path;
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

// ui
void		header(void);
char		*build_prompt(t_env *env);

// struct data
void		init_data(t_data *data, char **env);
void		clear_all(t_data *data);

// struct env
t_env		*set_envlist(char **env);
t_env		*env_lstnew(char *line, char **envp);
t_env		*env_lstadd_back(t_env **lst, t_env *new);
void		env_lstclear(t_env *lst);

// struct memory
t_memory	*memory_lstnew(char *name, char *value);
t_memory	*memory_lstadd_back(t_memory **lst, t_memory *new);
void		memory_lstclear(t_memory *lst);

// struct alias
t_alias		*alias_lstnew(char *name, char *value);
t_alias		*alias_lstadd_back(t_alias **lst, t_alias *new);
void		alias_lstclear(t_alias *lst);

// struct line
t_line		*set_line(t_memory *memory, t_env *env, t_alias *alias);
t_line		*line_lstnew(char *line_str, t_memory *memory, t_env *env, t_alias *alias);
t_line		*line_lstadd_back(t_line **lst, t_line *new);
void		line_lstclear(t_line **lst);



#endif
