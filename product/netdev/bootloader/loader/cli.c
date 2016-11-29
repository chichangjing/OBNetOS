/* includes. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "cli.h"

#define CLI_BUFFER_SIZE 	256
#define CLI_HISTORY_DEPTH	16
#define CLI_MAX_ARGS		16
#define CLI_PROMPT_SIZE		16
#define FSM_START			0x00
#define FSM_RX_FIRST_CHAR	0x01
#define FSM_RX_SECOND_CHAR	0x02

unsigned char DirectKeyFSM = 0;

static void help_top(int argc, char **argv);

static char         cli_input_buffers[CLI_HISTORY_DEPTH][CLI_BUFFER_SIZE + 1] = {{0}};
static char         cli_trash_buffer[CLI_BUFFER_SIZE+1] = {0};
static int          cli_cur_buffer_index = 0;		/* Buffer array index */
static int          cli_cur_index = 0;				/* Buffer index */
static int          cli_cur_context = 0;			/* Current context */
static char         cli_prompt[CLI_PROMPT_SIZE+1] = "CLI";
static char         cli_context_buffer[CLI_BUFFER_SIZE+1] = {0};
static cli_cmd_t   *cli_cur_cmd_ctx = 0;
static cli_cmd_t    cli_top_help = { 0,"help","Display help","<cr>", help_top,0,0,0 };
static cli_cmd_t   *cli_root_cmd = &cli_top_help;

enum { NULLMATCH,FULLMATCH,PARTMATCH,UNMATCH,MATCH,AMBIG };

/****************************************************************************************
	Full match    : s2 equal to s1
	Partial match : s1 starts with s2 but there are remaining chars in s1
	Not match     : s1 does not start with s2
 ****************************************************************************************/
static int check_match(char *s1, char *s2)
{
	while(*s1 && *s1==*s2) { s1++; s2++; }

	if(*s2==' ' || *s2==0) {
		if(*s1==0)
			return FULLMATCH;	/* full match */
		else
			return PARTMATCH;	/* partial match */ 
	} else
		return UNMATCH;			/* no match */
}


/****************************************************************************************
	Display the top help
 ****************************************************************************************/
static void help_top(int argc, char **argv)
{
	printf("\r\nSpecial keys list :\r\n");
	printf("    ?         .... Display available commands\r\n");
	printf("   <Backspce> .... Delete previous character\r\n");
	printf("   <Tab>      .... Command-line auto-completion\r\n");
	printf("   <Cr>       .... Execute current command line\r\n");
	printf("   <Up>       .... Goto previous line in history buffer\r\n");
	printf("   <Down>     .... Goto next line in history buffer\r\n");
	printf("\r\n");
	printf("Help may be requested at any point in a command by entering\r\n"); 
	printf("a question mark '?'. If nothing matches, the help list will\r\n");
	printf("be empty and you must backup until entering a '?' shows the\r\n");
	printf("available options.\r\n\r\n");
}

/****************************************************************************************
	Display subcommands help
 ****************************************************************************************/
static void help_child(cli_cmd_t *cmd)
{
	cli_cmd_t *cm;
	int len=0;

	printf("\r\n");
	for(cm=cmd;cm;cm=cm->next)
		if(len<strlen(cm->name))
			len=strlen(cm->name);
	for(cm=cmd;cm;cm=cm->next)
		if(cm->help) {
			int i;
			printf("%s", cm->name);
			for(i=strlen(cm->name);i<len+2;i++)
				printf(" ");
			printf("... %s", cm->help);
			printf("\r\n");
		}
}

/****************************************************************************************
	Switch to specific context ( child command switch )
 ****************************************************************************************/
/*
static void command_do_context(cli_cmd_t *cmd, char *str)
{
	while(*str) 
		cli_context_buffer[cli_cur_context++]=*str++;
	cli_context_buffer[cli_cur_context]=0;
	cli_cur_cmd_ctx=cmd;
}
*/
/****************************************************************************************
  * @brief  Parse input string
  * @param  _cmd: point to first command
  *         _str: point to current input string
  * @retval 
  ***************************************************************************************/
