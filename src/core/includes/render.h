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

    struct RenderDevice {
        Uint32 id;
        const char name[64];
    };

    struct RenderBuffer {
        void *data;
    };

    struct RenderPipeline {
        RenderBuffer *vertex_buffer;
        VertexShaderID vertex_shader;
        PixelShaderID pixel_shader;
    };

    void init_default_device(RenderDevice &device);
    void init_device(RenderDevice &device, Uint32 id);
}

#endif /* KAI_RENDER_H */
