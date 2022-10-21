.POSIX:

# Install prefix
PREFIX = /usr/local

# Common flags
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Werror -Os
LDFLAGS = -s

# Common headers and objects
HDR = arg.h common.h
OBJ = common.o

# static suid binaries
SUID_BIN = brightness charge-thresholds perf-profile
BRIGHTNESSOBJ = $(OBJ) brightness.o
CHARGETHRESHOLDSOBJ = $(OBJ) charge-thresholds.o
PERFPROFILEOBJ = $(OBJ) perf-profile.o

all: $(SUID_BIN)

$(BRIGHTNESSOBJ) $(CHARGETHRESHOLDSOBJ) $(PERFPROFILEOBJ): $(HDR)

clean:
	rm -f $(BRIGHTNESSOBJ) $(CHARGETHRESHOLDSOBJ) $(PERFPROFILEOBJ)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(SUID_BIN) $(DESTDIR)$(PREFIX)/bin
	cd $(DESTDIR)$(PREFIX)/bin && chmod 4711 $(SUID_BIN)
	cd $(DESTDIR)$(PREFIX)/bin && chmod u+s $(SUID_BIN)

uninstall:
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(SUID_BIN)

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<

brightness: $(BRIGHTNESSOBJ)
	$(CC) -o $@ $(BRIGHTNESSOBJ) $(LDFLAGS) -static

charge-thresholds: $(CHARGETHRESHOLDSOBJ)
	$(CC) -o $@ $(CHARGETHRESHOLDSOBJ) $(LDFLAGS) -static

perf-profile: $(PERFPROFILEOBJ)
	$(CC) -o $@ $(PERFPROFILEOBJ) $(LDFLAGS) -static

.PHONY: all clean install uninstall
