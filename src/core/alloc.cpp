/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include <string.h>

#include "alloc_internal.h"

#include "includes/utils.h"
#include "../platform/platform.h"

#define BLOCK_SIZE kai::kibibytes(4)

static struct {
    size_t used;
    size_t size;
    void *buffer;
    void *header;
    void *start;
} memory_manager;

static inline void reset_memory_manager(void) {
    memset(&memory_manager, 0, sizeof(memory_manager));
}

void init_memory_manager(size_t size) {
    if(size > 0 && !memory_manager.buffer) {
        reset_memory_manager();

        void *address = nullptr;
#ifdef KAI_DEBUG
        address = (void *)kai::gibibytes(2);
#endif

        kai::align_to_pow2(size, platform_get_page_size());

        memory_manager.buffer = platform_alloc_mem_arena(size, address);
        memory_manager.size = size;
        memory_manager.used = 0;
    }
}

void destroy_memory_manager(void) {
    platform_free_mem_arena(memory_manager.buffer);
    reset_memory_manager();
}

#undef BLOCK_SIZE
