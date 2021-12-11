/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ASSET_TYPE
#define KAI_ASSET_TYPE

#include "../core/includes/types.h"
#include "../core/includes/utils.h"

#define KAI_NULL_ASSET_ID 0

using AssetId = Uint64;
#define GET_KAI_ASSET_ID(name, str) constexpr AssetId name = kai::fnv1a64_str_hash(str)

namespace kai {
    enum class AssetType : Uint32 {
        unknown = 0,
        texture,
        mesh
    };
}

#endif /* KAI_ASSET_TYPE */
