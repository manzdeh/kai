/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include <string.h>

#include "includes/alloc.h"
#include "includes/utils.h"
#include "alloc_internal.h"
#include "../platform/platform.h"

#define BLOCK_SIZE 256

static MemoryManager memory_manager;

static void reset_memory_manager(void) {
    memset(&memory_manager, 0, sizeof(memory_manager));
}

template<typename T>
static void destroy_allocator(T *allocator, MemoryHandle &handle) {
    MemoryManager::free_blocks(handle);
    memset(allocator, 0, sizeof(*allocator));
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
    index %= (memory_manager.total_block_count / 8);
}

void MemoryManager::init(size_t size) {
    if(size > 0 && !memory_manager.buffer) {
        reset_memory_manager();

        void *address = nullptr;
#ifdef KAI_DEBUG
        address = reinterpret_cast<void *>(kai::gibibytes(2));
#endif

        memory_manager.bytes_size = size;

        Uint64 block_count = size / BLOCK_SIZE;
        Uint64 block_bytes = block_count / 8;
        size += block_bytes;

        kai::align_to_pow2(size, kai::get_page_size());

        memory_manager.buffer = platform_alloc_mem_arena(size, address);
        memory_manager.total_bytes = size;
        memory_manager.bytes_used = 0;
        memory_manager.total_block_count = block_count;

        memory_manager.header = static_cast<unsigned char *>(memory_manager.buffer);
        memory_manager.start = memory_manager.header + block_bytes;
    }
}

void MemoryManager::destroy(void) {
    platform_free_mem_arena(memory_manager.buffer);
    reset_memory_manager();
}

bool MemoryManager::reserve_blocks(MemoryHandle &handle, size_t bytes) {
    Uint64 bytes_used = memory_manager.bytes_used + bytes;

    if(bytes_used < memory_manager.bytes_size) {
        Uint32 block_count = static_cast<Uint32>((bytes - 1) / BLOCK_SIZE) + 1;
        Uint64 initial_block_id = memory_manager.next_block_id;

        Uint64 header_index;
        Uint64 header_bit;
        get_initial_header_index(header_index, header_bit);

        bool valid;
        Uint32 blocks_evaluated = 0;

        do {
            valid = true;

            for(Uint32 i = 0; i < block_count; i++) {
                blocks_evaluated++;

                if(memory_manager.header[header_index] & (1 << header_bit)) {
                    valid = false;
                    advance_header_index(header_index, header_bit);
                    break;
                }

                advance_header_index(header_index, header_bit);
            }

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

            memory_manager.bytes_used = bytes_used;
            memory_manager.next_block_id = (header_index * 8) + header_bit;
            return true;
        } else {
            memory_manager.next_block_id = initial_block_id;
        }
    }

    return false;
}

void MemoryManager::free_blocks(MemoryHandle &handle) {
    Uint64 header_index;
    Uint64 header_bit;
    get_initial_header_index(header_index, header_bit, &handle.block_start);

    for(Uint32 i = 0; i < handle.block_count; i++) {
        memory_manager.header[header_index] &= ~(1 << header_bit);
        advance_header_index(header_index, header_bit);
    }

    Uint64 bytes_reclaimed = handle.get_size();
    if((memory_manager.bytes_used - bytes_reclaimed) < memory_manager.bytes_used) { // Handle integer overflow
        memory_manager.bytes_used -= bytes_reclaimed;
    } else {
        memory_manager.bytes_used = 0;
    }

    handle.block_start = 0;
    handle.block_count = 0;
}

void * MemoryManager::get_ptr(const MemoryHandle &handle, Uint32 byte_offset) {
    if(handle.block_count > 0 && byte_offset < handle.get_size()) {
        return static_cast<unsigned char *>(memory_manager.start) + (handle.block_start * BLOCK_SIZE) + byte_offset;
    }

    return nullptr;
}

Uint64 MemoryHandle::get_size(void) const {
    return BLOCK_SIZE * block_count;
}

