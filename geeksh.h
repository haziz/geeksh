/*
 * Very simple and small shell
 * Copyright (C) 2008  Claudio M. Alessi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <err.h>

#define SH_NAME		"gsh"	/* Shell name */
#define SH_VERSION	"0x01"	/* Shell version */

/*
 * Builtins function prototypes
*/
typedef void builtin(int, char *[]);
builtin
	do_exit,
	do_echo,
	do_putenv,
	do_setenv,
	do_unsetenv,
	do_getenv,
	do_environ,
	do_help,
	do_cd
; /* builtin */

/*
 * Builtin keywords
*/
typedef struct {
	const char *name;
	void (*func)(int argc, char *argv[]);
	const char *desc;
	const char *args;
} keyword;

/* Builtin commands table */
static const keyword tblkeys[] = {

	/*
	* Command		Function
	* Description
	* Arguments
	*/

	{ "exit",		do_exit,
  	  "Exit from the shell",
	  "[exit code]"
	},

	{ "echo",		do_echo,
	  "Show a message",
	  "[message]"
	},

	{ "putenv",		do_putenv,
	  "Set the enviroment given variable(s)",
	  "<variable>=<value> [...]"
	},

	{ "setenv",		do_setenv,
	  "Set an environment variable",
	  "<variable> <value>"
	},

	{ "unsetenv",		do_unsetenv,
	  "Unset the environment given variable(s)",
	  "<variable> [...]"
	},

	{ "getenv",		do_getenv,
	  "Print the value of the given environment variable(s)",
	  "<variable> [...]"
	},

	{ "cd",			do_cd,
	  "Change the current working directory",
	  "[pathname | -]"
	},

	{ "environ",		do_environ,
	  "Show a list of the environment variables",
	  "[#limit]"
	},

	{ "help",		do_help,
	  "Show this help",
	  "[command]"
	},

	/* End of the list */
	{ NULL, NULL, NULL, NULL }

}; /* keywords */

/*
 * Function prototypes 
*/
int sh_init(void);
int run_cmd(int argc, char *argv[]);
int run_key(int argc, char *argv[]);
void remove_comment(char *p);
void hdr_sigint(int sig);
void cmdhelp(const char *cmd, int descr);
void sigint_handler(int signum);
void execute_command(char *command);
char *strip_spaces(char *string);
char *path_alloc(size_t *size);

