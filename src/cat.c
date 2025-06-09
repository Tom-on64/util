#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct {
	bool show_all;
	bool number_lines;
	bool number_empty;
	bool squeeze_lines;
	bool show_newline;
	bool show_tab;
	bool show_non_print;
} flags = { 0 };

char* exename;

void print_help(void) {
	printf(
		"Usage: %1$s [OPTION]... [FILE]...\n"
		"Concatenate FILEs to standard output.\n"
		"\n"
		"With no FILE or when FILE is '-', read standard input.\n"
		"\n"
		"  -A	Equivalent to -vET.\n"
		"  -b	Number non-empty lines.\n"
		"  -e	Equivalent to -vT.\n"
		"  -h	Print this help message and exit.\n"
		"  -n	Number all lines.\n"
		"  -s	Squeeze multiple adjacent blank lines into one.\n"
		"  -E	Display '$' at each newline.\n"
		"  -T	Show TAB as '^I'.\n"
		"  -v	Show non-printing characters (except for TAB and NEWLINE).\n"
		"\n"
		"Examples:\n"
		"  %1$s f		Print contents of f.\n"
		"  %1$s f - g	Output f's contents, then standard input, then g's contents.\n"
		"  %1$s		Copy standard input to standard output.\n"
		"  %1$s f g > h	Concatenate f's and g's contents into h.\n"
		, exename);
}

void set_flag(char f) {
	switch (f) {
	case 'A':
		flags.show_newline = true;
		flags.show_tab = true;
		flags.show_non_print = true;
		break;
	case 'b':
		flags.number_empty = true;
		break;
	case 'e':
		flags.show_tab = true;
		flags.show_non_print = true;
		break;
	case 'h':
		print_help();
		exit(0);
	case 'n':
		flags.number_lines = true;
		break;
	case 's':
		flags.squeeze_lines = true;
		break;
	case 'E':
		flags.show_newline = true;
		break;
	case 'T':
		flags.show_tab = true;
		break;
	case 'v':
		flags.show_non_print = true;
		break;
	default:
		fprintf(stderr, "%s: Invalid option -%c. Try '%s -h' for help.", exename, f, exename);
		exit(1);
		break;
	}
}

void proc_char(int c) {
	static int squeeze = 0;
	static int linenr = 1;
	static bool newline = true;
	
	if (c == '\n' && flags.squeeze_lines && squeeze == 0) squeeze = 1;
	else if (c == '\n' && squeeze == 1) squeeze = 2;
	else if (c == '\n' && squeeze == 2) return;
	else squeeze = 0;

	if (newline) {
		if ((flags.number_lines && !flags.number_empty) || (flags.number_empty && c != '\n')) {
			printf("%6d  ", linenr++);
		}
		newline = false;
	}

	if (c == '\n') {
		if (flags.show_newline) fputc('$', stdout);
		fputc('\n', stdout);
		newline = true;
	} else if (c == '\t' && flags.show_tab) {
		printf("^I");
	} else if (c < 0x20 && flags.show_non_print) {
		printf("^%c", c + 0x40);
	} else if (c == 0x7f && flags.show_non_print) {
		printf("^?");
	} else {
		fputc(c, stdout);
	}
}

static char* stdin_argv[] = { NULL, "/dev/stdin" };
int main(int argc, char** argv) {
	exename = argv[0];

	int i;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-' || argv[i][1] == '\0') break;
		int j = 1;
		while (argv[i][j] != '\0') set_flag(argv[i][j++]);
	}

	FILE* fp;
	if (i >= argc) {
		argv = stdin_argv;
		argv[0] = exename;
		argc = 2;
		i = 1;
	}

	while (i < argc) {
		if (strcmp("-", argv[i]) == 0) argv[i] = "/dev/stdin";
		fp = fopen(argv[i++], "r");
		if (fp == NULL) {
			perror(exename);
			continue;
		}

		int c;
		while (true) {
			c = fgetc(fp);
			if (c == EOF) break;
			proc_char(c);
		}
	}

	fclose(fp);
	exit(0);
}

