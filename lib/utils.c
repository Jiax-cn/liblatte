#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

void hexdump(void *buf, uint32_t size)
{
    uint8_t *out_buf = (uint8_t*)buf;
    uint32_t i = 0;

    for (i = 0; i < size; i++) {
        printf("%02x ", out_buf[i]);
        if (((i+1) % 16) == 0)
            printf("\n");
    }
    if (size % 16 != 0)
        printf("\n");
    printf("\n");
}

uint32_t get_file_size(const char *file_name)
{
    struct stat stat_buf = {};

    if (stat(file_name, &stat_buf) != 0) {
        printf("Failed to get file size: %s\n", strerror(errno));
        return 0;
    }

    return stat_buf.st_size;
}

uint32_t read_file_to_buf(const char *file_name, uint8_t *file_buf,
                          uint32_t buf_size)
{
    int file = 0;
    ssize_t read_size = 0;

    if ((file = open(file_name, O_RDONLY, 0)) == -1)
        goto fail;

    if ((read_size = read(file, file_buf, buf_size)) < 0)
        goto fail;

    return (uint32_t)read_size;

fail:
    printf("Failed to read %s: %s\n", file_name, strerror(errno));

    if (file > 0)
        close(file);

    return 0;
}

uint8_t *read_file(const char *file_name, uint32_t *ret_size)
{
    uint8_t *file_buf = NULL;
    uint32_t file_size = 0, read_size = 0;

    if ((file_size = get_file_size(file_name)) == 0)
        goto fail;

    if ((file_buf = (uint8_t *)malloc(file_size)) == NULL) {
        printf("Failed to read %s: alloc memory failed.\n", file_name);
        goto fail;
    }

    read_size = read_file_to_buf(file_name, file_buf, file_size);
    if (read_size != file_size) {
        printf("Failed to read %s: read size %u does not match file size %u.\n",
               file_name, read_size, file_size);
        goto fail;
    }

    *ret_size = read_size;
    return file_buf;

fail:
    if (file_buf)
        free(file_buf);

    return NULL;
}
