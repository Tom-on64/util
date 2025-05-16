#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Flags:
 * -A	Same as -vET
 * -b	Number non-empty lines
 * -e	Same as -vT
 * -n	Number all lines
 * -s	Squeeze multiple adjacent blank lines into one
 * -E	Display '$' at each newline
 * -T	Show '\t' as '^I'
 * -v	Show non-printing characters (except for '\t' & '\n')
 */
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
		"Usage: %s [OPTION]... [FILE]...\n"
		"Concatenate FILEs to standard output.\n"
		"\n"
		"With no FILE or when FILE is '-', read standard input.\n"
		"\n"
		"  -A	Equivalent to -vET.\n"
		"  -b	Number non-empty lines.\n"
		"  -e	Equivalent to -vT.\n"
		"  -n	Number all lines.\n"
		"  -s	Squeeze multiple adjacent blank lines into one.\n"
		"  -E	Display '$' at each newline.\n"
		"  -T	Show TAB as '^I'.\n"
		"  -v	Show non-printing characters (except for TAB and NEWLINE).\n"
		"\n"
		"Examples:\n"
		"  %s f		Print contents of f.\n"
		"  %s f - g	Output f's contents, then standard input, then g's contents.\n"
		"  %s		Copy standard input to standard output.\n"
		"  %s f g > h	Concatenate f's and g's contents into h.\n"
		, exename, exename, exename, exename, exename);
}

void error(char* fmt, ...) {
	va_list va;
	va_start(va, fmt);
	
	fprintf(stderr, "%s: ", exename);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");

	va_end(va);
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
		error("Invalid option -%c. Try '%s -h' for help.", f, exename);
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
			fprintf(stdout, "%6d  ", linenr++);
		}
		newline = false;
	}

	if (c == '\n') {
		if (flags.show_newline) fputc('$', stdout);
		fputc('\n', stdout);
		newline = true;
	} else if (c == '\t' && flags.show_tab) {
		fprintf(stdout, "^I");
	} else if (c < 0x20 && flags.show_non_print) {
		fprintf(stdout, "^%c", c + 0x40);
	} else if (c == 0x7f && flags.show_non_print) {
		fprintf(stdout, "^?");
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
			error("%s: No such file or directory", argv[i - 1]);
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

