#include <fs/fat/fat_wrapper.h>
#include <shmall_wrapper.h>
#include <string/string.h>
#include <logging/logging.h>

vfs_file_t *fat_open(char *path, uint8_t mode) {
    FIL *fp = (FIL *)kmalloc(sizeof(FIL));
    FRESULT res = f_open(fp, path, mode);
    
    if (res)
        return NULL;
    
    vfs_file_t *file = (vfs_file_t *)kmalloc(sizeof(file));
    file->fs_type = VFS_FS_FAT;
    file->additional_info = (void *)fp;

    return file;
}

int fat_close(vfs_file_t *file) {
    FRESULT res = f_close((FIL *)file->additional_info);
    if (res)
        return 1;
    kfree(file->additional_info);
    kfree(file);
    return 0;
}

int fat_read(vfs_file_t *file, uint32_t count, uint8_t *buffer) {
    UINT bytes_read = 0;
    FRESULT res = f_read((FIL *)file->additional_info, buffer, count, &bytes_read);
    if (res)
        return 0;
    f_lseek((FIL *)file->additional_info, 0);
    return bytes_read;
}

int fat_write(vfs_file_t *file, uint32_t count, uint8_t *buffer) {
    UINT bytes_written = 0;
    FRESULT res = f_write((FIL *)file->additional_info, buffer, count, &bytes_written);
    if (res)
        return 0;
    f_lseek((FIL *)file->additional_info, 0);
    return bytes_written;
}

int fat_mount(const char *name) {
    vfs_fs_t *fatvfs = (vfs_fs_t *)kmalloc(sizeof(vfs_fs_t));
    fatvfs->name = (char *)kmalloc(strlen(name));
    strcpy(fatvfs->name, name);

    fatvfs->open = fat_open;
    fatvfs->close = fat_close;
    fatvfs->read = fat_read;
    fatvfs->write = fat_write;

    fatvfs->additional_info = (void *)kmalloc(sizeof(FATFS));
    FRESULT res = f_mount((FATFS *)fatvfs->additional_info, "", 0);
    if (res) {
        log_error("FAT", "No FAT volume found.");
        kfree(fatvfs);
        return 1;
    }
    
    if (vfs_mount(fatvfs)) {
        return 1;
    }

    return 0;
}