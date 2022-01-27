#include "ops.h"

srvx_mq_client mqclient;
srvx_fs *fs;

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

#define min(x, y) ((x) < (y) ? (x) : (y))

static void srvx_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
	struct fuse_entry_param e;
	memset(&e, 0, sizeof(e));

	e.attr_timeout = 1.0;
	e.entry_timeout = 1.0;

	printf("Looking up %s\n", name);

	srvx_file_info *info = srvx_fs_lookup(fs, parent, name);
	if (info == NULL)
	{
		printf("Returning ENOENT for %s\n", name);
		fuse_reply_err(req, ENOENT);
		return;
	}
	srvx_fs_stat(fs, info->ino, &e.attr);
	e.ino = info->ino;

	fuse_reply_entry(req, &e);
	return;
}

static void
srvx_getattr(fuse_req_t req, fuse_ino_t ino,
			     struct fuse_file_info *fi)
{
	struct stat stbuf;
	(void) fi;
	memset(&stbuf, 0, sizeof(struct stat));
	srvx_fs_stat(fs, ino, &stbuf);
	if (fs->nodes[ino] == NULL)
		fuse_reply_err(req, ENOENT);
	else
		fuse_reply_attr(req, &stbuf, 1.0);
}

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

static void
srvx_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
	off_t off, struct fuse_file_info *fi)
{
	(void) fi;
	struct dirbuf b;
	memset(&b, 0, sizeof(b));

	printf("Reading dir with ino %ld\n", ino);
	
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

static void srvx_write(fuse_req_t req, fuse_ino_t ino, const char *buf,
		       size_t size, off_t off, struct fuse_file_info *fi)
{
	printf("Request to write %ld bytes to ino %ld\n", size, ino);
}

static void srvx_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
	srvx_file_info *info = fs->nodes[ino];
	if (info == NULL)
	{
		fuse_reply_err(req, ENOENT);
		return;
	}
	fi->direct_io = 1;
	fi->nonseekable = 1;
	fi->fh = open("/tmp/dooter", fi->flags);
	fuse_reply_open(req, fi);
}

static void srvx_create(fuse_req_t req, fuse_ino_t parent, const char *name,
			mode_t mode, struct fuse_file_info *fi)
{
	printf("Request to create file %s with parent %lu\n", name, parent);
	
	struct fuse_entry_param e;
	memset(&e, 0, sizeof(e));

	srvx_file_info *info = malloc(sizeof(srvx_file_info));
	memset(info, 0, sizeof(srvx_file_info));

	info->name = name;

	info->isdir = 0;
	if (parent == PUB_DIR_INO) {
		printf("Creating publish file %s\n", name);
		info->type = SRVX_FILE_TYPE_PUB;
	}
	if (parent == SUB_DIR_INO) {
		printf("Error! Can't create a file in /sub\n");
		fuse_reply_err(req, EACCES);
		return;
	}

	srvx_fs_create(fs, parent, info);

	fi->nonseekable = 1;
	fi->fh = open("/tmp/dooter", fi->flags);

	fuse_reply_create(req, &e, fi);
}

static void srvx_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr,
			 int to_set, struct fuse_file_info *fi)
{
	srvx_fs_stat(fs, ino, attr);
	fuse_reply_attr(req, attr, 5);
}

static const struct fuse_lowlevel_ops srvx_filesystem_operations = {
	.lookup   = srvx_lookup,
	.getattr  = srvx_getattr,
	.readdir  = srvx_readdir,
	.write    = srvx_write,
	.open     = srvx_open,
	.create   = srvx_create,
	.setattr  = srvx_setattr,
};

int srvx_fs_main(int argc, char **argv)
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse_chan *ch;
	char *mountpoint;
	int err = -1;

	fs = malloc(sizeof(srvx_fs));
	memset(fs, 0, sizeof(srvx_fs));
	srvx_fs_init(fs);

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