static int command_parse(cli_cmd_t **_cmd, char **_str)
{
	char *str=*_str;
	cli_cmd_t *cmd;
	cli_cmd_t *matched_cmd=0;

	/* Eliminate blanks before input string */
	while(*str==' ') str++;
		if(!*str) {
			*_str=str;
		return NULLMATCH;
	}

	/* Trace the command chain to match in sequence */
	for(cmd=*_cmd; cmd; cmd=cmd->next) {
		int ret=check_match(cmd->name,str);

		if(ret==FULLMATCH) {
			while(*str && *str!=' ') str++; 
			while(*str==' ') str++;
			*_str=str;
			*_cmd=cmd;
			return MATCH;
		} else if (ret==PARTMATCH) {
			if(matched_cmd) {
				*_cmd=matched_cmd;
				return AMBIG;
			} else {
				matched_cmd=cmd;
			}
		} else {/* UNMATCH */

		}
	}

	if(matched_cmd) {
		while(*str && *str!=' ') str++; 
		while(*str==' ') str++;
		*_cmd=matched_cmd;
		*_str=str;
		return MATCH;
	} else
		return UNMATCH;
}

/****************************************************************************************
	Extrat the arguments, and execute command by call fuction
 ****************************************************************************************/
static void command_execute(cli_cmd_t *cmd, char *str)
{
	char *argv[CLI_MAX_ARGS];
	int argc=0;
	int i;

	/* Save to history */
	for(i=0;i<CLI_BUFFER_SIZE;i++)
		cli_trash_buffer[i]=str[i];
	str=cli_trash_buffer;

	/* Arguments extract */
	argv[argc++]=cmd->name;
	while(*str && argc<CLI_MAX_ARGS) {
		while(*str==' ') str++;
		if(*str==0)
			break;
		argv[argc++]=str;
		while(*str!=' ' && *str) str++;
		if(!*str) break;
			*str++=0;
	}
	
	/* Call command function */
	if(cmd->function) {
		cmd->function(argc,&argv[0]);
	}
}

/****************************************************************************************
  * @brief  Execute the current command line
  * @param   cmd: point to command word
  *         _str: point to current input string
  * @retval 0
  ***************************************************************************************/
static int command_line_execute(cli_cmd_t *cmd, char *_str)
{
	char *str=_str;

	while(1) {
		int ret;
		ret=command_parse(&cmd,&str);
		if(ret==MATCH) {									/* Match */
			if(cmd) {
				if(!cmd->child) {
					/* no sub-command, execute */
					command_execute(cmd,str);
					return 0;
				} else {
					if(*str==0) {
						/* no more input, this is a context */
						help_child(cmd->child);
						return 0;
					} else {
						/* process next command */
						cmd=cmd->child;
					}
				}
			} else {
				return 0;
			}
		} else if(ret==AMBIG) {		/* Ambiguity */
			printf("%% Ambiguous command.\r\n");
			return 0;
		} else if(ret==UNMATCH) {							/* Unmatch */
			printf("%% Invalid command.\r\n");
			return 0;
		} else /* NULLMATCH */
			return 0;
	}
}

/****************************************************************************************
  * @brief  Auto-complete current command line
  * @param  cmd: command match witch input string
  *         str: input string, return next input
  * @retval 
  ***************************************************************************************/
