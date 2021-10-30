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
            static unsigned char temp_buffer[4096];
            void *buf = mesh_asset_storage.get_buffer();

            kai::MeshData *data = reinterpret_cast<kai::MeshData *>(buf);
            kai::MeshHeader *header = reinterpret_cast<kai::MeshHeader *>(reinterpret_cast<unsigned char *>(buf) +
                                                                          offsetof(kai::MeshData, header));

            kai::read_file(test_mesh_file, header);

            {
                kai::RenderBufferInfo buffer_info;
                buffer_info.data = temp_buffer + header->vertices.start;
                buffer_info.byte_size = header->vertices.size;
                buffer_info.stride = header->vertices.stride;
                buffer_info.type = kai::RenderBufferType::vertex;

                device->create_buffer(buffer_info, data->vertex_buffer);
            }

            {
                kai::RenderBufferInfo buffer_info;
                buffer_info.data = header + header->indices.start;
                buffer_info.byte_size = header->indices.size;
                buffer_info.type = kai::RenderBufferType::index;

                device->create_buffer(buffer_info, data->index_buffer);
            }

            kai::close_file(test_mesh_file);
        }
    }
}

kai::MeshView asset_manager_get_mesh(void) {
    return kai::MeshView(0); // TODO: returns a "view" to the temporary mesh for now
}

void destroy_asset_manager(void) {
    mesh_asset_storage.destroy();
}
