/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "asset_manager.h"

#include "asset_table.h"
#include "mesh.h"

#include "../core/includes/alloc.h"
#include "../core/includes/fileio.h"
#include "../core/includes/math.h"
#include "../core/includes/render.h"
#include "../core/includes/system.h"

#include <stddef.h>

struct AssetManager {
    struct HashTableEntry {
        AssetId key;
        size_t refcount;
        void *value;
    };

    AssetManager() = default;
    AssetManager(size_t bytes) {
        KAI_ASSERT(kai::is_pow2(bytes));

        size_t page_size = kai::get_page_size();

        page_count = bytes / page_size; // Every asset will occupy at least 1 unique page, so small assets should be packed together for optimal use
        pages = kai::reserve_pages(nullptr, page_count);

        // The asset name lookup table (manifest) is stored at the start of the address space that's reserved for assets
        kai::FileHandle asset_manifest = kai::open_file("data/asset_manifest.bin"); // TODO: Hardcoded for now, but this should probably be retrieved from some config
        KAI_ASSERT(asset_manifest);

        size_t manifest_size = kai::get_file_size(asset_manifest);
        size_t asset_page_count = (manifest_size / page_size) + 1;
        file_paths = static_cast<AssetTableHeader *>(kai::commit_pages(pages, asset_page_count));
        kai::read_file(asset_manifest, file_paths);
        kai::close_file(asset_manifest);

        pages_used += asset_page_count;

        // The hash table is stored immediately after the asset name lookup table
        void *table_page_start = get_page(pages_used);
        table_entries = page_count;
        size_t table_page_count = ((table_entries * sizeof(HashTableEntry)) / page_size) + 1;
        table = static_cast<HashTableEntry *>(kai::commit_pages(table_page_start, table_page_count));
        pages_used += table_page_count;

        table_entries = kai::max<size_t>(0, table_entries - pages_used);

        next_reserved_page = get_page(pages_used);

        // TODO: Need to keep track of a free-list of pages as well
    }

    void * get_page(size_t page_offset, void *start_page = nullptr) {
        start_page = (start_page) ? start_page : pages;
        return reinterpret_cast<void *>(reinterpret_cast<ptrdiff_t>(start_page) + (page_offset * kai::get_page_size()));
    }

    HashTableEntry * find(AssetId id, size_t *out_index = nullptr) const;
    HashTableEntry * insert(AssetId id, void *data);
    HashTableEntry * insert_at(size_t index, AssetId id, void *data);
    void * commit_pages(size_t bytes);

    void linear_probe(size_t &index) const {
        index = (index + 1) % table_entries;
    }

    AssetTableHeader *file_paths = nullptr;
    HashTableEntry *table = nullptr;
    size_t table_entries = 0;
    void *pages = nullptr;
    void *next_reserved_page = nullptr;
    size_t page_count = 0;
    size_t pages_used = 0;
};

AssetManager::HashTableEntry * AssetManager::find(AssetId id, size_t *out_index) const {
    AssetManager::HashTableEntry *entry = nullptr;
    size_t index = id % table_entries;

    while(table[index].key != 0) {
        if(table[index].key == id) {
            entry = &table[index];
            break;
        }

        linear_probe(index);
    }

    if(out_index) {
        *out_index = index;
    }

    return entry;
}

AssetManager::HashTableEntry * AssetManager::insert(AssetId id, void *data) {
    size_t index = id % table_entries;
    size_t count = 0;

    do {
        if(table[index].key == 0) {
            table[index].key = id;
            table[index].refcount = 0;
            table[index].value = data;
            return &table[index];
        } else if(table[index].key == id) {
            return &table[index];
        }

        linear_probe(index);
        count++;
    } while(count < table_entries);

    return nullptr;
}

AssetManager::HashTableEntry * AssetManager::insert_at(size_t index, AssetId id, void *data) {
    if(index < table_entries && table[index].key == 0) {
        table[index].key = id;
        table[index].refcount = 0;
        table[index].value = data;
        return &table[index];
    }

    return nullptr;
}

void * AssetManager::commit_pages(size_t bytes) {
    // TODO: This needs to be done through (and validated against) a free-list of available pages,
    // right now it simply bumps the page up by however many are needed and it could go out of bounds.
    void * committed_pages = nullptr;

    if(bytes > 0) {
        size_t count = (bytes / kai::get_page_size()) + 1;
        committed_pages = kai::commit_pages(next_reserved_page, count);
        next_reserved_page = get_page(count, next_reserved_page);
    }

    return committed_pages;
}

