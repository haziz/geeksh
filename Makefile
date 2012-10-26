#
# Geek Shell - Makefile
#

PROG=	geeksh
SRCS=	${PROG}.c builtins.c utils.c

CFLAGS=	-W -Wall -Wundef -Wendif-labels -Wshadow -Wpointer-arith	\
	-Wcast-align -Wwrite-strings -pedantic -Wstrict-prototypes	\
	-Wmissing-prototypes -Wnested-externs -Winline 			\
	-Wdisabled-optimization -funit-at-a-time -g

LDFLAGS=-lreadline

# NetBSD
#LIBS=	-L/usr/pkg/lib -R/usr/pkg/lib -I/usr/pkg/include

all:
	${CC} ${LIBS} ${LDFLAGS} ${CFLAGS} -o ${PROG} ${SRCS}
clean:
	rm -f ${PROG} ${PROG}.core

