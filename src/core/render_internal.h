/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_RENDER_INTERNAL_H
#define KAI_RENDER_INTERNAL_H

#include "includes/render.h"
#include "includes/types.h"

void init_renderer(kai::RenderingBackend backend, const Uint32 *device_id = nullptr);
void destroy_renderer(void);

enum class CommandEncoding : Uint32 {
    draw,
    draw_indexed,
    bind_buffer,
    clear_color,
    clear_depth,
    clear_stencil,
    clear_depth_stencil,

    end
};

#define COMMAND_DEFAULT_MEMBERS \
    CommandEncoding encoding

union CommandEncodingData {
    struct Draw {
        COMMAND_DEFAULT_MEMBERS;
        Uint32 count;
        Uint32 start;
    } draw;

    struct DrawIndexed {
        COMMAND_DEFAULT_MEMBERS;
        Uint32 count;
        Uint32 start;
        Int32 base;
    } draw_indexed;

    struct BindBuffer {
        COMMAND_DEFAULT_MEMBERS;
        kai::RenderBuffer *buffer;
        kai::RenderBufferType type;
        kai::ShaderType shader_type;
    } bind_buffer;
};

#undef COMMAND_DEFAULT_MEMBERS

#endif /* KAI_RENDER_INTERNAL_H */