static AssetManager asset_manager;

struct MeshData {
    kai::RenderBuffer vertex_buffer;
    kai::RenderBuffer index_buffer;

    kai::MeshHeader header;
};

void init_asset_manager(void) {
    if(!asset_manager.pages) {
        asset_manager = AssetManager(kai::gibibytes(4));
    }

#if 0
    kai::RenderDevice *device = kai::RenderDevice::get();

    if(device) {
        kai::FileHandle test_mesh_file = kai::open_file("data/test_mesh.bin");
        if(test_mesh_file) {
            void *buf = asset_storage.arena.get_buffer();

            MeshData *mesh_data = reinterpret_cast<MeshData *>(buf);
            kai::MeshHeader *header = reinterpret_cast<kai::MeshHeader *>(reinterpret_cast<unsigned char *>(buf) +
                                                                          offsetof(MeshData, header));

            memset(mesh_data, 0, sizeof(*mesh_data));

            kai::read_file(test_mesh_file, header);

            unsigned char *data_start = reinterpret_cast<unsigned char *>(header) + header->buffer_start;

            {
                kai::RenderBufferInfo buffer_info;
                buffer_info.data = data_start + header->vertices.start;
                buffer_info.byte_size = header->vertices.size;
                buffer_info.stride = header->vertices.stride;
                buffer_info.type = kai::RenderBufferType::vertex;

                device->create_buffer(buffer_info, mesh_data->vertex_buffer);
            }

            {
                kai::RenderBufferInfo buffer_info;
                buffer_info.data = data_start + header->indices.start;
                buffer_info.byte_size = header->indices.size;
                buffer_info.type = kai::RenderBufferType::index;

                device->create_buffer(buffer_info, mesh_data->index_buffer);
            }

            kai::close_file(test_mesh_file);
        }
    }
#endif
}

void destroy_asset_manager(void) {
#if 0
    kai::RenderDevice *device = kai::RenderDevice::get();

    // TODO: At the moment we only have a single mesh asset at the start of the allocated block.
    // This needs to be updated to delete all of the buffers for all of the meshes
    MeshData *data_buffer = reinterpret_cast<MeshData *>(asset_storage.arena.get_buffer());

    if(data_buffer->vertex_buffer.data) {
        device->destroy_buffer(data_buffer->vertex_buffer);
    }

    if(data_buffer->index_buffer.data) {
        device->destroy_buffer(data_buffer->index_buffer);
    }

    asset_storage.arena.destroy();
#endif

    if(asset_manager.pages) {
        kai::virtual_free(asset_manager.pages);
        asset_manager.pages = nullptr;
        asset_manager.page_count = 0;
    }
}

void * load_asset(AssetId id) {
    size_t table_index;
    AssetManager::HashTableEntry *asset = asset_manager.find(id, &table_index);

    if(asset && asset->refcount > 0) {
        asset->refcount++;
        return asset->value;
    }

    AssetTableHeader *table = asset_manager.file_paths;

    size_t index = (id ^ table->mod) % table->count;
    ptrdiff_t start = reinterpret_cast<ptrdiff_t>(table) + offsetof(AssetTableHeader, buffer);
    ptrdiff_t offset = *reinterpret_cast<ptrdiff_t *>(start + (index * sizeof(ptrdiff_t)));

    const char *path = reinterpret_cast<const char *>(start + offset);

    kai::FileHandle asset_file = kai::open_file(path);
    void *asset_data = nullptr;

    AssetManager::HashTableEntry *entry = asset_manager.insert_at(table_index, id, nullptr);
    if(entry) {
        asset_data = asset_manager.commit_pages(kai::get_file_size(asset_file));
        KAI_ASSERT(asset_data);

        kai::read_file(asset_file, asset_data);
    }

    kai::close_file(asset_file);

    return asset_data;
}

void unload_asset(AssetId id) {
    AssetManager::HashTableEntry *entry = asset_manager.find(id);

    if(entry) {
        if(entry->refcount > 0) {
            entry->refcount--;
        }

        if(entry->refcount == 0) {
            // TODO: Remove entry from the hash table
            memset(entry, 0, sizeof(*entry));
        }
    } else {
#ifdef KAI_DEBUG
        kai::log("Tried to unload a file with hash: \"%llu\", but the file was not loaded before", id);
#endif
    }
}
