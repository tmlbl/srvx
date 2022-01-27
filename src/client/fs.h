#ifndef SRVX_FS_H
#define SRVX_FS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fuse/fuse_lowlevel.h>

#include "ht.h"

// Data structure to store and retrieve information about virtual files

#define SRVX_MAX_FILES 4096

#define ROOT_DIR_INO 1
#define PUB_DIR_INO 2
#define SUB_DIR_INO 3

#define SRVX_FILE_TYPE_PUB 1
#define SRVX_FILE_TYPE_SUB 2

typedef struct {
	fuse_ino_t ino;
	int type;
	int isdir;
	char *name;
} srvx_file_info;

typedef struct {
	fuse_ino_t last_ino;
	// Lookup table for finding file info by name
	ht *table;
	// Lookup array for finding file info by inode number
	srvx_file_info *nodes[SRVX_MAX_FILES];
} srvx_fs;

void srvx_fs_create(srvx_fs *fs, fuse_ino_t parent, srvx_file_info *info);

void srvx_fs_init(srvx_fs *fs);

srvx_file_info *srvx_fs_lookup(srvx_fs *fs, fuse_ino_t parent, const char *name);

void srvx_fs_stat(srvx_fs *fs, fuse_ino_t ino, struct stat *stbuf);

#endif // SRVX_FS_H
