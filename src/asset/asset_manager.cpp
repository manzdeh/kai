/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "asset_manager.h"
#include "mesh.h"

#include "../core/includes/alloc.h"
#include "../core/includes/fileio.h"
#include "../core/includes/render.h"
#include "../core/includes/system.h"

struct AssetManager {
    struct HashTable {
        AssetId key;
        void *value;
    };

    AssetManager() = default;
    AssetManager(size_t bytes) {
        KAI_ASSERT(kai::is_pow2(bytes));

        size_t page_size = kai::get_page_size();

        page_count = bytes / page_size; // Every asset will occupy at least 1 unique page, so small assets should be packed together for optimal use
        pages = kai::reserve_pages(nullptr, page_count);

        // The hash table is stored at the start of the address space that's reserved for assets
        table_entries = page_count * sizeof(HashTable);
        size_t table_page_count = (table_entries / page_size) + 1;
        table = static_cast<HashTable *>(kai::commit_pages(pages, table_page_count));
        pages_used += table_page_count;

        next_reserved_page = static_cast<unsigned char *>(pages) + (table_page_count * page_size);

        // TODO: Need to keep track of a free-list of pages as well
    }

    void * find(AssetId id) const;
    void * insert(AssetId id, void *data);

    void linear_probe(size_t &index) const {
        index = (index + 1) % table_entries;
    }

    HashTable *table = nullptr;
    size_t table_entries = 0;
    void *pages = nullptr;
    void *next_reserved_page = nullptr;
    size_t page_count = 0;
    size_t pages_used = 0;
};

void * AssetManager::find(AssetId id) const {
    void *value = nullptr;
    size_t index = id % table_entries;

    do {
        if(table[index].key == id) {
            value = table[index].value;
            break;
        }

        linear_probe(index);
    } while(table[index].key != 0);

    return value;
}

void * AssetManager::insert(AssetId id, void *data) {
    size_t index = id % table_entries;

    do {
        if(table[index].key == id) {
            return table[index].value;
        } else if(table[index].key == 0) {
            table[index].key = id;
            table[index].value = data;
            return table[index].value;
        }

        linear_probe(index);
    } while(table[index].key != 0);

    return nullptr;
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

void * load_asset(AssetId /*id*/) {
    // STUB
    return nullptr;
}

void unload_asset(AssetId /*id*/) {
    // STUB
}
