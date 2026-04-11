#ifndef FS_H
#define FS_H

#include <stdint.h>

#define SLFS_MAX_FILES     64
#define SLFS_MAX_FILENAME  32
#define SLFS_MAX_FILESIZE  8192
#define SLFS_MAGIC         0x534C4653  // "SLFS"

#define SLFS_TYPE_EMPTY    0x00
#define SLFS_TYPE_FILE     0x01
#define SLFS_TYPE_DIR      0x02

#define SLFS_OK             0
#define SLFS_ERR_FULL      -1
#define SLFS_ERR_EXISTS    -2
#define SLFS_ERR_NOTFOUND  -3
#define SLFS_ERR_TOOBIG    -4

typedef struct {
    char     name[SLFS_MAX_FILENAME];
    uint32_t size;
    uint32_t type;
    uint32_t created;
    uint32_t modified;
    uint8_t  data[SLFS_MAX_FILESIZE];
} slfs_file_t;

typedef struct {
    uint32_t    magic;
    uint32_t    version;
    uint32_t    total_files;
    uint32_t    used_files;
    slfs_file_t files[SLFS_MAX_FILES];
} slfs_t;

void     fs_init(void);
int      fs_create(const char *name);
int      fs_write(const char *name, const char *data, uint32_t len);
int      fs_read(const char *name, char *buf, uint32_t buflen);
int      fs_delete(const char *name);
int      fs_exists(const char *name);
int      fs_list(char out[][SLFS_MAX_FILENAME], int *count);
uint32_t fs_used(void);
uint32_t fs_total(void);

#endif