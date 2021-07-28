/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include <string.h>

#include "alloc_internal.h"

#include "includes/utils.h"
#include "../platform/platform.h"

#define BLOCK_SIZE 64

static MemoryManager memory_manager;

static inline void reset_memory_manager(void) {
    memset(&memory_manager, 0, sizeof(memory_manager));
}

static KAI_FORCEINLINE void get_initial_header_index(Uint64 &index, Uint64 &bit, const Uint64 *start_id = nullptr) {
    Uint64 id = (start_id) ? *start_id : memory_manager.next_block_id;
    index = id / 8;
    bit = id % 8;
}

static KAI_FORCEINLINE void advance_header_index(Uint64 &index, Uint64 &bit) {
    bit++;
    index += (bit / 8);
    bit %= 8;
}

void MemoryManager::init(size_t size) {
    if(size > 0 && !memory_manager.buffer) {
        reset_memory_manager();

        void *address = nullptr;
#ifdef KAI_DEBUG
        address = (void *)kai::gibibytes(2);
#endif

        Uint64 block_count = size / BLOCK_SIZE;
        Uint64 block_bytes = block_count / 8;
        size += block_bytes;

        kai::align_to_pow2(size, platform_get_page_size());

        memory_manager.buffer = platform_alloc_mem_arena(size, address);
        memory_manager.bytes_size = size;
        memory_manager.bytes_used = 0;
        memory_manager.total_block_count = block_count;

        memory_manager.header = (unsigned char *)memory_manager.buffer;
        memory_manager.start = memory_manager.header + block_bytes;
    }
}

void MemoryManager::destroy(void) {
    platform_free_mem_arena(memory_manager.buffer);
    reset_memory_manager();
}

bool MemoryManager::alloc_backing_memory(MemoryHandle &handle, size_t bytes) {
    Uint64 bytes_used = memory_manager.bytes_used + bytes;

    if(bytes_used < memory_manager.bytes_size) {
        Uint32 block_count = (Uint32)((bytes - 1) / BLOCK_SIZE) + 1;
        Uint64 initial_block_id = memory_manager.next_block_id;

        Uint64 header_index;
        Uint64 header_bit;
        get_initial_header_index(header_index, header_bit);

        bool valid;
        Uint32 blocks_evaluated = 0;

        do {
            valid = true;

            for(Uint32 i = 0; i < block_count; i++) {
                if(memory_manager.header[header_index] & (1 << header_bit)) {
                    valid = false;
                }

                advance_header_index(header_index, header_bit);
            }

            blocks_evaluated += block_count;
            valid &= blocks_evaluated < memory_manager.total_block_count;

            if(!valid) {
                memory_manager.next_block_id = ((header_index * 8) + header_bit) % memory_manager.total_block_count;
            }
        } while(!valid && blocks_evaluated < memory_manager.total_block_count);

        if(valid) {
            get_initial_header_index(header_index, header_bit);

            // Mark all used blocks
            for(Uint32 i = 0; i < block_count; i++) {
                memory_manager.header[header_index] |= (1 << header_bit);
                advance_header_index(header_index, header_bit);
            }

            handle.block_start = memory_manager.next_block_id;
            handle.block_count = block_count;

            memory_manager.bytes_used += bytes_used;
            memory_manager.next_block_id = (header_index * 8) + header_bit;
            return true;
        } else {
            memory_manager.next_block_id = initial_block_id;
        }
    }

    return false;
}

void MemoryManager::free_backing_memory(MemoryHandle &handle) {
    Uint64 header_index;
    Uint64 header_bit;
    get_initial_header_index(header_index, header_bit, &handle.block_start);

    for(Uint32 i = 0; i < handle.block_count; i++) {
        memory_manager.header[header_index] &= ~(1 << header_bit);
        advance_header_index(header_index, header_bit);
    }

    memory_manager.bytes_used -= (BLOCK_SIZE * handle.block_count);
    handle.block_start = 0;
    handle.block_count = 0;
}

#undef BLOCK_SIZE
