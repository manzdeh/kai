/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include <windows.h>
#include "win32_fileio.h"

static size_t get_file_size(kai::FileHandle file) {
    KAI_ASSERT(file);

    LARGE_INTEGER size;
    GetFileSizeEx(file, &size);

    return static_cast<size_t>(size.QuadPart);
}

kai::FileHandle kai::open_file(const char *path, FileFlags access_flags, FileFlags share_flags) {
#define MAP_FLAG(flags_var, kai_flag, win_flag, mapping) \
    do { \
        if((flags_var) & kai::kai_flag) { \
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

#undef MAP_FLAG

    // TODO: Change to CreateFileW once this takes a UTF-8 string
    HANDLE f = CreateFileA(path, access, share, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    return (f != INVALID_HANDLE_VALUE) ? static_cast<kai::FileHandle>(f) : nullptr;
}

void kai::close_file(kai::FileHandle file) {
    if(file) {
        CloseHandle(static_cast<HANDLE>(file));
    }
}

size_t kai::get_file_size(kai::FileHandle file) {
    if(file) {
        return ::get_file_size(file);
    }

    return 0;
}

bool kai::read_file(kai::FileHandle file, void *buffer, size_t byte_count) {
    if(file && buffer) {
        size_t size = ::get_file_size(file);
        if(byte_count > size) {
            return false;
        }

        DWORD bytes_read;
        BOOL retval = ReadFile(file, buffer, static_cast<DWORD>(((byte_count > 0) ? byte_count : size)), &bytes_read, nullptr);
        return (retval) ? retval : GetLastError() == ERROR_IO_PENDING;
    }

    return false;
}

void kai::rewind_file(kai::FileHandle file) {
    if(file) {
        SetFilePointer(static_cast<HANDLE>(file), 0, nullptr, FILE_BEGIN);
    }
}
