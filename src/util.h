#ifndef _UTIL_H
#define _UTIL_H

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef VERSION
#define VERSION "(no version)"
#endif

#define PROGRAM(_s)	const char* __prog_name = (_s)
#define AUTHORS(...)	const char* __authors[] = { __VA_ARGS__, NULL }
#define OPTIONS(_s)	const char* __optstring = (_s)

extern const char* __prog_name;
extern const char* __authors[];
extern const char* __optstring;

static inline void usage(char *ext) {
	fprintf(stderr, "Usage: %s", __prog_name);

	char group[64] = { 0 };
	size_t gpos = 0;
	for (int i = 0; __optstring[i] != '\0'; i++) {
		char opt = __optstring[i];
		if (opt == ':' || opt == '+' || opt == '-') continue;
		if (__optstring[i + 1] == ':') continue;
		if (gpos < sizeof(group)) group[gpos++] = opt;
	}

	if (gpos > 0) fprintf(stderr, " [-%s]", group);

	for (int i = 0; __optstring[i] != '\0';	i++) {
		char opt = __optstring[i];
		if (opt == ':' || opt == '+' || opt == '-') continue;
		if (__optstring[i + 1] == ':') {
			if (__optstring[i + 2] == ':') fprintf(stderr, " [-%c [arg]]", opt);
			else fprintf(stderr, " [-%c <arg>]", opt);
		}
	}

	fprintf(stderr, " %s\n", ext);
}

static inline void version(void) {
	printf("%s (util.) %s\n", __prog_name, VERSION);
	printf("Written by ");
	size_t i = 0;
	while (__authors[i + 1] != NULL) printf("%s, ", __authors[i++]);
	printf("%s%s.\n", (i > 0) ? "& " : "", __authors[i]);
}

static inline void error(int stat) {
	perror(__prog_name);
	if (stat != 0) exit(stat);
}

#endif