// -------------------------------------------------- Stack Allocator -------------------------------------------------- //
kai::StackAllocator::StackAllocator(Uint32 bytes, Bool32 aligned_allocs) : should_align(aligned_allocs) {
    if(!MemoryManager::reserve_blocks(handle, bytes)) {
        // TODO: Error logging
    }
}

void kai::StackAllocator::destroy(void) {
    destroy_allocator(this, handle);
}

void * kai::StackAllocator::alloc(Uint32 elem_size, StackMarker *out_marker, Uint32 elem_count) {
    Uint32 bytes = elem_size * elem_count;
    void *address = nullptr;

    if(bytes > 0) {
        if(should_align) {
            kai::align_to_pow2(bytes, 4u);
        }

        Uint32 bytes_free = static_cast<Uint32>(handle.get_size()) - current_marker;

        if(bytes_free > 0 && bytes <= bytes_free) {
            address = MemoryManager::get_ptr(handle, current_marker);

            if(address) {
                if(out_marker) {
                    *out_marker = current_marker;
                }
                current_marker += bytes;
            }
        }
    }

    return address;
}

void kai::StackAllocator::free(StackMarker marker) {
    bool should_clear = marker == 0 || marker > current_marker;
    if(should_clear) {
        clear();
        return;
    }

    current_marker = marker;
}

void kai::StackAllocator::clear(void) {
    current_marker = 0;
}

const void * kai::StackAllocator::get_data(void) const {
    return MemoryManager::get_ptr(handle);
}

void * kai::StackAllocator::get_data(void) {
    return MemoryManager::get_ptr(handle);
}

// -------------------------------------------------- Pool Allocator -------------------------------------------------- //
kai::PoolAllocator::PoolAllocator(Uint32 elem_size, Uint32 count) :
    element_count(count),
    chunk_size(elem_size) {

    KAI_ASSERT(elem_size >= sizeof(PoolNode *) && count > 1);

    size_t bytes_needed = (elem_size + sizeof(PoolNode *)) * count;

    if(MemoryManager::reserve_blocks(handle, bytes_needed)) {
        clear();
    } else {
        // TODO: Error logging
    }
}

void kai::PoolAllocator::destroy(void) {
    destroy_allocator(this, handle);
}

void * kai::PoolAllocator::alloc(void) {
    if(head) {
        void *addr = head + 1;
        head = head->next;
        return addr;
    }

    // TODO: Log an error that we ran out of space in this allocator
    return nullptr;
}

void kai::PoolAllocator::free(void *address) {
    PoolNode *header = reinterpret_cast<PoolNode *>(static_cast<unsigned char *>(address) - sizeof(PoolNode *));
    header->next = head;
    head = header;
}

void kai::PoolAllocator::clear(void) {
    memset(MemoryManager::get_ptr(handle), 0, handle.get_size());

    // Set all the nodes in the free-list
    head = static_cast<PoolNode *>(MemoryManager::get_ptr(handle));
    PoolNode *node = head;
    uintptr_t next = reinterpret_cast<uintptr_t>(head);
    for(Uint32 i = 0; i < (element_count - 1); i++) {
        next += (chunk_size + sizeof(PoolNode *));

        PoolNode *p = reinterpret_cast<PoolNode *>(next);
        node->next = p;
        node = p;
    }

    node->next = nullptr;
}

// -------------------------------------------------- Arena Allocator -------------------------------------------------- //
kai::ArenaAllocator::ArenaAllocator(Uint64 bytes) {
    if(!MemoryManager::reserve_blocks(handle, bytes)) {
        // TODO: Error logging
    }
}

void kai::ArenaAllocator::destroy(void) {
    destroy_allocator(this, handle);
}

Uint64 kai::ArenaAllocator::get_size(void) {
    return handle.get_size();
}

void * kai::ArenaAllocator::get_buffer(void) {
    return MemoryManager::get_ptr(handle);
}

void kai::ArenaAllocator::clear(void) {
    memset(get_buffer(), 0, get_size());
}

#undef BLOCK_SIZE
