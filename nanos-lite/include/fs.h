#ifndef __FS_H__
#define __FS_H__

#include "common.h"

enum {SEEK_SET, SEEK_CUR, SEEK_END};

int fs_open(const char* pathname, int flags, int mode);

#endif
