#include <fs/vfs.h>
#include <stddef.h>
#include <string/string.h>
#include <printk/printk.h>
#include <shmall_wrapper.h>

#define NULL (void*)(0)

/* --- INTERNAL --- */

vfs_file_t *vfs_int_open(vfs_fs_t *fs, char *path, uint8_t mode) {
    if (!fs || !fs->open)
        return NULL;
    vfs_file_t *file = (vfs_file_t *)fs->open(path, mode);
    file->fs = fs;
    return file;
}

int vfs_int_close(vfs_fs_t *fs, vfs_file_t *file) {
    if (!fs || !fs->close)
        return 1;
    return fs->close(file);
}

int vfs_int_read(vfs_fs_t *fs, vfs_file_t *file, uint32_t count, void *buffer) {
    if (!fs || !fs->read)
        return 0;
    return fs->read(file, count, buffer);
}

int vfs_int_write(vfs_fs_t *fs, vfs_file_t *file, uint32_t count, void *buffer) {
    if (!fs || !fs->write)
        return 0;
    return fs->write(file, count, buffer);
}

/* --- GLOBAL --- */

vfs_fs_t *filesystems[VFS_MAX_FS];
//int fs_count = 0;

int vfs_init() {
    for (int i = 0; i < VFS_MAX_FS; i++) {
        filesystems[i] = NULL;
    }
    return 0;
}

int vfs_mount(vfs_fs_t *fs) {
    //if (fs_count == VFS_MAX_FS) {
        //return 1;
    //}

    int i;
    for (i = 0; i < VFS_MAX_FS; i++) {
        if (filesystems[i] == NULL)
            break;
    }

    filesystems[i] = fs;
    return 0;
}

int vfs_umount(char *name) {
    for (int i = 0; i < VFS_MAX_FS; i++) {
        if (filesystems[i] == NULL)
            continue;
        if (!strcmp(name, filesystems[i]->name)) {
            filesystems[i] = NULL;
            return 0;
        }
    }
    return 1;
}

vfs_file_t *vfs_open(const char *path, uint8_t mode) {
    //printk("original path: %s\n", path);

    char *copiedPath = (char *)kmalloc(strlen(path));
    strcpy(copiedPath, path);

    char *colon = strchr(path, ':');

    if (!colon) {
        return NULL;
    }

    *colon = '\0';
    char *fs_name = path;
    char *path_to_file_in_fs = colon + 1;

    //printk("fs name: %s, path to file: %s\n", fs_name, path_to_file_in_fs);

    vfs_fs_t *fs = NULL;

    for (int i = 0; i < VFS_MAX_FS; i++) {
        if (filesystems[i] == NULL) {
            continue;
        }
        if (!strcmp(fs_name, filesystems[i]->name)) {
            fs = filesystems[i];
        }
    }

    if (fs == NULL) {
        return NULL;
    }

    return vfs_int_open(fs, path_to_file_in_fs, mode);
}

int vfs_close(vfs_file_t *file) {
    return vfs_int_close(file->fs, file);
}

int vfs_read(vfs_file_t *file, uint32_t count, void *buffer) {
    return vfs_int_read(file->fs, file, count, buffer);
}

int vfs_write(vfs_file_t *file, uint32_t count, void *buffer) {
    return vfs_int_write(file->fs, file, count, buffer);
}
