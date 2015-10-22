# See LICENSE file for copyright and license details.

include config.mk

all: psg2

psg2: psg2.o json.o dmap.o

.c.o: config.mk
	@echo CC -c $<
	@${CC} -c $< ${CFLAGS}

psg2:
	@echo CC -o $@
	@${CC} -o $@ $+ ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f psg2 *.o


update: clean psg2
