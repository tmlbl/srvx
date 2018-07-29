#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#define FUSE_USE_VERSION 26
#include <fuse.h>

static int
srvx_getattr(const char *path, struct stat *stbuf)
{
    printf("getattr called %s\n", path);
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
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
    return size;
}

static struct fuse_operations srvx_filesystem_operations = {
    .getattr  = srvx_getattr,
    .access   = srvx_access,
    .readdir  = srvx_readdir,
    .flush    = srvx_flush,
    .getxattr = srvx_getxattr,
    .write    = srvx_write,
};

int
main(int argc, char **argv)
{
    printf("Starting the program\n");
    return fuse_main(argc, argv, &srvx_filesystem_operations, NULL);
}
