/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "includes/render.h"
#include "render_internal.h"

static struct {
    kai::Renderer *renderer;
    kai::RenderingBackend active_backend;
} render_manager = {};

void init_renderer(kai::RenderDevice &device, kai::RenderingBackend backend) {
    device.backend = backend;
    render_manager.active_backend = backend;

    switch(backend) {
        case kai::RenderingBackend::dx11:
#ifdef KAI_PLATFORM_WIN32
            render_manager.renderer = platform_get_backend_renderer(backend);
#else
            kai::log("Direct3D11 is not supported on this platform!\n");
#endif
            break;
        default:
            break;
    }
}

kai::Window * kai::get_window(void) {
    return platform_get_kai_window();
}

kai::Renderer * kai::get_renderer(void) {
    return render_manager.renderer;
}
