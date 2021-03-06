/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ASSET_TABLE
#define KAI_ASSET_TABLE

#include "../core/includes/types.h"
#include "../core/includes/utils.h"

struct AssetTableHeader {
    size_t count;
    Uint64 mod;
    char KAI_FLEXIBLE_ARRAY(buffer);
};

#endif /* KAI_ASSET_TABLE */
