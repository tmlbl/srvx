#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>

#include "fs.h"
#include "mq_client.h"
#include "path.h"
#include "zhelpers.h"

static char *rand_string(char *str, size_t size)
{
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRST";
	if (size) {
		--size;
		for (size_t n = 0; n < size; n++) {
			int key = rand() % (int) (sizeof charset - 1);
			str[n] = charset[key];
		}
		str[size] = '\0';
	}
	return str;
}

// Exit immediately on SIGNINT
// TODO: Unmount the filesystem cleanly, if possible
void int_handler(int sig)
{
	exit(0);
}

int main(int argc, char **argv)
{
	signal(SIGINT, int_handler);

	static struct option long_options[] = {
		{"verbose", no_argument, &verbose_flag, 1},
	};
	int option_index = 0;
	while (1) {
		int c = getopt_long(argc, argv, "",
			long_options, &option_index);
		if (c == -1)
			break;
	}

	// Resolve arguments to full paths so we can run in a different
	// directory context
	int srvx_flags_end = 0;
	int args_ix = 0;
	char *real_args[argc];
	for (int i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (!srvx_flags_end && arg[0] != '-')
			srvx_flags_end = 1;
		else if (!srvx_flags_end)
			continue;
		char actual_path[PATH_MAX + 1];
		realpath(arg, actual_path);
		// Check if full path is actually a file
		if (access(actual_path, F_OK) == -1) {
			// If it's not a file, we leave it as-is
			real_args[args_ix] = arg;
		} else {
			real_args[args_ix] = actual_path;
		}
		args_ix++;
	}

	// Random name for temp dir
	srand(time(NULL));
	char *rundir_base = "/tmp/srvxfs-%s";
	char rundir[strlen(rundir_base) + 8];
	char rundir_salt[10];
	rand_string(rundir_salt, 10);
	sprintf(rundir, rundir_base, rundir_salt);

	// Build a command string out of the args to be passed to popen
	int cmd_len = 0;
	for (int i = 0; i < args_ix; i++) {
		cmd_len++;
		cmd_len += strlen(real_args[i]);
	}
	char cmd_str[cmd_len+1];
	cmd_str[0] = 0;
	for (int i = 0; i < args_ix; i++) {
		if (i > 0) {
			strcat(cmd_str, " ");
		}
		strcat(cmd_str, real_args[i]);
	}
	char *cmd_template = "cd %s && %s";
	int cmd_template_len = strlen(cmd_template) + strlen(rundir);
	char full_cmd[cmd_template_len + cmd_len];
	sprintf(full_cmd, cmd_template, rundir, cmd_str);

	// Canned arguments to fuse_main
	char *fuse_args[3];
	// If we pass the same executable name to argument 0, fuse will give
	// errors if more than one instance of the program is active. However,
	// it doesn't seem to matter what we pass in here, as long as it's
	// different...
	fuse_args[0] = rundir_salt;
	fuse_args[1] = "-f";
	if (verbose_flag)
		fuse_args[1] = "-d";
	fuse_args[2] = strdup(rundir);

	// Create the run directory
	mkdir(rundir, 0755);

	int pid = fork();

	if (pid == 0) {
		// TODO: Is there a better way to know the filesystem is ready
		// than just waiting a second?
		// sleep(1);
		// if (verbose_flag)
		// 	printf("Running command %s\n", full_cmd);
		// system(full_cmd);
		printf("Doing nothing\n");
	} else {
		printf("Calling main for filesystem at %s\n", rundir);
		srvx_fs_main(3, fuse_args);
	}
}
