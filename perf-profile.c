/* suid tool for setting acpi performance profile
 * GPL-3
 * https://www.kernel.org/doc/html/latest/userspace-api/sysfs-platform_profile.html
 * https://mjmwired.net/kernel/Documentation/ABI/testing/sysfs-platform_profile
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include "arg.h"
#include "common.h"

static const char *ACPI_PLPR_PATH = "/sys/firmware/acpi/platform_profile";
static const char *S76_POW_PROF = "/run/system76-power.profile";

static const char *DIRTY_WRITEBACK = "/proc/sys/vm/dirty_writeback_centisecs";
static const char *DIRTY_EXPIRE = "/proc/sys/vm/dirty_expire_centisecs";

static const char *SYS_CPU_PREFIX = "/sys/devices/system/cpu/cpu";

static const char *PSTATE_DYNBOOST = "/sys/devices/system/cpu/intel_pstate/hwp_dynamic_boost";
static const char *PSTATE_MAX_PERF = "/sys/devices/system/cpu/intel_pstate/max_perf_pct";
static const char *PSTATE_MIN_PERF = "/sys/devices/system/cpu/intel_pstate/min_perf_pct";
static const char *PSTATE_NO_TURBO = "/sys/devices/system/cpu/intel_pstate/no_turbo";

char *argv0;

enum Profile {
	LOWPOWER,
	BALANCED,
	PERFORMANCE,
};

static void usage(void)
{
	die("usage: %s [-v] low-power|balanced|performance", argv0);
}

static void set_max_lost_work(int secs)
{
	int centisecs = secs * 100;

	if (write_oneshot_int(DIRTY_EXPIRE, centisecs))
		die("Could not open %s for writing:", DIRTY_EXPIRE);

	if (write_oneshot_int(DIRTY_WRITEBACK, centisecs))
		die("Could not open %s for writing:", DIRTY_WRITEBACK);
}

static int get_frequency(int typ, int cpu)
{
	/* typ is 0 for min, !0 for max */
	FILE *fd;
	char *line = NULL, *path, *rem;
	char ccpu[1]; /* Will break if cpu > 9 */
	size_t nread, len;
	int plen, ret;

	if (typ)
		rem = "/cpufreq/cpuinfo_max_freq";
	else
		rem = "/cpufreq/cpuinfo_min_freq";


	itoa(cpu, ccpu);

	plen = strlen(SYS_CPU_PREFIX) + strlen(rem) + 2;
	path = malloc(plen);
	memset(path, 0, plen);

	path = strcat(path, SYS_CPU_PREFIX);
	path = strcat(path, ccpu);
	path = strcat(path, rem);

	if ((fd = fopen(path, "r")) == NULL) {
		free(path);
		die("Could not open cpu%d min/max file for reading:", cpu);
	}

	free(path);

	nread = getline(&line, &len, fd);
	(void)nread;

	ret = atoi(line);
	free(line);

	return ret;
}

static void set_frequency(int typ, int cpu, int freq)
{
	/* typ is 0 for min, !0 for max */
	char *path, *rem, *ccpu;
	int plen;

	if (cpu > 9) {
		ccpu = malloc(2);
		memset(ccpu, 0, 2);
	} else {
		ccpu = malloc(1);
		memset(ccpu, 0, 1);
	}

	if (typ)
		rem = "/cpufreq/scaling_max_freq";
	else
		rem = "/cpufreq/scaling_min_freq";

	itoa(cpu, ccpu);
	plen = strlen(ccpu) + strlen(SYS_CPU_PREFIX) + strlen(rem);
	path = malloc(plen);
	memset(path, 0, plen);

	path = strcat(path, SYS_CPU_PREFIX);
	path = strcat(path, ccpu);
	path = strcat(path, rem);

	free(ccpu);

	if (write_oneshot_int(path, freq)) {
		free(path);
		die("Could not open cpu%d min/max file for writing:", cpu);
	}
		
	free(path);
}

static void set_governor(int cpu, const char *governor)
{
	char *path, *ccpu;
	char *rem = "/cpufreq/scaling_governor";
	int plen;

	if (cpu > 9) {
		ccpu = malloc(2);
		memset(ccpu, 0, 2);
	} else {
		ccpu = malloc(1);
		memset(ccpu, 0, 1);
	}

	itoa(cpu, ccpu);
	plen = strlen(ccpu) + strlen(SYS_CPU_PREFIX) + strlen(rem);
	path = malloc(plen);
	memset(path, 0, plen);

	path = strcat(path, SYS_CPU_PREFIX);
	path = strcat(path, ccpu);
	path = strcat(path, rem);

	free(ccpu);

	if (write_oneshot_str(path, governor)) {
		free(path);
		die("Could not open cpu%d governor file:", cpu);
	}

	free(path);
}

