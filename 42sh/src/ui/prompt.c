#include "sh_header.h"

void	header(void)
{
	printf("\033[H\033[J");
	printf("\033[36m/---------------------------------------------------------\\\n\n");
	printf(" /$$   /$$  /$$$$$$   /$$$$$$  /$$                 /$$ /$$\n");
	printf("| $$  | $$ /$$__  $$ /$$__  $$| $$                | $$| $$\n");
	printf("| $$  | $$|__/  \\ $$| $$  \\__/| $$$$$$$   /$$$$$$ | $$| $$\n");
	printf("| $$$$$$$$  /$$$$$$/|  $$$$$$ | $$__  $$ /$$__  $$| $$| $$\n");
	printf("|_____  $$ /$$____/  \\____  $$| $$  \\ $$| $$$$$$$$| $$| $$\n");
	printf("      | $$| $$       /$$  \\ $$| $$  | $$| $$_____/| $$| $$\n");
	printf("      | $$| $$$$$$$$|  $$$$$$/| $$  | $$|  $$$$$$$| $$| $$\n");
	printf("      |__/|________/ \\______/ |__/  |__/ \\_______/|__/|__/\n");
	printf("\n\\---------------------------------------------------------/\n\033[0m");
}

char	*build_prompt(t_env *env)
{
	char *user = get_env_value(env, "USER");
	char *cwd = get_env_value(env, "PWD");
	char *prompt;

	if (!user)
		user = "guest";
	if (!cwd)
		cwd = "~";
	prompt = malloc(strlen(user) + strlen(cwd) + 5);
	if (!prompt)
		return NULL;
	sprintf(prompt, "\033[32m%s\033[0m:\033[34m%s\033[0m$ ", user, cwd);
	return prompt;
}