#include <fs.h>

typedef size_t (*ReadFn)(void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void *buf, size_t offset, size_t len);

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
extern size_t serial_write(const void *buf, size_t offset, size_t len);
extern size_t events_read(void *buf, size_t offset, size_t len);
extern size_t dispinfo_read(void *buf, size_t offset, size_t len);
extern size_t fb_write(const void *buf, size_t offset, size_t len);
static size_t file_read(void *buf, size_t offset, size_t len)
{
    return ramdisk_read(buf, offset, len);
}
static size_t file_write(const void *buf, size_t offset, size_t len)
{
    return ramdisk_write(buf, offset, len);
}

typedef struct {
    char *name;
    size_t size;
    size_t disk_offset;
    ReadFn read;
    WriteFn write;
    size_t open_offset;
} Finfo;

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB };

size_t invalid_read(void *buf, size_t offset, size_t len)
{
    panic("should not reach here");
    return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len)
{
    panic("should not reach here");
    return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
    [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
    {"/dev/events", 0, 0, events_read, invalid_write},
    {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
    {"/dev/fb", 0, 0, invalid_read, fb_write},
#include "files.h"
};

#define NR_FILE (int)sizeof(file_table) / sizeof(file_table[0])

int fs_open(const char *pathname, int flags, int mode)
{
    for (int i = 0; i < NR_FILE; i++) {
        if (strcmp(pathname, file_table[i].name) == 0) {
            file_table[i].open_offset = 0;
            return i;
        }
    }
    assert(0);
}

size_t fs_read(int fd, void *buf, size_t len)
{
    size_t offset = file_table[fd].disk_offset + file_table[fd].open_offset;
    size_t real_len = (file_table[fd].open_offset + len > file_table[fd].size)
                          ? (file_table[fd].size - file_table[fd].open_offset)
                          : len;
    file_table[fd].open_offset += real_len;
    assert(file_table[fd].open_offset <= file_table[fd].size);
    return file_table[fd].read(buf, offset, real_len);
}

size_t fs_write(int fd, const void *buf, size_t len)
{
    size_t offset = file_table[fd].disk_offset + file_table[fd].open_offset;
    size_t real_len = (file_table[fd].open_offset + len > file_table[fd].size)
                          ? (file_table[fd].size - file_table[fd].open_offset)
                          : len;
    file_table[fd].open_offset += real_len;
    assert(file_table[fd].open_offset <= file_table[fd].size);
    return file_table[fd].write(buf, offset, real_len);
}

int fs_close(int fd) { return 0; }

size_t fs_lseek(int fd, size_t offset, int whence)
{
    assert(fd < NR_FILE);
    switch (whence) {
    case SEEK_SET:
        file_table[fd].open_offset = offset;
        break;
    case SEEK_CUR:
        file_table[fd].open_offset += offset;
        break;
    case SEEK_END:
        file_table[fd].open_offset = file_table[fd].size + offset;
        break;
    default:
        return -1;
    }
    // assert(file_table[fd].open_offset < file_table[fd].size);
    return file_table[fd].open_offset;
}

void fb_init()
{
    AM_GPU_CONFIG_T gpu_config;
    ioe_read(AM_GPU_CONFIG, &gpu_config);
    int width = gpu_config.width, height = gpu_config.height;
    int fb_fd = fs_open("/dev/fb", 0, 0);
    file_table[fb_fd].size = width * height;
}

void init_fs()
{
    // TODO: initialize the size of /dev/fb
    for (int i = 0; i < NR_FILE; i++) {
        if (file_table[i].read == NULL)
            file_table[i].read = file_read;
        if (file_table[i].write == NULL)
            file_table[i].write = file_write;
        file_table[i].open_offset = 0;
    }
    fb_init();
}
