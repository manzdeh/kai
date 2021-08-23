/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "includes/render.h"
#include "render_internal.h"

void init_renderer(kai::RenderingBackend backend) {
#ifndef KAI_PLATFORM_WIN32
    if(backend == kai::RenderingBackend::dx11) {
        kai::log("Direct3D11 is not supported on this platform!\n");
        return;
    }
#endif

    platform_renderer_init_backend(backend);
}

void destroy_renderer(void) {
    platform_renderer_destroy_backend();
}

kai::RenderDevice * kai::RenderDevice::init_device(void) {
    return  platform_renderer_init_device(*get_engine_memory());
}

kai::RenderDevice * kai::RenderDevice::init_device(Uint32 id) {
    return platform_renderer_init_device(*get_engine_memory(), id);
}

kai::Window * kai::get_window(void) {
    return platform_get_kai_window();
}
