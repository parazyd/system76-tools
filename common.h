#ifndef __COMMON_H__
#define __COMMON_H__

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void die(const char *fmt, ...);
int intlen(int n);
int write_oneshot_str(const char *path, const char *text);
int write_oneshot_int(const char *path, int value);

#endif
