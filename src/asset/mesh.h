/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_MESH_H
#define KAI_MESH_H

#include "asset_type.h"

#include "../core/includes/types.h"

namespace kai {
    // The actual data that this header describes is stored immediately after the header in the file and in memory
#pragma pack(push, 1)
    struct MeshHeader {
        AssetType asset_type;

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
            Uint32 count; // Refers to the number of UV coordinate attributes (TEXCOORD_0, TEXCOORD_1, etc.)
            Uint32 start; // Stores the offset to the 1st UV coordinate in the buffer. The subsequent ones can be inferred from the .start member and vertices.stride
        } texcoords;

        AssetId texture_id; // TODO: For now meshes can only reference one texture. Eventually this probably needs to be a reference to a material of sorts
    };
#pragma pack(pop)
}

#endif /* KAI_MESH_H */