static int command_complete(cli_cmd_t *cmd, char *_str)
{
	char *str=_str;

	while(1) {
		int ret;
		int common_len=CLI_BUFFER_SIZE;
		int _str_len;
		int i;
		char *__str=str;

		ret=command_parse(&cmd,&str);
		for(_str_len=0;__str[_str_len]&&__str[_str_len]!=' ';_str_len++);
		if(ret==MATCH && *str) {
			cmd=cmd->child;
		} else if(ret==AMBIG || ret==MATCH || ret==NULLMATCH) {
			cli_cmd_t *cm;
			cli_cmd_t *matched_cmd=0;
			int nb_match=0;

			for(cm=cmd;cm;cm=cm->next) {
				int r=check_match(cm->name,__str);
				if(r==FULLMATCH) {
					for(i=_str_len;cmd->name[i];i++)
						cli_char_input(cmd->name[i]);
					if(*(str-1)!=' ')
						cli_char_input(' ');
					if(!cmd->child) {
						if(cmd->usage) {
							printf("%s", cmd->usage);
							printf("\r\n");
							return 1;
						} else
							return 0;
					} else {
						cmd=cmd->child;
						break;
					}
				} else if(r==PARTMATCH) {
					nb_match++;
					if(!matched_cmd) {
						matched_cmd=cm;
						common_len=strlen(cm->name);
					} else {
						for(i=_str_len;cm->name[i] && i<common_len &&
							cm->name[i]==matched_cmd->name[i];i++);
						if(i<common_len)
							common_len=i;
					}
				}
			}
			if(cm)
				continue;
			if(matched_cmd) {
				if(_str_len==common_len) {
					printf("\r\n");;
					for(cm=cmd;cm;cm=cm->next) {
						int r=check_match(cm->name,__str);
						if(r==FULLMATCH || r==PARTMATCH) {
							printf("%s", cm->name);
							printf("\r\n");
						}
					}
					return 1;
				} else {
					for(i=_str_len;i<common_len;i++)
						cli_char_input(matched_cmd->name[i]);
					if(nb_match==1)
						cli_char_input(' ');
				}
			}
			return 0;
		} else {/* UNMATCH */	
			return 0;
		}
	}
}

static int command_help(cli_cmd_t *cmd, char *_str)
{
	char *str=_str;

	while(1) {
		int ret;
		ret=command_parse(&cmd,&str);

		/* Found the unique command or empty line */
		if(ret==MATCH && *str==0) {
			if(cmd->child) {
				/* display sub-commands help */
				help_child(cmd->child);
				return 0;
			} else  {
				#if 0
				/* if no sub-command, show single help */
				if(*(str-1)!=' ')
					printf(" ");
				if(cmd->usage)
					printf("%s",cmd->usage);
				printf(": ");
				if(cmd->help)
					printf("%s",cmd->help);
				else
					printf("No available help");
				printf("\r\n");
				#else
				/* if no sub-command, show single help */
				if(*(str-1)!=' ')
					printf(" ");
				printf("\r\n");
				printf("%s",cmd->name);
				if(cmd->help) {
					int i, len;
					for(i=len=strlen(cmd->name);i<len+2;i++)
						printf(" ");
					printf("... %s", cmd->help);					
				} else {
					printf("\t: No available help");
				}
				printf("\r\n");			
				#endif
			}
			return 0;
		} else if(ret==MATCH && *str)	{	
			cmd=cmd->child;
		} else if(ret==AMBIG) {
			printf("\r\n%% Ambiguous command.\r\n");
			return 0;
		} else if(ret==UNMATCH) {
			printf("\r\n%% Invalid command.\r\n");
			return 0;
		} else {
			if(cli_cur_cmd_ctx)
				help_child(cli_cur_cmd_ctx->child);
			else
				help_child(cli_root_cmd);
			return 0;
		}
	}
}

static void cli_start_line()
{
	printf(cli_prompt);
	if(cli_cur_context) {
		printf("%s", cli_context_buffer);
		printf("> ");
	}
	cli_cur_index=0;
}

void cli_key_enter(void)
{
	char *line = cli_input_buffers[cli_cur_buffer_index];
	cli_cmd_t *cmd;

	/* Echo the newline */
	printf(" \b");
	printf("\r\n");

	while(*line && *line==' ') line++;
	if(*line) {/* not empty line */
		cmd = cli_cur_context? cli_cur_cmd_ctx->child : cli_root_cmd;
		command_line_execute(cmd,line);
		cli_cur_buffer_index=(cli_cur_buffer_index+1 )%CLI_HISTORY_DEPTH;
		cli_cur_index = 0;
		cli_input_buffers[cli_cur_buffer_index][0] = 0;
	}
	cli_start_line();
}

void cli_key_backspace(void)
{
	char *line = cli_input_buffers[cli_cur_buffer_index];
	
	if(cli_cur_index > 0) {
		printf("\b \b");
		cli_cur_index--;
		line[cli_cur_index] = 0;
	}
	printf(" \b");
}

