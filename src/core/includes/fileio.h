/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_FILEIO_H
#define KAI_FILEIO_H

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
    KAI_API FileHandle open_file(const char *path, FileFlags access_flags, FileFlags share_flags = FileFlags::FILE_NONE);
    KAI_API void close_file(FileHandle *file);

    KAI_API void read_file(FileHandle file, size_t byte_count = 0, void *location = nullptr);

    KAI_API void * map_file(const char *path, size_t &out_bytes_mapped, void *location = nullptr);
}

#endif /* KAI_FILEIO_H */
