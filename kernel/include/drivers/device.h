#ifndef _KERNEL_DEVICE_H
#define _KERNEL_DEVICE_H

#include <stdint.h>

#define DEVICES_MAX_DEV 128

typedef int (*devioctl_type_t) (int, void *);
typedef int (*devread_type_t)  (int, void *);
typedef int (*devwrite_type_t) (int, void *);

typedef struct device {
    char name[16];
    int dev_id;
    devioctl_type_t devioctl;
    devread_type_t devread;
    devwrite_type_t devwrite;
} device_t;

int device_mkdev(char *name, devioctl_type_t ioctl, devread_type_t read, devwrite_type_t write);
device_t *devopen(char *name);
int devclose(device_t *device);

int devioctl(device_t *dev, int type, void *buffer);
int devread(device_t *dev, int type, void *buffer);
int devwrite(device_t *dev, int type, void *buffer);

#endif