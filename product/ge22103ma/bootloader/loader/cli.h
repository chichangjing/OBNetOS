#ifndef __CLI_H
#define __CLI_H

typedef void (*cli_fnt_t)(int argc, char **argv);
typedef struct cli_cmd_t {
	struct cli_cmd_t	*parent;	/* 0 if top level command */
	char				*name;		/* command input name, not 0 */
	char				*help;		/* help string, can be 0 */
	char				*usage;		/* usage string, can be 0 */
	cli_fnt_t			function;	/* function to launch on cmd, can be 0 */
	void				*arg;		/* current argument when function called */
	struct cli_cmd_t	*next;		/* must be set to 0 at init */
	struct cli_cmd_t	*child;		/* must be set to 0 at init */
} cli_cmd_t;

#ifdef __cplusplus
extern "C" {
#endif

void cli_char_input(char c);
void cli_register_command(cli_cmd_t *cmd);
void cli_set_prompt(char *str);

#ifdef __cplusplus
}
#endif

#endif

