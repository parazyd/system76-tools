/* suid tool for setting screen brightness in increments
 * GPL-3
 */
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
	die("usage: %s [-u] [-d] [-z] [-x]\n\n"
	"    -u: brightness up by one increment\n"
	"    -d: brightness down by one increment\n"
	"    -z: brightness to lowest increment\n"
	"    -x: brightness to maximum possible", argv0);
}

enum Op {
	UP,
	DN,
	MN,
	MX,
};

int main(int argc, char *argv[])
{
	enum Op op = 0;
	int max, cur, inc;
	int uflag = 0, dflag = 0, minflag = 0, maxflag = 0;
	char buf[10];
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
	case 'z':
		minflag = 1;
		op = MN;
		break;
	case 'x':
		maxflag = 1;
		op = MX;
		break;
	default:
		usage();
	} ARGEND;

	if ((uflag && dflag) || (maxflag && minflag))
		usage();

	/* Find out max brightness */
	if ((fd = fopen(BRIGHT_MAX, "r")) == NULL)
		die("Couldn't read %s:", BRIGHT_MAX);

	max = atoi(fgets(buf, 10, fd));
	fclose(fd);
	fd = NULL;

	/* Here the number of available increments can be configured */
	inc = max / 20;

	/* Find out current brightness */
	if ((fd = fopen(BRIGHT_CUR, "w+")) == NULL)
		die("Couldn't open %s for r/w:", BRIGHT_CUR);

	cur = atoi(fgets(buf, 10, fd));

	switch(op) {
	case UP:
		fprintf(fd, "%d", cur + inc > max ? max : cur + inc);
		break;
	case DN:
		fprintf(fd, "%d", cur - inc < 1 ? 1 : cur - inc);
		break;
	case MN:
		fprintf(fd, "%d", inc);
		break;
	case MX:
		fprintf(fd, "%d", max);
		break;
	}

	fclose(fd);
	return 0;
}
