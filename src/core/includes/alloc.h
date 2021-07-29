/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ALLOC_H
#define KAI_ALLOC_H

#include "includes/types.h"

struct MemoryManager;

struct MemoryHandle {
    friend struct MemoryManager;
private:
    Uint64 block_start = 0;
    Uint32 block_count = 0;
};

namespace kai {
    typedef Uint32 StackMarker;

    struct StackAllocator {
        explicit StackAllocator(size_t bytes);

        void *alloc(size_t bytes);
        void free(const StackMarker &marker);

        StackMarker get_marker(void);

    private:
        MemoryHandle handle;
        StackMarker current_marker = 0;
    };
}

#endif /* KAI_ALLOC_H */
