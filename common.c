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

int write_oneshot_str(const char *path, const char *text)
{
	FILE *fd;

	if ((fd = fopen(path, "w")) == NULL)
		return 1;

	fputs(text, fd);
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
