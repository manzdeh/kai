/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_WIN32_DX11_H
#define KAI_WIN32_DX11_H

#include <windows.h>

#include "../../core/includes/kai.h"

void init_dx11(void);
void destroy_dx11(void);

struct DX11Renderer : public kai::RenderDevice {
    DX11Renderer() = default;

    void set_viewport(Int32 x, Int32 y, Uint32 width = 0, Uint32 height = 0) const override;

    bool compile_shader(const char *shader_stream, kai::ShaderType type,
                        const char *entry, void *out_id, void **bytecode = nullptr) const override;

    bool create_render_pipeline(const kai::RenderPipelineInfo &info, const kai::RenderInputLayoutInfo *input_layouts,
                                Uint32 input_layout_count, kai::RenderPipeline &out_pipeline) const override;

    void destroy_render_pipeline(kai::RenderPipeline &pipeline) override;

    void set_render_pipeline(const kai::RenderPipeline &pipeline) const override;
};

#endif /* KAI_WIN32_DX11_H */
