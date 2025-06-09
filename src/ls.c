#include <stdnoreturn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>

#define MAX_ENTRIES 1024

struct {
	bool list_all;
	bool list_long;
	bool list_subd;
	bool list_dirs;
	bool append_type;
	bool sort_reverse;
	bool sort_date;
	bool human_read;
} flags = { 0 };

char* exename;

void print_help(void) {
	printf(
		"Usage: %1$s [OPTION]... [FILE]...\n"
		"List information about FILEs. Uses current directory by default.\n"
		"\n"
		"  -a	Show all files.\n"
		"  -l	Use a long listing format.\n"
		"  -R	List subdirectories recursively.\n"
		"  -d	List directories instead of their contents.\n"
		"  -F	Append a character indicating the file type.\n"
		"  -r	Reverse the order of sorting.\n"
		"  -t	Sort by modification date.\n"
		"  -h	Print this help message and exit.\n"
		"\n"
		"Examples:\n"
		"  %1$s d	List contents of d.\n"
		"  %1$s -a d	Print all contents of d, including hidden files.\n"
		"  %1$s -lh d	Output a long list of d's contents with human readable file sizes.\n"
		"  %1$s -RF d	List contents of d and it's subdirectories with characters like '/' for directories.\n"
		, exename
	      );
}

void set_flag(int f) {
	switch (f) {
	case 'a': flags.list_all = true; break;
	case 'l': flags.list_long = true; break;
	case 'R': flags.list_subd = true; break;
	case 'd': flags.list_dirs = true; break;
	case 'F': flags.append_type = true; break;
	case 'r': flags.sort_reverse = true; break;
	case 't': flags.sort_date = true; break;
	case 'h': print_help(); exit(0);
	default: fprintf(stderr, "%1$s: Invalid option -%2$c. Try '%1$s -h' for help.\n", exename, f); exit(1);
	}
}

noreturn
void crash(int err) {
	perror(exename);
	exit(err);
}

int namecmp(const void* a, const void* b) {
	const char** sa = (const char**)a;
	const char** sb = (const char**)b;

	if (flags.sort_reverse) return -strcmp(*sa, *sb);
	else return strcmp(*sa, *sb);
}

void print_mode(mode_t mode) {
	char str[11];
	str[0] = S_ISDIR(mode) ? 'd' : '-';
	str[1] = (mode & S_IRUSR) ? 'r' : '-';
	str[2] = (mode & S_IWUSR) ? 'w' : '-';
	str[3] = (mode & S_IXUSR) ? 'x' : '-';
	str[4] = (mode & S_IRGRP) ? 'r' : '-';
	str[5] = (mode & S_IWGRP) ? 'w' : '-';
	str[6] = (mode & S_IXGRP) ? 'x' : '-';
	str[7] = (mode & S_IROTH) ? 'r' : '-';
	str[8] = (mode & S_IWOTH) ? 'w' : '-';
	str[9] = (mode & S_IXOTH) ? 'x' : '-';
	str[10] = '\0';
	printf("%s", str);
}

int print_long(char* path, size_t* total) {
	struct stat st;
	if (stat(path, &st) == -1) return 1;
	print_mode(st.st_mode);
	printf(" %lu", st.st_nlink);
	struct passwd* pw = getpwuid(st.st_uid);
	struct group* gr = getgrgid(st.st_gid);
	printf(" %s %s", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");

	// TODO: Human readable
	printf(" %5ld", st.st_size);

	char timebuf[512];
	strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));
	printf(" %s ", timebuf);
	
	*total += st.st_blocks;

	return 0;
}

int main(int argc, char** argv) {
	exename = argv[0];

	size_t dir_count = 0;
	char** dirs = malloc(argc - 1);
	if (!dirs) crash(1);

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			dirs[dir_count++] = argv[i];
			continue;
		}

		int j = 1;
		while (argv[i][j] != '\0') set_flag(argv[i][j++]);
	}
	if (dir_count == 0) dirs[dir_count++] = ".";

	for (size_t i = 0; i < dir_count; i++) {
		if (dir_count > 1) printf("%s:\n", dirs[i]);

		char* entries[MAX_ENTRIES];
		size_t count = 0;
		DIR* dir = opendir(dirs[i]);
		if (!dir) crash(1);
		struct dirent* dent;

		while ((dent = readdir(dir)) != NULL) {
			if (!flags.list_all && dent->d_name[0] == '.') continue;
			entries[count] = strdup(dent->d_name);
			if (!entries[count]) crash(1);
			count++;

			if (count >= MAX_ENTRIES) {
				fprintf(stderr, "%s: Directory contains too many entries. Stopping at 1024th.\n", exename);
				break;
			}
		}
		closedir(dir);

		qsort(entries, count, sizeof(char*), namecmp);

		size_t total = 0;
		if (flags.list_long) printf("total:\n"); // Dummy total
		for (size_t j = 0; j < count; j++) {
			char fullpath[1024];
			snprintf(fullpath, 1024, "%s/%s", dirs[i], entries[j]);
			if (flags.list_long && print_long(fullpath, &total) != 0) {
				perror(exename);
				free(entries[j]);
				continue;
			}
			printf("%s%s", entries[j], (flags.list_long) ? "\n" : " ");
			free(entries[j]);
		}
		if (flags.list_long) printf("\x1b[%ldAtotal: %ld\x1b[%ldB\n", count + 1, total, count);

		if (dir_count > 1) printf("\n");
	}
	if (!flags.list_long) printf("\n");

	free(dirs);
	return 0;
}

