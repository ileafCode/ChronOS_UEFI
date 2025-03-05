#ifndef _KERNEL_VFS_H
#define _KERNEL_VFS_H

#include <stdint.h>

#define VFS_MAX_FS 10

#define VFS_FS_FAT 0
#define VFS_FS_DEV 1

struct vfs_file;

typedef struct vfs_file *(*open_type_t)  (char *, uint8_t);
typedef int              (*close_type_t) (struct vfs_file *);
typedef int              (*read_type_t)  (struct vfs_file *, uint32_t, uint8_t *);
typedef int              (*write_type_t) (struct vfs_file *, uint32_t, uint8_t *);

typedef struct vfs_fs {
    char *name;
    int fs_type;
    void *additional_info;
    open_type_t open;
    close_type_t close;
    read_type_t read;
    write_type_t write;
} vfs_fs_t;

typedef struct vfs_file {
    int fs_type;
    vfs_fs_t *fs;
    void *additional_info;
} vfs_file_t;

int vfs_init();

/// @brief Mounts a filesystem
/// @param fs The filesystem
/// @return 0 if OK, otherwise an error occurred
int vfs_mount(vfs_fs_t *fs);

/// @brief Unmounts a filesystem
/// @param name The name of the filesystem
/// @return 0 if OK, otherwise an error occurred
int vfs_umount(char *name);


/// @brief Opens a file
/// @param path Path to the file
/// @param mode How to open the file
/// @return The file
vfs_file_t *vfs_open(const char *path, uint8_t mode);

/// @brief Closes a file
/// @param file The file
/// @return 0 if OK, otherwise an error occurred
int vfs_close(vfs_file_t *file);

/// @brief Reads from a file
/// @param file The file
/// @param count How many bytes to read
/// @param buffer Destination
/// @return Bytes read
int vfs_read(vfs_file_t *file, uint32_t count, void *buffer);

/// @brief Writes to a file
/// @param file The file
/// @param count How many bytes to write
/// @param buffer Source
/// @return Bytes written
int vfs_write(vfs_file_t *file, uint32_t count, void *buffer);

#endif