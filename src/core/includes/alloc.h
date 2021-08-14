/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ALLOC_H
#define KAI_ALLOC_H

#include "types.h"
#include "utils.h"

struct MemoryManager;
namespace kai {
    struct StackAllocator;
}

struct MemoryHandle {
    friend struct MemoryManager;
    friend struct kai::StackAllocator;

private:
    Uint64 get_size(void) const;

    Uint64 block_start = 0;
    Uint32 block_count = 0;
};

namespace kai {
    typedef Uint32 StackMarker;

    struct StackAllocator {
        KAI_API StackAllocator(Uint32 bytes, Bool32 aligned_allocs = true); // NOTE: Allocators are limited to 4GB

        KAI_API void destroy(void);

        KAI_API void * alloc(Uint32 bytes, StackMarker *out_marker = nullptr);

        template<typename T>
        T * alloc(StackMarker *out_marker = nullptr, Uint32 elem_count = 1) {
            return (T *)alloc(sizeof(T) * elem_count, out_marker);
        }

        KAI_API void free(StackMarker marker);
        KAI_API void clear(void);

        StackMarker get_marker(void) const {
            return current_marker;
        }

    private:
        MemoryHandle handle;
        StackMarker current_marker = 0;
        Bool32 should_align = true;
    };
}

#endif /* KAI_ALLOC_H */
