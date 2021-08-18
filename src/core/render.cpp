/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "includes/render.h"
#include "render_internal.h"

static struct {
#ifdef KAI_PLATFORM_WIN32
    DX11Renderer *dx11;
#endif
    kai::RenderingBackend active_backend;
} render_manager = {};

#define RENDERER_CASE(renderer, backend_name, name, ...) \
    case kai::RenderingBackend::backend_name: \
        render_manager.renderer->name(__VA_ARGS__); \
        break

#ifdef KAI_PLATFORM_WIN32
#define DX11_CASE(name, ...) \
    RENDERER_CASE(dx11, dx11, name, __VA_ARGS__);
#else
#define DX11_CASE(name, ...)
#endif

#define RENDERER_FUNC(name, ...) \
    switch(render_manager.active_backend) { \
        DX11_CASE(name, __VA_ARGS__); \
    }

void init_renderer(kai::RenderDevice &device, kai::RenderingBackend backend) {
    device.backend = backend;
    render_manager.active_backend = backend;

    switch(backend) {
#ifdef KAI_PLATFORM_WIN32
        case kai::RenderingBackend::dx11:
            render_manager.dx11 = reinterpret_cast<DX11Renderer *>(platform_get_backend_renderer(backend));
            break;
#endif
        default:
            break;
    }
}

kai::Window * kai::get_window(void) {
    return platform_get_kai_window();
}

void kai::init_default_device(RenderDevice &out_device) {
    RENDERER_FUNC(init_default_device, out_device);
}

void kai::init_device(RenderDevice &out_device, Uint32 id) {
    RENDERER_FUNC(init_device, out_device, id);
}

#undef RENDERER_CASE
#undef DX11_CASE
#undef RENDERER_FUNC
