.POSIX:

PREFIX = /usr/local
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Werror -Os -static
LDFLAGS = -s -static

SBIN = charge-thresholds brightness

CC = cc

all: $(SBIN)

$(SBIN):
	$(CC) -c $(CFLAGS) $@.c
	$(CC) -o $@ $@.o $(LDFLAGS)

clean:
	for i in $(SBIN); do \
		rm -f $$i.o $$i ; \
	done

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	for i in $(SBIN); do \
		cp -f $$i $(DESTDIR)$(PREFIX)/bin ; \
		chmod 4711 $(DESTDIR)$(PREFIX)/bin/$$i ; \
		chmod u+s $(DESTDIR)$(PREFIX)/bin/$$i ; \
	done

uninstall:
	for i in $(SBIN); do \
		rm -f $(DESTDIR)$(PREFIX)/bin/$$i ; \
	done

.PHONY: all clean install uninstall
