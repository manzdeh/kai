/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ASSET_TYPE
#define KAI_ASSET_TYPE

#include "../core/includes/types.h"

namespace kai {
    enum class AssetType : Uint32 {
        unknown = 0,
        texture,
        mesh
    };
}

#endif /* KAI_ASSET_TYPE */
