/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_RENDER_H
#define KAI_RENDER_H

#include "types.h"

namespace kai {
    typedef uintptr_t VertexShaderID;
    typedef uintptr_t PixelShaderID;

    enum class ShaderType {
        vertex,
        pixel
    };

    enum class RenderFormat {
        unknown,
        r_u8,
        rg_u8,
        rgba_u8,
        r_f32,
        rg_f32,
        rgb_f32,
        rgba_f32
    };

    struct Window {
        void *platform_window;
        Uint32 width;
        Uint32 height;
    };

    enum class RenderingBackend {
        unknown,
        dx11,
    };

#define KAI_INPUT_LAYOUT_APPEND 0xffffffff

    struct RenderInputLayoutInfo {
        const char *name;
        Uint32 index;
        RenderFormat format;
        Uint32 offset;
    };

    struct RenderPipelineInfo {
        const char *vertex_shader_source;
        const char *vertex_shader_entry;
        const char *pixel_shader_source;
        const char *pixel_shader_entry;

        enum class FillMode {
            solid,
            wireframe
        } fill_mode;

        enum class CullMode {
            none,
            front,
            back
        } cull_mode;

        enum class TopologyType {
            undefined,
            point_list,
            line_list,
            line_strip,
            triangle_list,
            triangle_strip
        } topology;

        Bool32 front_ccw;
    };

    struct RenderPipeline {
        void *state;
        VertexShaderID vertex_shader;
        PixelShaderID pixel_shader;
    };

    // Abstraction for both the GPU and rendering API
    struct RenderDevice {
        static RenderDevice * init_device(void);
        static RenderDevice * init_device(Uint32 id);

        // A width/height of 0 simply means that it'll use the window's width/height
        virtual void set_viewport(Int32 x, Int32 y, Uint32 width = 0, Uint32 height = 0) = 0;

        virtual bool compile_shader(const char *shader_stream, ShaderType type,
                                    const char *entry, void *out_id, void **bytecode = nullptr) const = 0;

        virtual bool create_render_pipeline(const RenderPipelineInfo &info, const RenderInputLayoutInfo *input_layouts,
                                            Uint32 input_layout_count, RenderPipeline &out_pipeline) = 0;

        Uint32 id;
        RenderingBackend backend;
        char name[128] = {}; // TODO: Change to UTF-8 string once that is implemented
    };

    KAI_API Window * get_window(void);
}

#endif /* KAI_RENDER_H */
