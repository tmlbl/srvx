#ifndef SRVX_PATH_H
#define SRVX_PATH_H

#define SRVX_MSG_TYPE_INVALID -1
#define SRVX_MSG_TYPE_REQ 1
#define SRVX_MSG_TYPE_PUB 2

static const char SRVX_MSG_PREFIX_REQ[] = "/req";
static const char SRVX_MSG_PREFIX_PUB[] = "/pub";

static int srvx_is_dir_path(const char *path)
{
    if (strcmp(SRVX_MSG_PREFIX_REQ, path) == 0) {
        return 1;
    } else if (strcmp(SRVX_MSG_PREFIX_PUB, path) == 0) {
        return 1;
    }
    return 0;
}

static int srvx_msg_type(const char *path)
{
    if (strncmp(SRVX_MSG_PREFIX_REQ, path, 4) == 0) {
        return SRVX_MSG_TYPE_REQ;
    } else if (strncmp(SRVX_MSG_PREFIX_PUB, path, 4) == 0) {
        return SRVX_MSG_TYPE_PUB;
    }
    return SRVX_MSG_TYPE_INVALID;
}

#endif // SRVX_PATH_H
