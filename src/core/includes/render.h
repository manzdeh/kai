/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_RENDER_H
#define KAI_RENDER_H

#include "types.h"

namespace kai {
    typedef Uint32 VertexShaderID;
    typedef Uint32 PixelShaderID;

    struct Window {
        void *platform_window;
        Uint32 width;
        Uint32 height;
    };

    enum class RenderingBackend {
        unknown,
        dx11,
    };

    struct RenderDevice {
        Uint32 id;
        RenderingBackend backend;
        char name[64];
    };

    struct RenderBuffer {
        void *data;
    };

    struct RenderPipeline {
        RenderBuffer *vertex_buffer;
        VertexShaderID vertex_shader;
        PixelShaderID pixel_shader;
    };

    Window * get_window(void);

    void init_default_device(RenderDevice &out_device);
    void init_device(RenderDevice &out_device, Uint32 id);
}

#endif /* KAI_RENDER_H */
