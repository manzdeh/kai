/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ASSET_MANAGER_H
#define KAI_ASSET_MANAGER_H

#include "../core/includes/types.h"

using AssetId = Uint64;
#define KAI_ASSET_ID constexpr AssetId

void init_asset_manager(void);
void destroy_asset_manager(void);

void * load_asset(AssetId id);

#endif /* KAI_ASSET_MANAGER_H */
