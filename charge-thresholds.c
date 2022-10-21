/* suid tool for setting battery charge thresholds
 * GPL-3
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "arg.h"
#include "common.h"

static const char *START_FD = "/sys/class/power_supply/BAT0/charge_control_start_threshold";
static const char *END_FD = "/sys/class/power_supply/BAT0/charge_control_end_threshold";

char *argv0;

static void usage(void)
{
	die("usage: %s full-charge|balanced|max-lifespan\n\n"
	"full-charge:\n"
	"Battery is charged to its full capacity for longest possible use on\n"
	"battery power. Charging resumes when the battery falls below 96%%.\n\n"

	"balanced:\n"
	"Use this threshold when you unplug frequently but don't need the\n"
	"full battery capacity. Charging stops when the battery reaches 90%%\n"
	"capacity and resumes when it falls below 85%%\n\n"

	"max-lifespan:\n"
	"Use this threshold if you rarely use the system on battery for\n"
	"extended periods. Charging stops when the battery reaches 60%%\n"
	"capacity and resumes when battery falls below 50%%\n", argv0);
}

int main(int argc, char *argv[])
{
    int start, end; 

    ARGBEGIN {
    } ARGEND;

    if (argc != 1)
    	usage();

    if (!strcmp(argv[0], "full-charge")) {
        start = 96;
        end = 100;
    } else if (!strcmp(argv[0], "balanced")) {
        start = 86;
        end = 90;
    } else if (!strcmp(argv[0], "max-lifespan")) {
        start = 50;
        end = 60;
    } else {
        usage();
        return 1;
    }

    /* Without this, setting start threshold may fail if the previous end
     * threshold is higher */
	if (write_oneshot_int(END_FD, 100))
		die("Could not open %s for writing:", END_FD);

	if (write_oneshot_int(START_FD, start))
		die("Could not open %s for writing:", START_FD);

	if (write_oneshot_int(END_FD, end))
		die("Could not open %s for writing:", END_FD);

    return 0;
}
