#include <drivers/device.h>
#include <logging/logging.h>
#include <mm/pmm/pmm.h>
#include <string/string.h>

device_t *devices[DEVICES_MAX_DEV];

int device_mkdev(char *name, devioctl_type_t ioctl, devread_type_t read, devwrite_type_t write) {
    // find a free spot
    int i, freeSpotFound = 0;
    for (i = 0; i < DEVICES_MAX_DEV; i++) {
        if (devices[i]) {
            continue;
        }
        freeSpotFound = 1;
        break;
    }

    // if there is no free spot, then return
    if (!freeSpotFound) {
        return 1;
    }
    
    // make a new device
    device_t *newdev = (device_t *)pmm_getpage();
    memcpy(newdev->name, name, 16);
    newdev->devioctl = ioctl;
    newdev->devread = read;
    newdev->devwrite = write;

    // add device to list
    devices[i] = newdev;
    return 0;
}

device_t *devopen(char *name) {
    for (int i = 0; i < DEVICES_MAX_DEV; i++) {
        // if device is null
        if (devices[i] == NULL)
            continue;
        
        // if name is equal, return device
        if (!strcmp(devices[i]->name, name)) {
            return devices[i];
        }
    }

    // no device with the same name found, return null
    return NULL;
}

int devclose(device_t *device) {
    // i don't really have to explain this
    if (device == NULL)
        return 1;
    device = NULL;
    return 0;
}

int devioctl(device_t *dev, int type, void *buffer) {
    if (dev == NULL)
        return 1;
    if (dev->devioctl == NULL)
        return 1;
    dev->devioctl(type, buffer);
}

int devread(device_t *dev, int type, void *buffer) {
    if (dev == NULL)
        return 1;
    if (dev->devread == NULL)
        return 1;
    dev->devread(type, buffer);
}

int devwrite(device_t *dev, int type, void *buffer) {
    if (dev == NULL)
        return 1;
    if (dev->devwrite == NULL)
        return 1;
    dev->devwrite(type, buffer);
}