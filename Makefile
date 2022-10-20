.POSIX:

# Install prefix
PREFIX = /usr/local

# Common flags
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Werror -Os
LDFLAGS = -s

# static suid binaries
SUID_BIN = brightness charge-thresholds

HDR = arg.h common.h
SRC = common.c
OBJ = $(SRC:.c=.o)

all: $(OBJ) $(SUID_BIN)

$(OBJ): $(HDR)

$(SUID_BIN): $(SUID_BIN:=.c) $(OBJ)
	$(CC) -c $(CFLAGS) $@.c
	$(CC) -o $@ $@.o $(OBJ) $(LDFLAGS) -static

clean:
	rm -f *.o $(SUID_BIN)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	for i in $(SUID_BIN); do \
		cp -f $$i $(DESTDIR)$(PREFIX)/bin ; \
		chmod 4711 $(DESTDIR)$(PREFIX)/bin/$$i ; \
		chmod u+s $(DESTDIR)$(PREFIX)/bin/$$i ; \
	done

uninstall:
	for i in $(SUID_BIN); do \
		rm -f $(DESTDIR)$(PREFIX)/bin/$$i ; \
	done

.PHONY: all clean install uninstall
