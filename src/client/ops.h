#ifndef SRVX_OPS_H
#define SRVX_OPS_H

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define FUSE_USE_VERSION 34
#include <fuse.h>
#include <fuse/fuse_lowlevel.h>

#include "fs.h"
#include "mq_client.h"
#include "zhelpers.h"
#include "path.h"

static int verbose_flag;

int srvx_fs_main(int argc, char **argv);

#endif // SRVX_OPS_H
