VERSION = 0.1


# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\"
CFLAGS   = -std=c11 -pedantic -rdynamic -Wextra -Wall ${CPPFLAGS} -g
LDFLAGS  = ${LIBS}

# compiler and linker
CC = gcc
