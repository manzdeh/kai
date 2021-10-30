/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_MESH_H
#define KAI_MESH_H

#include "../core/includes/types.h"

namespace kai {
    struct RenderBuffer;

    // The actual data that this header describes is stored immediately after the header in the file and in memory
    struct MeshHeader {
        Uint64 buffer_size;
        Uint64 buffer_start;

        struct {
            Uint32 count;
            Uint32 start;
            Uint32 size;
            Uint32 stride;
        } vertices;

        struct {
            Uint32 count;
            Uint32 start;
            Uint32 size;
        } indices;

        struct {
            Uint32 count;
            Uint32 start;
            Uint32 size;
        } texcoords;
    };

    struct MeshData {
        kai::RenderBuffer vertex_buffer;
        kai::RenderBuffer index_buffer;

        MeshHeader header;
    };

    struct MeshView {
        KAI_API MeshView(Uint32 id) : id(id) {
        }

        KAI_API const kai::RenderBuffer * get_vertex_buffer(void) const;
        KAI_API const kai::RenderBuffer * get_index_buffer(void) const;

    private:
        Uint32 id = 0; // TODO: Unused for now
    };
}

#endif /* KAI_MESH_H */
