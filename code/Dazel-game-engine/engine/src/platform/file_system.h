#pragma once

#include "defines.h"

typedef struct file_handle
{
    void *handle;
    bool is_valid;
} file_handle;

enum file_modes
{
    FILE_MODE_READ = 0x1,
    FILE_MODE_WRITE = 0x2
};

// #define CLOSE_IF_FAILED(func, handle)     \
//     if (!func) {                          \
//         KERROR("File operation failed."); \
//         filesystem_close(handle);         \
//         return false;                     \
//     }

EXP bool filesystem_exists(const char *path);

EXP bool filesystem_open(const char *path, file_modes mode, bool binary, file_handle *out_handle);

EXP void filesystem_close(file_handle *handle);

EXP bool filesystem_size(file_handle *handle, u64 *out_size);


EXP bool filesystem_read_line(file_handle *handle, u64 max_length, char **line_buf, u64 *out_line_length);


EXP bool filesystem_write_line(file_handle *handle, const char *text);


EXP bool filesystem_read(file_handle *handle, u64 data_size, void *out_data, u64 *out_bytes_read);


EXP bool filesystem_read_all_bytes(file_handle *handle, u8 *out_bytes, u64 *out_bytes_read);


EXP bool filesystem_read_all_text(file_handle *handle, char *out_text, u64 *out_bytes_read);



EXP bool filesystem_write(file_handle *handle, u64 data_size, const void *data, u64 *out_bytes_written);

