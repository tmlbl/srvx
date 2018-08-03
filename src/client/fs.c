#include "fs.h"

srvx_mq_client mqclient;

#define SRVX_MAX_MSG_LEN 4095

// Functions to support stat-like information. It is necessary to enable
// 'peeking' at the length of the next message so that clients will stop reading
// after receiving the full message.

static size_t
peek_msg_size(srvx_mq_client *client, const char *path)
{
	switch (srvx_msg_type(path)) {
	case SRVX_MSG_TYPE_PUB:
		return srvx_mq_client_sub_peek_len(&mqclient, path);
	}
	return 0;
}

static int
srvx_getattr(const char *path, struct stat *stbuf)
{
	memset(stbuf, 0, sizeof(struct stat));

	if (srvx_is_dir_path(path)) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else {
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;
		stbuf->st_size = peek_msg_size(&mqclient, path);

		// If the next message is 0, we tell the client the file is
		// really big so they keep trying to read it
		if (stbuf->st_size == 0)
			stbuf->st_size = 4096;
	}

	return 0;
}

static int
srvx_access(const char *path, int perm)
{
	return 0;
}

static int
srvx_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
	off_t offset, struct fuse_file_info *fi)
{
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

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
	return 0;
}

static int
srvx_write(const char *path, const char *data, size_t size, off_t offset,
		struct fuse_file_info *info)
{
	char buf[size + 1];
	strcpy(buf, data);
	buf[size] = '\0';
	switch (srvx_msg_type(path)) {
	case SRVX_MSG_TYPE_PUB:
		srvx_mq_client_publish(&mqclient, strdup(path), buf);
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
	// TODO: Handle reads and writes larger than the standard block size
	// (4K on this machine)
	if (offset > 0)
		return 0;
	char *msg;

	switch (srvx_msg_type(path)) {
	case SRVX_MSG_TYPE_PUB:
		msg = srvx_mq_client_sub_read(&mqclient, path, size, offset);
		break;
	default:
		return -1;
	}

	msg = srvx_chop_path(msg);
	size_t len = strlen(msg);
	memcpy(buf, msg, len);

	return len;
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
	srvx_mq_client_connect(&mqclient);
	int rc = fuse_main(argc, argv, &srvx_filesystem_operations, NULL);
	srvx_mq_client_destroy(&mqclient);
	return rc;
}
