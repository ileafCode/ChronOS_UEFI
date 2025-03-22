#include <drivers/fb/fb.h>
#include <drivers/device.h>
#include <string/string.h>

gop_framebuffer_t *framebuffer_dev;

int fb_ioctl(int type, void *buffer) {
    int ret = 0;
    switch (type) {
        case FB_IOCTL_DATA: // Get data of the framebuffer (width, height, etc...)
            if (!buffer) {
                ret = 1;
                break;
            }
            fb_ioctl_data_t data;
            data.width = framebuffer_dev->width;
            data.height = framebuffer_dev->height;
            data.fb_size = framebuffer_dev->buf_size;
            memcpy(buffer, &data, sizeof(fb_ioctl_data_t));
            
            break;
    }
    return ret;
}

int fb_init(gop_framebuffer_t *fb) {
    framebuffer_dev = fb;
    int res = device_mkdev("fb", fb_ioctl, NULL, NULL);
    if (res)
        return 1;
    return 0;
}