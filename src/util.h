#ifndef _UTIL_H
#define _UTIL_H

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef VERSION
#define VERSION ""
#error "VERSION is undefined"
#endif

#define GETOPT_HELP_CHAR 'h'
#define GETOPT_VERSION_CHAR 'v'
#define GETOPT_HELP	{ "help", no_argument, NULL, GETOPT_HELP_CHAR}
#define GETOPT_VERSION	{ "version", no_argument, NULL, GETOPT_VERSION_CHAR }

#define PROGRAM_NAME(_s) const char* __prog_name = (_s)
#define AUTHORS(...) const char* __authors[] = { __VA_ARGS__, NULL }

extern const char* __prog_path;
extern const char* __prog_name;
extern const char* __authors[];

#define USAGE_FAILURE	0
#define USAGE_HELP	1
extern void usage(int status);
static inline void version(void) {
	printf("%s (util.) %s\n", __prog_name, VERSION);
	printf("Written by ");
	size_t i = 0;
	while (__authors[i + 1] != NULL) printf("%s, ", __authors[i++]);
	printf("%s%s.\n", (i > 0) ? "& " : "", __authors[i]);
}

#endif
