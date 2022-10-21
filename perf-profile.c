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

enum MType {
	MIN,
	MAX,
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

static int get_frequency(enum MType typ, int cpu)
{
	const char *rem;
	char *line = NULL, *path;
	size_t nread, len;
	int plen, ret;
	FILE *fd;

	switch(typ) {
	case MIN:
		rem = "/cpufreq/cpuinfo_min_freq";
		break;
	case MAX:
		rem = "/cpufreq/cpuinfo_max_freq";
		break;
	}

	plen = strlen(SYS_CPU_PREFIX) + strlen(rem) + intlen(cpu) + 1;
	path = malloc(plen);
	memset(path, 0, plen);
	snprintf(path, plen, "%s%d%s", SYS_CPU_PREFIX, cpu, rem);

	if ((fd = fopen(path, "r")) == NULL) {
		free(path);
		die("Could not open cpu%d%s file for reading:", cpu, rem);
	}

	nread = getline(&line, &len, fd);
	(void)nread;

	ret = atoi(line);

	free(path);
	free(line);

	return ret;
}

static void set_frequency(enum MType typ, int cpu, int freq)
{
	const char *rem;
	char *path;
	int plen;

	switch(typ) {
	case MIN:
		rem = "/cpufreq/scaling_min_freq";
		break;
	case MAX:
		rem = "/cpufreq/scaling_max_freq";
		break;
	}

	plen = strlen(SYS_CPU_PREFIX) + strlen(rem) + intlen(cpu) + 1;
	path = malloc(plen);
	memset(path, 0, plen);
	snprintf(path, plen, "%s%d%s", SYS_CPU_PREFIX, cpu, rem);

	if (write_oneshot_int(path, freq)) {
		free(path);
		die("Could not open cpu%d%s file for writing:", cpu, rem);
	}
		
	free(path);
}

static void set_governor(int cpu, const char *governor)
{
	const char *rem = "/cpufreq/scaling_governor";
	char *path;
	int plen;

	plen = strlen(SYS_CPU_PREFIX) + strlen(rem) + intlen(cpu) + 1;
	path = malloc(plen);
	memset(path, 0, plen);
	snprintf(path, plen, "%s%d%s", SYS_CPU_PREFIX, cpu, rem);

	if (write_oneshot_str(path, governor)) {
		free(path);
		die("Could not write to cpu%d%s file:", cpu, rem);
	}

	free(path);
}

static void cpufreq_set(enum Profile profile, int max_percent)
{
	const char *governor;
	int i, nproc, min, max;

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

static void set_lowpower(int acpi_platform_supported)
{
	if (acpi_platform_supported) {
		if (write_oneshot_str(ACPI_PLPR_PATH, "low-power"))
			die("Could not open %s for writing:", ACPI_PLPR_PATH);
		return;
	}

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

static void set_balanced(int acpi_platform_supported)
{
	if (acpi_platform_supported) {
		if (write_oneshot_str(ACPI_PLPR_PATH, "balanced"))
			die("Could not open %s for writing:", ACPI_PLPR_PATH);
		return;
	}

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

static void set_performance(int acpi_platform_supported)
{
	if (acpi_platform_supported) {
		if (write_oneshot_str(ACPI_PLPR_PATH, "performance"))
			die("Could not open %s for writing:", ACPI_PLPR_PATH);
		return;
	}

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

	ARGBEGIN {
	case 'v':
		vflag = 1;
		break;
	default:
		usage();
	} ARGEND;

	if (vflag) {
		char *line = NULL;
		size_t len, nread;
		FILE *fd;

		if ((fd = fopen(S76_POW_PROF, "r")) == NULL)
			die("Could not open %s for reading:", S76_POW_PROF);

		nread = getline(&line, &len, fd);
		fclose(fd);
		(void)nread;

		printf("Current profile: %s\n", line);
		free(line);
		return 0;
	}

	if (argc != 1)
		usage();

	if (!access(ACPI_PLPR_PATH, F_OK))
		acpi_platform_supported = 1;

	if (!strcmp(argv[0], "low-power"))
		set_lowpower(acpi_platform_supported);
	else if (!strcmp(argv[0], "balanced"))
		set_balanced(acpi_platform_supported);
	else if (!strcmp(argv[0], "performance"))
		set_performance(acpi_platform_supported);
	else
		usage();

	return write_oneshot_str(S76_POW_PROF, argv[0]);
}
