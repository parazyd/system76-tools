#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char *START_FD = "/sys/class/power_supply/BAT0/charge_control_start_threshold";
static const char *END_FD = "/sys/class/power_supply/BAT0/charge_control_end_threshold";

void usage(void)
{
	printf("usage: charge-thresholds full-charge|balanced|max-lifespan\n\n");
    printf("Profiles:\n");

    printf("full-charge: Battery is charged to its full capacity for\n"
    	"the longest possible use on battery power.\n"
    	"Charging resumes when the battery falls below 96%%.\n\n");

	printf("balanced: Use this threshold when you unplug frequently but\n"
		"don't need the full battery capacity. Charging stops when the\n"
		"battery reaches 90%% capacity and resumes when the battery\n"
		"falls below 85%%.\n\n");

    printf("max-lifespan: Use this threshold if you rarely use the system\n"
    	"on battery for extended periods. Charging stops when the battery\n"
    	"reaches 60%% capacity and resumes when the battery falls below 50%%\n");

    exit(1);
}

void die(const char *m)
{
    perror(m);
    exit(1);
}

int main(int argc, char *argv[])
{
    int start, end; 
    FILE *fd;

    if (argc != 2)
    	usage();

    if (!strcmp(argv[1], "full-charge")) {
        start = 96;
        end = 100;
    } else if (!strcmp(argv[1], "balanced")) {
        start = 86;
        end = 90;
    } else if (!strcmp(argv[1], "max-lifespan")) {
        start = 50;
        end = 60;
    } else {
        usage();
    }

    /* Without this, setting start threshold may fail if the previous end
     * threshold is higher
     */
    if ((fd = fopen(END_FD, "w")) == NULL)
        die("fopen");

    fprintf(fd, "100");
    fclose(fd);
    fd = NULL;

    if ((fd = fopen(START_FD, "w")) == NULL)
        die("fopen");

    fprintf(fd, "%d", start);
    fclose(fd);
    fd = NULL;

    if ((fd = fopen(END_FD, "w")) == NULL)
        die("fopen");

    fprintf(fd, "%d", end);
    fclose(fd);
    fd = NULL;

    printf("Thresholds set to: %d-%d\n", start, end);
    return 0;
}
