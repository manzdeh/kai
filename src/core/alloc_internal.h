/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ALLOC_INTERNAL_H
#define KAI_ALLOC_INTERNAL_H

struct MemoryManager;

struct MemoryHandle {
    friend struct MemoryManager;
private:
    Uint32 block_start;
    Uint32 block_count;
};

struct MemoryManager {
    static void init(size_t size);
    static void destroy(void);

    static bool alloc_backing_memory(MemoryHandle &handle, size_t bytes);
    static void free_backing_memory(const MemoryHandle &handle);

    static void * get_ptr(const MemoryHandle &handle);

    void *buffer;
    unsigned char *header;
    void *start;
    Uint64 bytes_used;
    Uint64 bytes_size;
    Uint64 total_block_count;
    Uint64 next_block_id;
};

#endif /* KAI_ALLOC_INTERNAL_H */
