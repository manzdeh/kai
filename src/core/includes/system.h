/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_SYSTEM_H
#define KAI_SYSTEM_H

#include "utils.h"

namespace kai {
    enum PageAllocFlags {
        ALLOC_RESERVE = 1 << 0,
        ALLOC_COMMIT = 1 << 1
    };

    enum class PageProtection {
        execute,
        execute_read,
        execute_read_write,
        read,
        read_write,
        no_access,
        guard
    };

    KAI_API void * virtual_alloc(void *starting_address, size_t bytes, PageAllocFlags page_flags, PageProtection page_protection);
    KAI_API void * reserve_pages(void *starting_address, size_t page_count);
    KAI_API void * commit_pages(void *reserved_pages, size_t page_count);
    KAI_API void decommit_pages(void *pages, size_t page_count);
    KAI_API void virtual_free(void *address);
    KAI_API size_t get_page_size(void);
}

#endif /* KAI_SYSTEM_H */
