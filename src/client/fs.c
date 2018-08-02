#include "fs.h"

srvx_mq_client mqclient;

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
    switch (srvx_msg_type(path)) {
    case SRVX_MSG_TYPE_PUB:
        srvx_mq_client_publish(&mqclient, strdup(path), strdup(data));
        break;
    }
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
    char *msg;

    switch (srvx_msg_type(path)) {
    case SRVX_MSG_TYPE_PUB:
        msg = srvx_mq_client_subscribe(&mqclient, path);
        printf("Message received in read: %s\n", msg);
        break;
    default:
        return -1;
    }

    msg = srvx_chop_path(msg);
    size_t len;
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

int srvx_fs_main(int argc, char **argv)
{
    printf("Starting the filesystem in dir %s\n", argv[2]);
    srvx_mq_client_connect(&mqclient);
    int rc = fuse_main(argc, argv, &srvx_filesystem_operations, NULL);
    srvx_mq_client_destroy(&mqclient);
    return rc;
}
