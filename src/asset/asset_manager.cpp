/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "asset_manager.h"
#include "mesh.h"

#include "../core/includes/alloc.h"
#include "../core/includes/fileio.h"
#include "../core/includes/render.h"

static kai::ArenaAllocator mesh_asset_storage;

void init_asset_manager(void) {
    mesh_asset_storage = kai::ArenaAllocator(4096); // TODO: Really small buffer for now, because we only want to load 1 simple mesh at the moment
    kai::RenderDevice *device = kai::RenderDevice::get();

    if(device) {
        kai::FileHandle test_mesh_file = kai::open_file("data/test_mesh.bin");
        if(test_mesh_file) {
            void *buf = mesh_asset_storage.get_buffer();

            kai::MeshData *mesh_data = reinterpret_cast<kai::MeshData *>(buf);
            kai::MeshHeader *header = reinterpret_cast<kai::MeshHeader *>(reinterpret_cast<unsigned char *>(buf) +
                                                                          offsetof(kai::MeshData, header));

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
}

kai::MeshView asset_manager_get_mesh(void) {
    return kai::MeshView(0); // TODO: returns a "view" to the temporary mesh for now
}

void destroy_asset_manager(void) {
    kai::RenderDevice *device = kai::RenderDevice::get();

    // TODO: At the moment we only have a single mesh asset at the start of the allocated block.
    // This needs to be updated to delete all of the buffers for all of the meshes
    kai::MeshData *data_buffer = reinterpret_cast<kai::MeshData *>(mesh_asset_storage.get_buffer());

    if(data_buffer->vertex_buffer.data) {
        device->destroy_buffer(data_buffer->vertex_buffer);
    }

    if(data_buffer->index_buffer.data) {
        device->destroy_buffer(data_buffer->index_buffer);
    }

    mesh_asset_storage.destroy();
}
