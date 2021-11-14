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
    AssetManager() = default;
    AssetManager(size_t bytes) : page_count(bytes / kai::get_page_size()) {
        KAI_ASSERT(kai::is_pow2(bytes));
        pages = kai::reserve_pages(nullptr, page_count);
    }

    void *pages = nullptr;
    size_t page_count = 0;
};

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
