#include <string.h>

#include "fat_filelib.h"
#include "getopt.h"

#include "cli.h"

enum {
	RM_UNKNOWN,
	RM_FILE,
	RM_DIR
};

static struct option const long_options[] = {
	{ "recursive", no_argument, 0, 'r' },
	{ "verbose",   no_argument, 0, 'v' },
	{  0,          0,           0,  0  }
};

int
rm_recursive(CLI *cli, const char *path, int verbose) {
	FL_DIR dirstat, *dirp = fl_opendir(path, &dirstat);
	char buf[PATH_MAX];
	int r = -1;

	if (dirp) {
		struct fs_dir_ent dirent;

		r = 0;

		while (!r && (fl_readdir(&dirstat, &dirent) == 0)) {
			int r2 = -1;

			/* Skip the names "." and ".." as we don't want to recurse on them. */
			if (!strcmp(dirent.filename, ".") || !strcmp(dirent.filename, "..")) {
				continue;
			}

			sprintf(buf, "%s/%s", path, dirent.filename);

			if (fl_is_dir(buf)) {
				r2 = rm_recursive(cli, buf, verbose);
			} else {
				r2 = fl_remove(buf);

				if (verbose) {
					printf("rm: removing '%s'\n", buf);
				}
			}

			r = r2;
		}

		fl_closedir(dirp);
	}

	if (!r) {
		r = fl_remove(path);

		if (verbose) {
			printf("rm: removing '%s'\n", path);
		}
	}

	return (r);
}

static int
rm_single(CLI *cli, const char *path, int verbose) {
	if (fl_remove(path)) {
		printf( "rm: could not remove file '%s'\n", path);

		return (1);
	}

	if (verbose) {
		printf("rm: removing '%s'\n", path);
	}

	return (0);
}

static int
rm_scope(const char *path) {
	if (fl_is_dir(path)) {
		return (RM_DIR);
	}

	return (RM_FILE);
}

void
cmd_rm_help(CLI *cli) {
	printf("'rm' removes files and directories.                           \n");
	printf("                                                              \n");
	printf("Usage:  rm [options] <path>                                   \n");
	printf("Options:                                                      \n");
	printf("  -r, --recursive  Recursively remove sub directories         \n");
	printf("  -v, --verbose    Be extremely noisy about what is happening \n");
}

int
cmd_rm_exec(CLI *cli, char *argv[]) {
	int argc = cli_num_args(argv);
	int recursive = 0, verbose = 0;
	char path[PATH_MAX];
	int scope, ret = 0;
	int i, c, opt_ind;

	for (c = 0, optind = 0, opt_ind = 0; c != -1;) {
		c = getopt_long(argc, argv, "rv", long_options, &opt_ind);
		switch (c) {
			case 'r': recursive = 1; break;
			case 'v': verbose   = 1; break;
		}
	}

	if (argc - optind < 2) {
		printf("rm: insufficient number of arguments. Try 'man rm'\n");

		return (-1);
	}

	for (i=optind+1; argv[i]; i++) {
		cli_clean_path(cli, argv[i], path);

		scope = rm_scope(path);

		switch (scope) {
			case RM_FILE:
				ret += rm_single(cli, path, verbose);
			break;

			case RM_DIR:
				if (!recursive) {
					printf("%s is a directory, use -r to remove it.\n", path);
					ret++;
				} else {
					ret += rm_recursive(cli, path, verbose);
				}
			break;
		}
	}

	if (ret) return (-1);

	return (0);
}
