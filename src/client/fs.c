#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "mq_client.h"

static const char SRVX_MSG_TYPE_REQ[] = "/req";
static const char SRVX_MSG_TYPE_PUB[] = "/pub";
static const char SRVX_MSG_TYPE_SUB[] = "/sub";

srvx_mq_client mqclient;

static int srvx_is_dir_path(const char *path)
{
    if (strcmp(SRVX_MSG_TYPE_REQ, path) == 0) {
        return 1;
    } else if (strcmp(SRVX_MSG_TYPE_PUB, path) == 0) {
        return 1;
    } else if (strcmp(SRVX_MSG_TYPE_SUB, path) == 0) {
        return 1;
    }
    return 0;
}

static int
srvx_getattr(const char *path, struct stat *stbuf)
{
    printf("getattr called %s\n", path);
    memset(stbuf, 0, sizeof(struct stat));

    if (srvx_is_dir_path(path)) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else {
        stbuf->st_mode = S_IFREG | 0666;
        stbuf->st_nlink = 1;
        stbuf->st_size = 20;
    }

    return 0;
}

static int
srvx_access(const char *path, int perm)
{
    printf("Permitting to access %s with perm %d\n", path, perm);
    return 0;
}

static int
srvx_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
        off_t offset, struct fuse_file_info *fi)
{
    printf("Readdir called %s\n", path);
    filler(buf, ".", NULL, 0);           /* Current directory (.)  */
    filler(buf, "..", NULL, 0);          /* Parent directory (..)  */

    return 0;
}

static int
srvx_flush(const char *path, struct fuse_file_info *info)
{
    return 0;
}

static int
srvx_getxattr(const char *path, const char *one, char *two, size_t size)
{
    printf("Getxaddr string params: %s %s %s\n", path, one, two);
    return 0;
}

static int
srvx_write(const char *path, const char *data, size_t size, off_t offset,
		struct fuse_file_info *info)
{
    printf("Writing to path %s data %s\n", path, data);
    char *msg = strdup(data);
    srvx_mq_client_send(&mqclient, msg);
    return size;
}

static int
srvx_open(const char *path, struct fuse_file_info *info)
{
    return 0;
}

static int
srvx_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
    return 0;
}

static int
srvx_utimens(const char *path, const struct timespec tv[2])
{
    return 0;
}

static int
srvx_truncate(const char *path, off_t offset)
{
    return 0;
}

static int
srvx_chown(const char *path, uid_t uid, gid_t gid)
{
    return 0;
}

static int
srvx_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *info)
{
    size_t len;
    char *msg = "hello";
	len = strlen(msg);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, msg + offset, size);
	} else
		size = 0;

	return size;
}

static struct fuse_operations srvx_filesystem_operations = {
    .getattr  = srvx_getattr,
    .access   = srvx_access,
    .readdir  = srvx_readdir,
    .flush    = srvx_flush,
    .getxattr = srvx_getxattr,
    .write    = srvx_write,
    .open     = srvx_open,
    .create   = srvx_create,
    .utimens  = srvx_utimens,
    .truncate = srvx_truncate,
    .chown    = srvx_chown,
    .read     = srvx_read,
};

int
main(int argc, char **argv)
{
    // Resolve arguments to full paths so we can run in a different directory
    // context
    char *real_args[argc];
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        char actual_path[PATH_MAX + 1];
        realpath(arg, actual_path);
        // Check if full path is actually a file, if not, try to resolve within
        // executable PATH
        if (access(actual_path, F_OK) == -1) {
            // Use the output of the which command to find it
            char cmd[256];
            sprintf(cmd, "which %s", arg);
            FILE *which = popen(cmd, "r");
            char which_out[PATH_MAX];
            fgets(which_out, PATH_MAX, which);
            // Trim newline
            int out_len = strlen(which_out);
            for (int j = 0; j < out_len; j++) {
                if (which_out[j] == '\n') {
                    which_out[j] = '\0';
                }
            }
            real_args[i - 1] = which_out;
        } else {
            real_args[i - 1] = actual_path;
        }
    }

    // TODO: Random name for temp dir
    char *rundir = "/tmp/srvxfs";

    // Build a command string out of the args to be passed to popen
    int cmd_len = 0;
    for (int i = 0; i < argc - 1; i++) {
        cmd_len++;
        cmd_len += strlen(real_args[i]);
    }
    char cmd_str[cmd_len+1];
    cmd_str[0] = 0;
    for (int i = 0; i < argc - 1; i++) {
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
    fuse_args[0] = "srvx";
    fuse_args[1] = "-f";
    fuse_args[2] = strdup(rundir);

    // Create the run directory
    mkdir(rundir, 0755);

    int pid = fork();

    if (pid == 0) {
        // TODO: Is there a better way to know the filesystem is ready than just
        // waiting a second?
        sleep(1);
        printf("Running command %s\n", full_cmd);
        system(full_cmd);
    } else {
        srvx_mq_client_connect(&mqclient);
        fuse_main(3, fuse_args, &srvx_filesystem_operations, NULL);
        srvx_mq_client_destroy(&mqclient);
    }
}
