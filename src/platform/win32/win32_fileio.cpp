/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "win32_fileio.h"

kai::FileHandle kai::open_file(const char *path, FileFlags access_flags, FileFlags share_flags) {
#define MAP_FLAG(flags_var, kai_flag, win_flag, mapping) \
    do { \
        if((flags_var) & kai::FileFlags::kai_flag) { \
            win_flag |= mapping; \
        } \
    } while(0)

    DWORD access = 0;
    DWORD share = 0;

    MAP_FLAG(access_flags, FILE_READ, access, GENERIC_READ);
    MAP_FLAG(access_flags, FILE_WRITE, access, GENERIC_WRITE);
    MAP_FLAG(share_flags, FILE_READ, share, FILE_SHARE_READ);
    MAP_FLAG(share_flags, FILE_WRITE, share, FILE_SHARE_WRITE);
    MAP_FLAG(share_flags, FILE_DELETE, share, FILE_SHARE_DELETE);

    // TODO: Change to CreateFileW once this takes a UTF-8 string
    HANDLE f = CreateFileA(path, access, share, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    return (f != INVALID_HANDLE_VALUE) ? static_cast<kai::FileHandle>(f) : nullptr;
}

void kai::close_file(kai::FileHandle *file) {
    if(file) {
        if(*file) {
            CloseHandle(static_cast<HANDLE>(file));
        }

        *file = nullptr;
    }
}
