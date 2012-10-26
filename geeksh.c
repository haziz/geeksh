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

#include <errno.h>
#include <signal.h>
#include <wordexp.h>
#include <sys/wait.h>

#include "geeksh.h"	/* Common definitions */

/*
 * Avoid some NetBSD obscure build time warning
*/
#if defined(__NetBSD__)
# define _FUNCTION_DEF
# define USE_VARARGS
# define PREFER_STDARG
#endif

#include <readline/readline.h>
#include <readline/history.h>

int
main(void)
{
	char *buf, *p;

	/* Initialize the shell */
	if( sh_init() )
		return EXIT_FAILURE;

	/* Main loop */
	while(1) {

		/* Get the command line buffer */
		if( (buf = readline(getenv("PS1"))) == NULL )
			break;

		remove_comment(p); /* Remove the comment */

		/* Ignore empty lines */
		if( *(p = strip_spaces(buf)) ) {
			add_history(p);	/* Add the line into the history buffer */
			execute_command(p);	/* Execute the command */
		}

		free(buf);
	}

	printf("\n");

	return EXIT_SUCCESS;	/* Make C compiler happy! */
} /* eof main() */

/*
 * Call the specified builtin function and return zero. If the function is not
 * found 1 is returned instead.
*/
int
run_key(int argc, char *argv[])
{
	register int i;

	for(i = 0; tblkeys[i].name; i++) {
		if( ! strncmp(tblkeys[i].name, argv[0], strlen(tblkeys[i].name)) ) {
			tblkeys[i].func(argc, argv);
			return 0;
		}
	}

	return 1;
} /* eof run_key() */

/*
 * Execute the specified program. If it's not executable, then the relative
 * process terminate with an exit status of 126; if an error occurred an exit
 * status of 127 is used instead, assuming the command was not found.
*/
int
run_cmd(int argc, char *argv[])
{
	register pid_t pid;

	(void)argc; /* Avoid a C compiler warning */

	if( (pid = fork()) == -1 )
		return -1;

	else if( pid == 0 ) {	/* child */

		/* Run the given command line */
		execvp(argv[0], argv);

		/*
		 * Exit status
		 *
		 *    126: The program is not executable
		 *    127: The program was not found
		 *      1: any other error
		*/

		if( errno == ENOENT )
			errx(127, "%s: command not found", argv[0]);
		else
			err((errno == EACCES ? 126 : 1), "%s", argv[0]);
	}
	else {			/* parent */
		while( waitpid(pid, NULL, 0) == -1 ) {
			if(errno != EINTR)
			return -1;
		}
	}

	return 0;
} /* eof run_cmd() */

/*
 * Handle the SIGINT signal. Technically just (re)initialize the readline's
 * internal state and write a newline. The current buffer is added into the
 * history.
*/
void
sigint_handler(int signum)
{
	char *p;

	(void)signum; /* Avoid a C compiler warning */

	/*
	 * XXX: is it too extreme? It's geek, anyway.
	*/

	/* Ignore empty buffers */
	if( ! *(p = strip_spaces(rl_line_buffer)) )
		return;

	add_history(p);		/* Store it anyway */
	*rl_line_buffer = '\0';	/* Clear the buffer */
	rl_point =rl_end = 0;	/* Reset the offsets */
	rl_redisplay();		/* Update the line */
	printf("(interrupt)\n");	/* Show a message */
	rl_initialize();		/* (Re)Initialize the state */
	rl_redisplay();		/* Update the line */

} /* sigint_handler() */

/*
 * Initialize the shell.
 * This function is used to install all the needed signal handlers, create a
 * default environment, load the shell startup file and call all initialization
 * routines. The function return zero on success or non-zero if something went
 * wrong.
*/
int
sh_init()
{
	signal(SIGINT, sigint_handler);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	putenv((char *)"PS1=GeekSH: ");	/* Default prompt */
	putenv((char *)"IFS= \n\t");		/* Internal fields separator */

	/* Set the default PATH, if unset (LFS 1.2) */
	if( ! getenv("PATH") )
		putenv((char *)"PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin");

	/*
	 * XXX: load the startup file
	*/

	return 0;
} /* eof sh_init() */

/*
 * Execute the argument _command_ either as builtin or external command.
*/
void
execute_command(char *command)
{
	wordexp_t wordv;
	register int wordc, wret;

	/*
	 * XXX: would be good to use the WRDE_REUSE flag for wordexp(3) when
	 *      it's called from a loop (likely when the buffer will need to be
	 *      parsed for meta-characters regarding pipelines, job control,
	 *      I/O redirect, etc.).
	*/

	/* Expands the wildcards */
	if( (wret = wordexp(command, &wordv, 0)) ) {

		/* Show the "selected" error message */
		switch(wret) {
			case WRDE_BADCHAR:
				warnx("illegal character");
				break;
			case WRDE_NOSPACE:
				warnx("out of memory");
				break;
			case WRDE_SYNTAX:
				warnx("syntax error");
				break;
		}

		return;
	}

	/* Avoid expanded empty lines */
	if( ! wordv.we_wordv[0] ) {
		wordfree(&wordv);
		return;
	}

	/* Count the words */
	for(wordc = 0; wordv.we_wordv[wordc]; wordc++)
		;

	/* Try to run the command as builtin first */
	if( run_key(wordc, wordv.we_wordv) ) {

		/* It's likely an external command */
		if( run_cmd(wordc, wordv.we_wordv) )
			warn("run_cmd");

	}

	wordfree(&wordv);

} /* eof execute_command() */

