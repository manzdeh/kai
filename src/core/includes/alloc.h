/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ALLOC_H
#define KAI_ALLOC_H

#include "includes/types.h"
#include <type_traits>

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
        explicit StackAllocator(Uint32 bytes);

        void * alloc(Uint32 bytes);

        template<typename T>
        T * alloc(void) {
            static_assert(!std::is_pointer<T>::value, "Type shouldn't be a pointer!");
            return (T *)alloc(sizeof(T));
        }

        void free(StackMarker marker);

        StackMarker get_marker(void) const {
            return current_marker;
        }

    private:
        MemoryHandle handle;
        StackMarker current_marker = 0;
    };
}

#endif /* KAI_ALLOC_H */
