/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_ASSET_MANAGER_H
#define KAI_ASSET_MANAGER_H

namespace kai {
    struct MeshView;
}

void init_asset_manager(void);
kai::MeshView asset_manager_get_mesh(void); // TODO: Temporary function
void destroy_asset_manager(void);

#endif /* KAI_ASSET_MANAGER_H */
