/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ALLOC_H
#define KAI_ALLOC_H

#include "types.h"
#include "utils.h"

#include <new>
#include <utility>

namespace kai {
    struct StackAllocator;
    struct PoolAllocator;
    struct ArenaAllocator;
}

struct MemoryHandle {
    friend struct MemoryManager;
    friend struct kai::StackAllocator;
    friend struct kai::PoolAllocator;
    friend struct kai::ArenaAllocator;

private:
    Uint64 get_size(void) const;

    Uint64 block_start = 0;
    Uint32 block_count = 0;
};

namespace kai {
    typedef Uint32 StackMarker;

    struct StackAllocator {
        KAI_API StackAllocator(void) = default;
        KAI_API StackAllocator(Uint32 bytes, Bool32 aligned_allocs = true); // NOTE: Allocators are limited to 4GB

        KAI_API void destroy(void);

        KAI_API void * alloc(Uint32 elem_size, StackMarker *out_marker = nullptr, Uint32 elem_count = 1);

        template<typename T>
        T * alloc(StackMarker *out_marker = nullptr, Uint32 elem_count = 1) {
            return static_cast<T *>(alloc(sizeof(T), out_marker, elem_count));
        }

        template<typename T, typename U, typename...ARGS>
        T * alloc(StackMarker *out_marker = nullptr, Uint32 elem_count = 1, ARGS &&...args) {
            T *obj = alloc<T>(out_marker, elem_count);
            new(obj) U(std::forward<ARGS>(args)...);
            return obj;
        }

        KAI_API void free(StackMarker marker);
        KAI_API void clear(void);

        KAI_API const void * get_data(void) const;
        KAI_API void * get_data(void);

        StackMarker get_marker(void) const {
            return current_marker;
        }

    private:
        MemoryHandle handle;
        StackMarker current_marker = 0;
        Bool32 should_align = true;
    };

    struct PoolAllocator {
        KAI_API PoolAllocator(void) = default;
        KAI_API PoolAllocator(Uint32 elem_size, Uint32 count);

        KAI_API void destroy(void);

        KAI_API void * alloc(void);
        KAI_API void free(void *address);

        KAI_API void clear(void);

    private:
        struct PoolNode {
            PoolNode *next;
        };

        MemoryHandle handle;
        PoolNode *head = nullptr;
        Uint32 element_count = 0;
        Uint32 chunk_size = 0;
    };

    struct ArenaAllocator {
        KAI_API ArenaAllocator(void) = default;
        KAI_API ArenaAllocator(Uint64 bytes);

        KAI_API void destroy(void);

        KAI_API Uint64 get_size(void);
        KAI_API void * get_buffer(void);
        KAI_API void clear(void);

    private:
        MemoryHandle handle;
    };
}

#endif /* KAI_ALLOC_H */
