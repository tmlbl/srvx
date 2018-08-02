#ifndef SRVX_FS_H
#define SRVX_FS_H

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "mq_client.h"
#include "zhelpers.h"
#include "path.h"

int srvx_fs_main(int argc, char **argv);

#endif // SRVX_FS_H
