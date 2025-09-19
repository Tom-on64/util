#include <stdbool.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

PROGRAM("echo");
AUTHORS("Tom-on");
OPTIONS("ehnv");

struct {
	bool escape;
	bool newline;
} flags = { 0 };

char escape(char c) {
	switch (c) {
	case 'a': return '\x07';	// Bell (alert)
	case 'b': return '\b';		// Backspace
	case 'c': exit(0);		// No further output
	case 'e': return '\x1b';	// Escape
	case 'f': return '\f';		// Form feed
	case 'n': return '\n';		// Newline
	case 'r': return '\r';		// Carridge return
	case 't': return '\t';		// Horizontal tab
	case 'v': return '\v';		// Vertical tab
	}

	putc('\\', stdout);
	return c;
}

int main(int argc, char** argv) {
	flags.newline = true;

	int opt;
	while ((opt = getopt(argc, argv, __optstring)) != -1) {
		switch (opt) {
		case 'e': flags.escape = true; break;
		case 'h': usage(); return 0;
		case 'n': flags.newline = false; break;
		case 'v': version(); return 0;
		}
	}
	
	for (int i = optind; i < argc; i++) {
		int j = 0, len = strlen(argv[i]);
		for (char c = argv[i][j]; j < len; c = argv[i][++j]) {
			if (c == '\\') c = escape(argv[i][++j]);
			putc(c, stdout);
		}
		if (i < argc - 1) putc(' ', stdout);
	}

	if (flags.newline) putc('\n', stdout);

	return 0;
}