static void cpufreq_set(enum Profile profile, int max_percent)
{
	int i, nproc;
	int min, max;
	char *governor;

	/* We assume we have intel_pstate */
	switch(profile) {
	case LOWPOWER:
	case BALANCED:
		governor = "powersave";
		break;
	case PERFORMANCE:
		governor = "performance";
		break;
	}

	/* We look at cpu0 but assume they're all the same */
	min = get_frequency(0, 0);
	max = get_frequency(1, 0);

	max = max * MIN(max_percent, 100) / 100;

	nproc = get_nprocs();
	for (i = 0; i < nproc; i++) {
		set_frequency(0, i, min);
		set_frequency(1, i, max);
		set_governor(i, governor);
	}
}

static void set_lowpower(void)
{
	set_max_lost_work(15);
	cpufreq_set(LOWPOWER, 50);

	/* intel_pstate values */
	if (write_oneshot_int(PSTATE_MIN_PERF, 0))
		die("Could not open %s for writing:", PSTATE_MIN_PERF);

	if (write_oneshot_int(PSTATE_MAX_PERF, 50))
		die("Could not open %s for writing:", PSTATE_MAX_PERF);

	if (write_oneshot_int(PSTATE_NO_TURBO, 1))
		die("Could not open %s for writing:", PSTATE_NO_TURBO);
}

static void set_balanced(void)
{
	set_max_lost_work(15);
	cpufreq_set(BALANCED, 100);

	/* intel_pstate values */
	if (write_oneshot_int(PSTATE_DYNBOOST, 1))
		die("Could not open %s for writing:", PSTATE_DYNBOOST);

	if (write_oneshot_int(PSTATE_MIN_PERF, 0))
		die("Could not open %s for writing:", PSTATE_MIN_PERF);

	if (write_oneshot_int(PSTATE_MAX_PERF, 100))
		die("Could not open %s for writing:", PSTATE_MAX_PERF);

	if (write_oneshot_int(PSTATE_NO_TURBO, 0))
		die("Could not open %s for writing:", PSTATE_NO_TURBO);
}

static void set_performance(void)
{
	set_max_lost_work(15);
	cpufreq_set(PERFORMANCE, 100);

	/* intel_pstate values */
	if (write_oneshot_int(PSTATE_DYNBOOST, 1))
		die("Could not open %s for writing:", PSTATE_DYNBOOST);

	if (write_oneshot_int(PSTATE_MIN_PERF, 0))
		die("Could not open %s for writing:", PSTATE_MIN_PERF);

	if (write_oneshot_int(PSTATE_MAX_PERF, 100))
		die("Could not open %s for writing:", PSTATE_MAX_PERF);

	if (write_oneshot_int(PSTATE_NO_TURBO, 0))
		die("Could not open %s for writing:", PSTATE_NO_TURBO);

	/* TODO: PCI runtime pm off */
}

int main(int argc, char *argv[])
{
	int vflag = 0;
	int acpi_platform_supported = 0;
	char *line = NULL;
	size_t len, nread;
	FILE *fd;

	ARGBEGIN {
	case 'v':
		vflag = 1;
		break;
	default:
		usage();
		exit(1);
	} ARGEND;

	if (!access(ACPI_PLPR_PATH, F_OK))
		acpi_platform_supported = 1;


	if (vflag) {
		if (acpi_platform_supported) {
			if ((fd = fopen(ACPI_PLPR_PATH, "r")) == NULL)
				die("Could not open %s for reading:", ACPI_PLPR_PATH);
		} else {
			if ((fd = fopen(S76_POW_PROF, "r")) == NULL)
				die("Could not open %s for reading:", S76_POW_PROF);
		}

		nread = getline(&line, &len, fd);
		fclose(fd);
		(void)nread;

		printf("Current profile: %s\n", line);
		free(line);
		exit(0);
	}

	if (argc != 1)
		usage();

	if (acpi_platform_supported) {
		if ((fd = fopen(ACPI_PLPR_PATH, "w")) == NULL)
			die("Could not open %s for writing:", ACPI_PLPR_PATH);

		if (!strcmp(argv[0], "low-power"))
			fprintf(fd, "low-power");
		else if (!strcmp(argv[0], "balanced"))
			fprintf(fd, "balanced");
		else if (!strcmp(argv[0], "performance"))
			fprintf(fd, "performance");
		else {
			fclose(fd);
			usage();
		}

		fclose(fd);
		printf("Platform profile set to: %s\n", argv[0]);
		return 0;
	}

	if (!strcmp(argv[0], "low-power"))
		set_lowpower();
	else if (!strcmp(argv[0], "balanced"))
		set_balanced();
	else if (!strcmp(argv[0], "performance"))
		set_performance();
	else
		usage();

	return write_oneshot_str(S76_POW_PROF, argv[0]);
}
