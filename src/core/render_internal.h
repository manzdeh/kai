/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "includes/render.h"
#include "includes/types.h"

#ifndef KAI_RENDER_INTERNAL_H
#define KAI_RENDER_INTERNAL_H

#define BACKEND_RENDERER(name) struct name { \
    void init_default_device(kai::RenderDevice &device); \
    void init_device(kai::RenderDevice &device, Uint32 id); \
}

// The rendering backends are not created through an abstract interface,
// because a rendering backend is only set up once upon application
// initialization and can't be changed during runtime. So we wouldn't gain
// much from dynamic dispatch
BACKEND_RENDERER(DX11Renderer);

void init_renderer(kai::RenderDevice &device, kai::RenderingBackend backend);

#endif /* KAI_RENDER_INTERNAL_H */
