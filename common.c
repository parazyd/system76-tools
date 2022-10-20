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

void reverse(char *s)
{
	int i, j;
	char c;

	for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

void itoa(int n, char *s)
{
	int i, sign;

	if ((sign = n) < 0)
		n = -n;

	i = 0;
	do {
		s[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);

	if (sign < 0)
		s[i++] = '-';

	s[i] = '\0';
	reverse(s);
}

int write_oneshot_str(const char *path, const char *text)
{
	FILE *fd;

	if ((fd = fopen(path, "w")) == NULL)
		return 1;

	fprintf(fd, text);
	fclose(fd);

	fprintf(stderr, "Wrote into %s: %s\n", path, text);
	return 0;
}

int write_oneshot_int(const char *path, int value)
{
	FILE *fd;

	if ((fd = fopen(path, "w")) == NULL)
		return 1;

	fprintf(fd, "%d", value);
	fclose(fd);

	fprintf(stderr, "Wrote into %s: %d\n", path, value);
	return 0;
}
