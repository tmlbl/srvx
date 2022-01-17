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

// static int
// srvx_getattr(const char *path, struct stat *stbuf)
// {
// 	memset(stbuf, 0, sizeof(struct stat));

// 	if (srvx_is_dir_path(path)) {
// 		stbuf->st_mode = S_IFDIR | 0755;
// 		stbuf->st_nlink = 2;
// 	} else {
// 		stbuf->st_mode = S_IFREG | 0666;
// 		stbuf->st_nlink = 1;
// 		stbuf->st_size = peek_msg_size(&mqclient, path);

// 		// If the next message is 0, we tell the client the file is
// 		// really big so they keep trying to read it
// 		if (stbuf->st_size == 0)
// 			stbuf->st_size = 4096;
// 	}

// 	return 0;
// }

#define ROOT_DIR_INO 1
#define PUB_DIR_INO 2
#define SUB_DIR_INO 3

#define min(x, y) ((x) < (y) ? (x) : (y))

static int srvx_stat(fuse_ino_t ino, struct stat *stbuf)
{
	switch (ino) {
	case ROOT_DIR_INO:
	case PUB_DIR_INO:
	case SUB_DIR_INO:
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 1;
		stbuf->st_size = 0;
		return 0;
	default:
		return -1;
	}
}

static void srvx_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
	struct fuse_entry_param e;
	printf("Lookup %s\n", name);

	if (parent == PUB_DIR_INO) {
		e.ino = 0;
		e.attr_timeout = 1.0;
		e.entry_timeout = 1.0;
		fuse_reply_entry(req, &e);
		return;
	}

	// if (parent != 1 || strcmp(name, hello_name) != 0)
	// 	fuse_reply_err(req, ENOENT);
	// else {
		memset(&e, 0, sizeof(e));
		e.ino = 2;
		e.attr_timeout = 1.0;
		e.entry_timeout = 1.0;
		srvx_stat(e.ino, &e.attr);

		fuse_reply_entry(req, &e);
	// }
}


static void
srvx_getattr(fuse_req_t req, fuse_ino_t ino,
			     struct fuse_file_info *fi)
{
	struct stat stbuf;
	(void) fi;
	memset(&stbuf, 0, sizeof(struct stat));
	if (srvx_stat(ino, &stbuf) == -1)
		fuse_reply_err(req, ENOENT);
	else
		fuse_reply_attr(req, &stbuf, 1.0);
}

// static int
// srvx_access(const char *path, int perm)
// {
// 	return 0;
// }

struct dirbuf {
	char *p;
	size_t size;
};

static void dirbuf_add(fuse_req_t req, struct dirbuf *b, const char *name,
		       fuse_ino_t ino)
{
	struct stat stbuf;
	size_t oldsize = b->size;
	b->size += fuse_add_direntry(req, NULL, 0, name, NULL, 0);
	b->p = (char *) realloc(b->p, b->size);
	memset(&stbuf, 0, sizeof(stbuf));
	stbuf.st_ino = ino;
	fuse_add_direntry(req, b->p + oldsize, b->size - oldsize, name, &stbuf,
			  b->size);
}


static int reply_buf_limited(fuse_req_t req, const char *buf, size_t bufsize,
			     off_t off, size_t maxsize)
{
	if (off < bufsize)
		return fuse_reply_buf(req, buf + off,
				      min(bufsize - off, maxsize));
	else
		return fuse_reply_buf(req, NULL, 0);
}


// int
// srvx_readdir_root(void *buf, fuse_fill_dir_t filler)
// {
// 	filler(buf, ".", NULL, 0);
// 	filler(buf, "pub", NULL, 0);
// 	filler(buf, "sub", NULL, 0);
// 	return 0;
// }

