/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_FILEIO_H
#define KAI_FILEIO_H

#include "alloc.h"
#include "utils.h"

namespace kai {
    typedef void * FileHandle;

    enum FileFlags {
        FILE_NONE = 0,
        FILE_READ = 1 << 0,
        FILE_WRITE = 1 << 1,
        FILE_DELETE = 1 << 2
    };

    // TODO: Change open_file to take a UTF-8 string for the path once that's implemented
    KAI_API FileHandle open_file(const char *path, FileFlags access_flags, FileFlags share_flags = FILE_READ);
    KAI_API void close_file(FileHandle file);

    KAI_API size_t get_file_size(FileHandle file);

    KAI_API bool read_file(FileHandle file, void *buffer, size_t byte_count = 0);

    KAI_API void * map_file(const char *path, size_t &out_bytes_mapped, void *location = nullptr);
}

#endif /* KAI_FILEIO_H */
