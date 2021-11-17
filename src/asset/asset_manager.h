/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ASSET_MANAGER_H
#define KAI_ASSET_MANAGER_H

#include "../core/includes/types.h"

using AssetId = Uint64;
#define GET_KAI_ASSET_ID(name, str) constexpr AssetId name = kai::fnv1a64_str_hash(str)

void init_asset_manager(void);
void destroy_asset_manager(void);

void * load_asset(AssetId id);
void unload_asset(AssetId id);

#endif /* KAI_ASSET_MANAGER_H */
