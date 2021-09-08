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

    enum class RenderResourceUsage {
        gpu_r,
        gpu_rw,
        cpu_w_gpu_r,
        cpu_rw_gpu_rw,
    };

    enum class RenderCPUUsage {
        none,
        read,
        write,
        read_write,
    };

    enum class RenderBufferType {
        vertex,
        index,
        constant
    };

    struct RenderBufferInfo {
        const void *data;
        size_t byte_size;

        Uint32 stride;

        RenderBufferType type;
        RenderCPUUsage cpu_usage;
        RenderResourceUsage resource_usage;
    };

    struct RenderBuffer {
        void *data;
        Uint32 stride;
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

        Bool32 color_enable;
        Float32 color_clear_values[4];

        Bool32 depth_enable;
        Float32 depth_clear_value;

        Bool32 stencil_enable;
        Uint32 stencil_clear_value;

        Bool32 front_ccw;
    };

    struct RenderPipeline {
        void *data;
        VertexShaderID vertex_shader;
        PixelShaderID pixel_shader;
    };

    struct CommandBuffer {
        KAI_API explicit CommandBuffer(void) = default;
        KAI_API explicit CommandBuffer(Uint32 command_count);

        void KAI_API destroy(void);

        void KAI_API begin(void);
        void KAI_API end(void);

        void KAI_API draw(Uint32 vertex_count, Uint32 starting_index = 0);

        void KAI_API bind_buffer(RenderBuffer &buffer, RenderBufferType type,
                                 ShaderType shader_type = ShaderType::vertex);

        void KAI_API clear_color(void);
        void KAI_API clear_depth(void);
        void KAI_API clear_stencil(void);
        void KAI_API clear_depth_stencil(void);

        const void * get_data(void) const {
            return allocator.get_data();
        }

    private:
        kai::StackAllocator allocator;
    };

    // Abstraction for both the GPU and rendering API
    struct RenderDevice {
        static KAI_API RenderDevice * init_device(void);
        static KAI_API RenderDevice * init_device(Uint32 id);

        virtual void destroy(void) = 0;

        virtual void execute(const CommandBuffer &command_buffer) const = 0;
        virtual void present(void) const = 0;

        // A width/height of 0 simply means that it'll use the window's width/height
        virtual void set_viewport(Int32 x, Int32 y, Uint32 width = 0, Uint32 height = 0) const = 0;

        virtual bool compile_shader(const char *shader_stream, ShaderType type,
                                    const char *entry, void *out_id, void **bytecode = nullptr) const = 0;

        virtual bool create_render_pipeline(const RenderPipelineInfo &info, const RenderInputLayoutInfo *input_layouts,
                                            Uint32 input_layout_count, RenderPipeline &out_pipeline) const = 0;

        virtual void destroy_render_pipeline(RenderPipeline &pipeline) = 0;

        virtual void set_render_pipeline(const RenderPipeline &pipeline) const = 0;

        virtual bool create_buffer(const RenderBufferInfo &info, RenderBuffer &out_buffer) const = 0;

        virtual void destroy_buffer(RenderBuffer &buffer) = 0;

        void *data;
        Uint32 id;
        RenderingBackend backend;
        char name[128] = {}; // TODO: Change to UTF-8 string once that is implemented
    };

    KAI_API Window * get_window(void);
}

#endif /* KAI_RENDER_H */
