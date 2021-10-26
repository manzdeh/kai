/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef GLTF2_PARSER_H
#define GLTF2_PARSER_H

#include <stdint.h>

#include "../../core/includes/utils.h"

namespace kai::gltf2 {
    struct MeshData {
        size_t vertex_count = 0;
        size_t vertex_byte_size = 0;
        size_t vertex_stride = 0;

        size_t index_count = 0;
        size_t index_byte_size = 0;
        size_t index_byte_offset = 0;

        // Values above -1 indicate the attributes' vertex layout position.
        // TODO: This should be changed to an array, because it doesn't
        // allow for more than 1 texture coordinate to be set for instance
        union Attributes {
            struct {
                int32_t position;
                int32_t normal;
                int32_t tangent;
                int32_t texcoord;
                int32_t color;
                int32_t joint;
                int32_t weight;
            };
            int32_t data[7] = { -1, -1, -1, -1, -1, -1, -1 };
        } attributes;

        unsigned char KAI_FLEXIBLE_ARRAY(data);
    };

    // TODO: Change this to take the path to the file instead of taking a buffer (should make use of the engine's File I/O)
    MeshData * load(const char *buffer, const char *dir = nullptr, size_t size = 0);
    void destroy(MeshData *data);
}

#endif /* GLTF2_PARSER_H */
