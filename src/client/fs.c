#include "fs.h"

void srvx_fs_create(srvx_fs *fs, fuse_ino_t parent, srvx_file_info *info)
{
	char *key = malloc(strlen(info->name)+7);
	memset(key, '\0', strlen(info->name)+7);
	sprintf(key, "%07lu%s", parent, info->name);

	printf("Creating file with key %s\n", key);

	fs->last_ino++;
	info->ino = fs->last_ino;

	// Insert into lookup table
	ht_set(fs->table, key, info);

	// Insert into lookup array
	fs->nodes[info->ino] = info;
}

void srvx_fs_init(srvx_fs *fs)
{
	fs->table = ht_create();
	fs->last_ino = 0;

	srvx_file_info *root_info = malloc(sizeof(srvx_file_info));
	root_info->type = 0;
	root_info->isdir = 1;
	root_info->name = "/";
	srvx_fs_create(fs, 0, root_info);

	srvx_file_info *pub_info = malloc(sizeof(srvx_file_info));
	pub_info->type = SRVX_FILE_TYPE_PUB;
	pub_info->isdir = 1;
	pub_info->name = "pub";
	srvx_fs_create(fs, ROOT_DIR_INO, pub_info);

	srvx_file_info *sub_info = malloc(sizeof(srvx_file_info));
	sub_info->type = SRVX_FILE_TYPE_SUB;
	sub_info->isdir = 1;
	sub_info->name = "sub";
	srvx_fs_create(fs, ROOT_DIR_INO, sub_info);
}

srvx_file_info *srvx_fs_lookup(srvx_fs *fs, fuse_ino_t parent, const char *name)
{
	char key[strlen(name)+7];
	memset(key, '\0', strlen(name)+7);
	sprintf(key, "%07lu%s", parent, name);
	printf("Lookup with key %s\n", key);
	return ht_get(fs->table, key);
}

void srvx_fs_stat(srvx_fs *fs, fuse_ino_t ino, struct stat *stbuf)
{
	srvx_file_info *info = fs->nodes[ino];
	if (info == NULL)
	{
		// File does not exist
		printf("Cannot stat inode %lu, does not exist\n", ino);
		stbuf->st_nlink = 0;
		return;
	}

	printf("Found info for %s for inode %lu\n", info->name, ino);

	if (info->isdir)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		stbuf->st_size = 0;
		return;
	}

	stbuf->st_mode = S_IFREG | 0666;
	stbuf->st_nlink = 1;
	stbuf->st_size = 10;
	return;
}
