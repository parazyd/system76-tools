#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char *BRIGHTNESS_MAX = "/sys/class/backlight/intel_backlight/max_brightness";
const char *BRIGHTNESS_CUR = "/sys/class/backlight/intel_backlight/brightness";

void usage(void)
{
	fprintf(stderr, "usage: brightness up|dn\n");
	exit(1);
}

void die(const char *m)
{
	perror(m);
	exit(1);
}

enum Op {
	UP,
	DN,
};

int main(int argc, char *argv[])
{
	enum Op op;
	int new, max, cur, inc;
	size_t len;
	char *line;
	FILE *fd;

	if (argc != 2)
		usage();

	if (!strcmp(argv[1], "up"))
		op = UP;
	else if (!strcmp(argv[1], "dn"))
		op = DN;
	else
		usage();

	/* Find out max brightness */
	if ((fd = fopen(BRIGHTNESS_MAX, "r")) == NULL)
		die("fopen");

	size_t nread = getline(&line, &len, fd);
	(void)nread;
	fclose(fd);
	fd = NULL;

	max = atoi(line);
	free(line);
	line = NULL;

	inc = max / 20;

	if ((fd = fopen(BRIGHTNESS_CUR, "w+")) == NULL)
		die("fopen");

	nread = getline(&line, &len, fd);
	(void)nread;
	cur = atoi(line);
	free(line);
	line = NULL;

	switch(op) {
	case UP:
		if (cur + inc > max)
			new = max;
		else
			new = cur + inc;
		break;
	case DN:
		if (cur - inc < 1)
			new = 1;
		else
			new = cur - inc;
	}

	fprintf(fd, "%d", new);
	fclose(fd);

	return 0;
}
