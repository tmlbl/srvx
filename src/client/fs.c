#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "messages.h"

static const char SRVX_MSG_TYPE_REQ[] = "/req";

srvx_mq_client mqclient;

// Tells us whether a given path matches our route key schema.
// The root "/" is always valid
static int
srvx_is_valid_path(const char *path)
{
    if (strcmp("/", path) == 0) {
        return 1;
    } else if (strcmp(SRVX_MSG_TYPE_REQ, path) == 0) {
        return 1;
    }
    return 0;
}

static int
srvx_getattr(const char *path, struct stat *stbuf)
{
    printf("getattr called %s\n", path);
    memset(stbuf, 0, sizeof(struct stat));

    if (srvx_is_valid_path(path)) {
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
};

int
main(int argc, char **argv)
{
    printf("Starting the program\n");
    srvx_mq_client_connect(&mqclient);
    fuse_main(argc, argv, &srvx_filesystem_operations, NULL);
    srvx_mq_client_destroy(&mqclient);
}
