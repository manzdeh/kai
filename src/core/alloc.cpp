/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include <string.h>

#include "alloc_internal.h"

#include "includes/utils.h"
#include "../platform/platform.h"

#define BLOCK_SIZE 256

static MemoryManager memory_manager;

static inline void reset_memory_manager(void) {
    memset(&memory_manager, 0, sizeof(memory_manager));
}

void MemoryManager::init(size_t size) {
    if(size > 0 && !memory_manager.buffer) {
        reset_memory_manager();

        void *address = nullptr;
#ifdef KAI_DEBUG
        address = (void *)kai::gibibytes(2);
#endif

        Uint64 block_size = (size / (BLOCK_SIZE * 8)) + 1;
        size += block_size;

        kai::align_to_pow2(size, platform_get_page_size());

        memory_manager.buffer = platform_alloc_mem_arena(size, address);
        memory_manager.bytes_size = size;
        memory_manager.bytes_used = 0;
        memory_manager.total_block_count = block_size;

        memory_manager.header = (unsigned char *)memory_manager.buffer;
        memory_manager.start = memory_manager.header + block_size;
    }
}

void MemoryManager::destroy(void) {
    platform_free_mem_arena(memory_manager.buffer);
    reset_memory_manager();
}

bool MemoryManager::alloc_backing_memory(MemoryHandle &handle, size_t bytes) {
    Uint64 bytes_used = memory_manager.bytes_used + bytes;

    if(bytes_used < memory_manager.bytes_size) {
        Uint32 block_count = (Uint32)(bytes / BLOCK_SIZE) + 1;
        Uint64 initial_block_id = memory_manager.next_block_id;

        Uint64 starting_header_index = (memory_manager.next_block_id / 8);
        Uint64 starting_header_bit = (memory_manager.next_block_id % 8);

        Uint64 header_index = starting_header_index;
        Uint64 header_bit = starting_header_bit;

        bool valid;
        Uint32 blocks_evaluated = 0;

        do {
            valid = true;

            for(Uint32 i = 0; i < block_count; i++) {
                if(memory_manager.header[header_index] & (1 << header_bit)) {
                    valid = false;
                    break;
                }

                header_bit++;
                header_index += (header_bit / 8);
                header_bit %= 8;
            }

            blocks_evaluated += block_count;
            memory_manager.next_block_id = (header_index * 8) + header_bit;

        } while(!valid && blocks_evaluated < memory_manager.total_block_count);

        if(valid) {
            header_index = starting_header_index;
            header_bit = starting_header_bit;

            // Mark all used blocks
            for(Uint32 i = 0; i < block_count; i++) {
                memory_manager.header[header_index] |= (1 << header_bit);
                header_bit++;
                header_index += (header_bit / 8);
                header_bit %= 8;
            }

            handle.block_start = (Uint32)memory_manager.next_block_id;
            handle.block_count = block_count;

            memory_manager.bytes_used = bytes_used;
            memory_manager.next_block_id = (header_index * 8) + header_bit;
            return true;
        } else {
            memory_manager.next_block_id = initial_block_id;
        }
    }

    return false;
}

#undef BLOCK_SIZE