void cli_key_tab(void)
{
	char *line = cli_input_buffers[cli_cur_buffer_index];
	cli_cmd_t *cmd;
	
	cmd=cli_cur_cmd_ctx?cli_cur_cmd_ctx->child : cli_root_cmd;
	if(command_complete(cmd,line)) {
		cli_start_line();
		printf("%s",line);
	}
	cli_cur_index=strlen(line);
    printf(" \b");
}

void cli_key_question(void)
{
	char *line = cli_input_buffers[cli_cur_buffer_index];
	cli_cmd_t *cmd;
	
	cmd=cli_cur_cmd_ctx?cli_cur_cmd_ctx->child : cli_root_cmd;
	command_help(cmd,line);
	cli_start_line();
	printf("%s",line);
	cli_cur_index=strlen(line);	
}

void cli_key_ctrlc(void)
{

}

void cli_key_up(void)
{
	char *line;
	int prevline = (cli_cur_buffer_index+CLI_HISTORY_DEPTH-1)%CLI_HISTORY_DEPTH;

	if(cli_input_buffers[prevline][0]) {
		line = cli_input_buffers[prevline];
		while(cli_cur_index-- > strlen(line)) printf("\b \b");
		printf("\r");
		cli_start_line();
		printf(line);
		cli_cur_index = strlen(line);
		cli_cur_buffer_index = prevline;
	}
}

void cli_key_down(void)
{
	char *line = cli_input_buffers[cli_cur_buffer_index];
	int nextline=(cli_cur_buffer_index+1)%CLI_HISTORY_DEPTH;

	if(cli_input_buffers[nextline][0]) {
		line=cli_input_buffers[nextline];
		while(cli_cur_index-- > strlen(line))
			printf("\b \b");
		printf("\r");
		cli_start_line();
		printf("%s",line);
		cli_cur_index=strlen(line);
		cli_cur_buffer_index=nextline;
	}
}

void cli_char_input(char c)
{
	char *line = cli_input_buffers[cli_cur_buffer_index];

	switch(DirectKeyFSM) {
		case FSM_START:
		if(c == 0x1B) {
			DirectKeyFSM = FSM_RX_FIRST_CHAR;
			return;
		}
		break;

		case FSM_RX_FIRST_CHAR:
		if(c == 0x5B) 
			DirectKeyFSM = FSM_RX_SECOND_CHAR;
		else 
			DirectKeyFSM = FSM_START;
		return;
		break;

		case FSM_RX_SECOND_CHAR:
			switch(c) {
				case 0x41:	cli_key_up();		break;	/* Up */
				case 0x42:	cli_key_down();		break;	/* Down */
				default: break;
			}
			DirectKeyFSM = FSM_START;
			return;
		break;

		default:
		break;
	}

	switch(c) {
		case 0x0D:	cli_key_enter();		break;	/* <CR> */
		case 0x09:	cli_key_tab();			break;	/* <Tab> */
		case 0x3F:	cli_key_question();		break;	/* '?' */
		case 0x08:	cli_key_backspace();	break;	/* Backspace */
		case 0x03:	cli_key_ctrlc();		break;	/* Ctrl-C */
		default:
			if((c >= 0x20 && c <= 0x7e) && (cli_cur_index < CLI_BUFFER_SIZE)) {
				printf("%c", c);
				line[cli_cur_index++]=c;
				line[cli_cur_index]=0;
			}
	}	
}


/****************************************************************************************
  * @brief  Register a CLI command 
  * @param  cmd: Command will be registered
  * @retval Null
  ***************************************************************************************/
void cli_register_command(cli_cmd_t *cmd)
{
	cli_cmd_t *cm;

	if(cmd->parent){
		cm=cmd->parent->child;
		if(!cm){
			cmd->parent->child=cmd;
		} else{
			while(cm->next) cm=cm->next;
			cm->next=cmd;
		}
	} else if(!cli_root_cmd) {
		cli_root_cmd=cmd;
	} else {
		cm=cli_root_cmd;
		while(cm->next) cm=cm->next;
		cm->next=cmd;      
	}
}

void cli_set_prompt(char *str)
{
	int i;
	for(i=0;str[i] && i<CLI_PROMPT_SIZE;i++)
		cli_prompt[i]=str[i];
	cli_prompt[i]=0;
	cli_char_input('\r');
}