static void
srvx_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
	off_t off, struct fuse_file_info *fi)
{
	(void) fi;
	struct dirbuf b;
	memset(&b, 0, sizeof(b));
	
	switch (ino) {
	case ROOT_DIR_INO:
		dirbuf_add(req, &b, ".", ROOT_DIR_INO);
		dirbuf_add(req, &b, "..", ROOT_DIR_INO);
		dirbuf_add(req, &b, "pub", PUB_DIR_INO);	
		dirbuf_add(req, &b, "sub", SUB_DIR_INO);	
		break;
	case PUB_DIR_INO:
		dirbuf_add(req, &b, ".", PUB_DIR_INO);
		dirbuf_add(req, &b, "..", ROOT_DIR_INO);
		break;
	case SUB_DIR_INO:
		dirbuf_add(req, &b, ".", SUB_DIR_INO);
		dirbuf_add(req, &b, "..", ROOT_DIR_INO);
		break;
	}
	reply_buf_limited(req, b.p, b.size, off, size);
	free(b.p);
}

// static int
// srvx_flush(const char *path, struct fuse_file_info *info)
// {
// 	return 0;
// }

// static int
// srvx_getxattr(const char *path, const char *one, char *two, size_t size)
// {
// 	return 0;
// }

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

static void
srvx_open(fuse_req_t req, fuse_ino_t ino,
			  struct fuse_file_info *fi)
{
	
}

static int
srvx_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
	return 0;
}

// static int
// srvx_utimens(const char *path, const struct timespec tv[2])
// {
// 	return 0;
// }

// static int
// srvx_truncate(const char *path, off_t offset)
// {
// 	return 0;
// }

// static int
// srvx_chown(const char *path, uid_t uid, gid_t gid)
// {
// 	return 0;
// }

static void
srvx_read(fuse_req_t req, fuse_ino_t ino, size_t size,
			  off_t off, struct fuse_file_info *fi)
{
	// // TODO: Handle reads and writes larger than the standard block size
	// // (4K on this machine)
	// if (offset > 0)
	// 	return 0;
	// char *msg;

	// switch (srvx_msg_type(path)) {
	// case SRVX_MSG_TYPE_PUB:
	// 	msg = srvx_mq_client_sub_read(&mqclient, path, size, offset);
	// 	break;
	// default:
	// 	return -1;
	// }

	// msg = srvx_chop_path(msg);
	// size_t len = strlen(msg);
	// memcpy(buf, msg, len);

	// return len;
}

static const struct fuse_lowlevel_ops srvx_filesystem_operations = {
	.lookup   = srvx_lookup,
	.getattr  = srvx_getattr,
	// .access   = srvx_access,
	.readdir  = srvx_readdir,
	// .flush    = srvx_flush,
	// .getxattr = srvx_getxattr,
	// .write    = srvx_write,
	.open     = srvx_open,
	// .create   = srvx_create,
	// .utimens  = srvx_utimens,
	// .truncate = srvx_truncate,
	// .chown    = srvx_chown,
	.read     = srvx_read,
};

int srvx_fs_main(int argc, char **argv)
{
	// int ret;

	// struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	// struct fuse_session *se;
	// struct fuse_session_ops *ops;

	// se = fuse_session_new(ops, &srvx_filesystem_operations);
	
	// if (se == NULL)
	// 	return printf("Failed to create the fuse session\n");

	// fuse_daemonize(true);

	// printf("Starting the session loop\n");
	// ret = fuse_session_loop(se);

	// printf("Cleaning up the session...\n");
	// // fuse_session_unmount(se);
	// // srvx_mq_client_connect(&mqclient);
	// // printf("Setting up...\n");
	// // int rc = fuse_main(argc, argv, &srvx_filesystem_operations, NULL);
	// // printf("Tearing down...\n");
	// // srvx_mq_client_destroy(&mqclient);
	// // return rc;

	// return ret;

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse_chan *ch;
	char *mountpoint;
	int err = -1;

	if (fuse_parse_cmdline(&args, &mountpoint, NULL, NULL) != -1 &&
	    (ch = fuse_mount(mountpoint, &args)) != NULL) {
		struct fuse_session *se;

		se = fuse_lowlevel_new(&args, &srvx_filesystem_operations,
				sizeof(srvx_filesystem_operations), NULL);

		if (se != NULL) {
			if (fuse_set_signal_handlers(se) != -1) {
				fuse_session_add_chan(se, ch);
				err = fuse_session_loop(se);
				fuse_remove_signal_handlers(se);
				fuse_session_remove_chan(ch);
			}
			fuse_session_destroy(se);
		}
		fuse_unmount(mountpoint, ch);
	}
	fuse_opt_free_args(&args);

	return err ? 1 : 0;

}
