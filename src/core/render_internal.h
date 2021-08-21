/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_RENDER_INTERNAL_H
#define KAI_RENDER_INTERNAL_H

#include "includes/render.h"
#include "includes/types.h"

void init_renderer(kai::RenderDevice &device, kai::RenderingBackend backend);

#define DEFINE_RENDERER_BACKEND(backend_name) struct backend_name : public kai::Renderer { \
    void init_default_device(kai::RenderDevice &out_device) override; \
    void init_device(kai::RenderDevice &out_device, Uint32 id) override; \
    void set_viewport(Int32 x, Int32 y, Uint32 width = 0, Uint32 height = 0) override; \
    bool compile_shader(const char *shader_stream, kai::ShaderType type, \
                        const char *entry, void *out_id, void **blob = nullptr) const override; \
    bool create_render_pipeline(const kai::RenderPipelineInfo &info, const kai::RenderInputLayoutInfo *input_layouts, \
                                Uint32 input_layout_count, kai::RenderPipeline &out_pipeline); \
}

#endif /* KAI_RENDER_INTERNAL_H */
