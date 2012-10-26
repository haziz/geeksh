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

#include <pwd.h>
#include "geeksh.h"	/* Common definitions */

extern char **environ;

/*
 * Call exit(3) making the C compiler happy :-) If an exit status is given then
 * it's used instead.
*/
void
do_exit(int argc, char *argv[])
{

	if( argc > 2 ) {
		cmdhelp(argv[0], 0);
		return;
	}

	/*
	 * XXX: are there processes in background?
	*/

	exit((argv && argv[1] ? atoi(argv[1]) : EXIT_SUCCESS)); /* Make C compiler happy! */

} /* eof do_exit() */

/*
 * Print to the standard output the given list of arguments.
*/
void
do_echo(int argc, char *argv[])
{
	int i = 0;

	/* Echoing the arguments */
	while(argv[++i]) {
		printf("%s", argv[i]);

		if( i == argc )
			break;

		printf(" ");
	}

	printf("\n");

} /* eof do_echo() */

/*
 * Add the given list of pairs variable=value to the
 * environment.
*/
void
do_putenv(int argc, char *argv[])
{
	size_t envsz;
	int i;
	char *envstr;

	if( argc == 1 ) {
		cmdhelp(argv[0], 0);
		return;
	}

	for(i = 0; i < argc; i++) {

		envsz = strlen(argv[i]) + 1;

		if( (envstr = malloc(envsz)) == NULL )
			return;

		strncpy(envstr, argv[i], envsz);

		if( putenv(envstr) )
			free(envstr);
	}

} /* eof do_putenv() */

/*
 * Add the given pair variable/value to the environment.
*/
void
do_setenv(int argc, char *argv[])
{
	if( argc != 3 ) {
		cmdhelp(argv[0], 0);
		return;
	}

	setenv(argv[1], argv[2], 1);

} /* eof do_setenv() */

/*
 * Remove the given list of variables from the environment.
*/
void
do_unsetenv(int argc, char *argv[])
{
	if( argc == 1 ) {
		cmdhelp(argv[0], 0);
		return;
	}

	while( --argc && ! unsetenv(argv[argc]) )
		;

} /* eof do_unsetenv() */

/*
 * Print the pair variable=value for any of the given
 * environment variables.
*/
void
do_getenv(int argc, char *argv[])
{
	int i;

	if( argc == 1 ) {
		cmdhelp(argv[0], 0);
		return;
	}

	for(i = 1; i < argc; i++)
		printf("%s=%s\n", argv[i], getenv(argv[i]));

} /* eof do_getenv() */

/*
 * Change the current working directory to the given path. If no path is
 * specified then the user home directory is used instead. The '-'
 * meta-directory goes to the directory specified by the OLDPWD environment
 * variable, if any.
*/
void
do_cd(int argc, char *argv[])
{
	struct passwd *pwd;
	size_t tplen;
	char *tp, *path;

	if( argc > 2 ) {
		cmdhelp(argv[0], 0);
		return;
	}

	if( ! (path = argv[1]) ) {
		if( ! (path = getenv("HOME")) ) {
			if( (pwd = getpwuid(getuid())) != NULL )
				path = pwd->pw_dir;

			else { /* This should never happen */
				warnx("%s: cannot get the user home directory", argv[0]);
				return;
			}
		}
	}

	if( *path == '-' && ! path[1] ) {
		if( ! (path = getenv("OLDPWD")) )
			return;
		printf("%s\n", path); /* TODO: An option should toggle this call */
	}

	tp = path_alloc(&tplen);
	getcwd(tp, tplen);
	if( chdir(path) ) {
		warn("cd: %s", path);
		return;
	}
	setenv("OLDPWD", tp, 1);

} /* eof do_cd() */

/*
 * Show the full list of the environment variables. The number of maximum
 * variables to show may be passed in order to control the listing length.
*/
void
do_environ(int argc, char *argv[])
{
	int i, max;

	if( argc > 2 ) {
		cmdhelp(argv[0], 0);
		return;
	}

	for(i = 0, max = (argc == 2 ? atoi(argv[1]) : -1); environ[i] && max; i++, max--)
		printf("%s\n", environ[i]);

} /* eof do_environ() */

/*
 * Print a full list of the builtins commands. If a command name is given, then
 * its passed as argument for cmdhelp() in order to show the relative help
 * message.
*/
void
do_help(int argc, char *argv[])
{
	int i;

	if( argc >= 2 ) {
		cmdhelp(argv[1], argc == 2);
		return;
	}

	/* Few informations */
	printf("\n%s v.%s\n\n", SH_NAME, SH_VERSION);

	for(i = 0; tblkeys[i].name; i++)
		printf(" %-9s%s\n", tblkeys[i].name, tblkeys[i].desc);

	printf("\n");

} /* eof do_help() */

