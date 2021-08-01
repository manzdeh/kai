/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ALLOC_INTERNAL_H
#define KAI_ALLOC_INTERNAL_H

struct MemoryManager {
    static void init(size_t size);
    static void destroy(void);

    static bool reserve_blocks(MemoryHandle &handle, size_t bytes);
    static void free_blocks(MemoryHandle &handle);

    static void * get_ptr(const MemoryHandle &handle, Uint32 block_offset = 0);

    void *buffer;
    unsigned char *header;
    void *start;
    Uint64 bytes_used;
    Uint64 bytes_size;
    Uint64 total_bytes;
    Uint64 total_block_count;
    Uint64 next_block_id;
};

#endif /* KAI_ALLOC_INTERNAL_H */
