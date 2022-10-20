/* suid tool for setting screen brightness in increments
 * GPL-3
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "arg.h"
#include "common.h"

static const char *BRIGHT_MAX = "/sys/class/backlight/intel_backlight/max_brightness";
static const char *BRIGHT_CUR = "/sys/class/backlight/intel_backlight/brightness";

char *argv0;

static void usage(void)
{
	die("usage: %s [-u] [-d]\n\n"
	"    -u: brightness up by one increment\n"
	"    -d: brightness down by one increment", argv0);
}

enum Op {
	UP,
	DN,
};

int main(int argc, char *argv[])
{
	enum Op op = 0;
	int max, cur, inc;
	int uflag = 0, dflag = 0;
	size_t len, nread;
	char *line = NULL;
	FILE *fd;

	ARGBEGIN {
	case 'u':
		uflag =1;
		op = UP;
		break;
	case 'd':
		dflag = 1;
		op = DN;
		break;
	default:
		usage();
	} ARGEND;

	if ((uflag && dflag) || (!dflag && !uflag))
		usage();

	/* Find out max brightness */
	if ((fd = fopen(BRIGHT_MAX, "r")) == NULL)
		die("Couldn't read %s:", BRIGHT_MAX);

	nread = getline(&line, &len, fd);
	fclose(fd);
	fd = NULL;

	max = atoi(line);
	free(line);
	line = NULL;
	
	/* Here the number of available increments can be configured */
	inc = max / 20;

	/* Find out current brightness */
	if ((fd = fopen(BRIGHT_CUR, "w+")) == NULL)
		die("Couldn't open %s for r/w:", BRIGHT_CUR);

	nread = getline(&line, &len, fd);
	(void)nread;

	cur = atoi(line);
	free(line);
	line = NULL;

	switch(op) {
	case UP:
		fprintf(fd, "%d", cur + inc > max ? max : cur + inc);
		break;
	case DN:
		fprintf(fd, "%d", cur - inc < 1 ? 1 : cur - inc);
		break;
	}

	fclose(fd);
	return 0;
}
