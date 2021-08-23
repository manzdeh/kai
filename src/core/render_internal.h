/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_RENDER_INTERNAL_H
#define KAI_RENDER_INTERNAL_H

#include "includes/render.h"
#include "includes/types.h"

void init_renderer(kai::RenderingBackend backend);
void destroy_renderer(void);

#endif /* KAI_RENDER_INTERNAL_H */
