#ifndef _KERNEL_FAT_H
#define _KERNEL_FAT_H

#include <fs/fat/ff.h>
#include <fs/vfs.h>
#include <stdint.h>

int fat_mount(const char *name);

#endif