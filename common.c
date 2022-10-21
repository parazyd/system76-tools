#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}

int intlen(int n)
{
	if (n < 0) return intlen(-n) + 1;
	if (n < 10) return 1;
	if (n < 100) return 2;
	if (n < 1000) return 3;
	if (n < 10000) return 4;
	if (n < 100000) return 5;
	if (n < 1000000) return 6;
	if (n < 10000000) return 7;
	if (n < 100000000) return 8;
	if (n < 1000000000) return 9;
	return 10;
}

int write_oneshot_str(const char *path, const char *text)
{
	FILE *fd;

	if ((fd = fopen(path, "w")) == NULL)
		return 1;

	fprintf(fd, text);
	fclose(fd);

	fprintf(stderr, "%s: %s\n", path, text);
	return 0;
}

int write_oneshot_int(const char *path, int value)
{
	FILE *fd;

	if ((fd = fopen(path, "w")) == NULL)
		return 1;

	fprintf(fd, "%d", value);
	fclose(fd);

	fprintf(stderr, "%s: %d\n", path, value);
	return 0;
}
