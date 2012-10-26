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

#ifdef PATH_MAX
static int      pathmax = PATH_MAX;
#else
static int      pathmax = 0;
#endif
#define PATH_MAX_GUESS  1024

#include "geeksh.h"	/* Common definitions */

/*
 * Search the given string for a comment symbol and, if found, it's replaced
 * with a null-byte character.
*/
void
remove_comment(char *p)
{
	register int i = 0;

	while( p[i] && p[i] != '#' )
		++i;

	if( p[i] )
		p[i] = '\0';

} /* eof remove_comment() */

/*
 * Show the arguments list for the command _cmd_. If _descr_ is non-zero then
 * a short description is print first.
*/
void
cmdhelp(const char *cmd, int descr)
{
	int i;

	for(i = 0; tblkeys[i].name; i++) {
		if( ! strncmp(tblkeys[i].name, cmd, strlen(tblkeys[i].name)) ) {
			if( descr )
				printf("%s\n", tblkeys[i].desc);
			printf("Usage: %s %s\n", tblkeys[i].name, tblkeys[i].args);
			break;
		}
	}

	if( ! tblkeys[i].name )
		warnx("%s: unknown command", cmd);

} /* eof cmdhelp() */

/*
 * Strip the spaces from _string_ and return the new clean string or NULL if
 * it's empty.
*/
char *
strip_spaces(char *string)
{
	char *s, *e;

	/* Strip leading characters */
	for(s = string; *s == ' ' && *s; s++)
		;

	if( ! *s )
		return s;

	/* Strip trailing characters */
	for(e = s + strlen(s) - 1; *e == ' '; e--)
		;

	*(e + 1) = '\0';

	return s;
} /* eof strip_spaces() */

/*
 * Dynamically allocate a pathname.
*/
char *
path_alloc(size_t *size)
{
	char *ptr;

	if( ! pathmax ) {
		errno = 0;
		if ( (pathmax = pathconf("/", _PC_PATH_MAX)) < 0) {
			if( ! errno )
				pathmax = PATH_MAX_GUESS;
			else
	    			warnx("pathconf caused error while reading _PC_PATH_MAX");
		}
		else
			++pathmax;
	}

	if ( (ptr = malloc(pathmax + 1)) == NULL )
		warnx("cannot allocate the pathname");

	if (size != NULL)
		*size = pathmax + 1;

	return(ptr);
} /* eof path_alloc() */

