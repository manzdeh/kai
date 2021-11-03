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
        const void *data = nullptr;
        size_t byte_size = 0;

        Uint32 stride = 0;

        RenderBufferType type;
        RenderCPUUsage cpu_usage = RenderCPUUsage::none;
        RenderResourceUsage resource_usage = RenderResourceUsage::gpu_r;
    };

    struct RenderBuffer {
        void *data = nullptr;
        Uint32 stride = 0;
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
        } fill_mode = FillMode::solid;

        enum class CullMode {
            none,
            front,
            back
        } cull_mode = CullMode::back;

        enum class TopologyType {
            undefined,
            point_list,
            line_list,
            line_strip,
            triangle_list,
            triangle_strip
        } topology = TopologyType::triangle_list;

        Bool32 color_enable = true;
        Float32 color_clear_values[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        Bool32 depth_enable = false;
        Float32 depth_clear_value = 0.0f;

        Bool32 stencil_enable = false;
        Uint32 stencil_clear_value = 0;

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

        KAI_API void destroy(void);

        KAI_API void begin(void);
        KAI_API void end(void);

        KAI_API void draw(Uint32 vertex_count, Uint32 starting_index = 0);

        KAI_API void bind_buffer(RenderBuffer &buffer, RenderBufferType type,
                                 ShaderType shader_type = ShaderType::vertex);

        KAI_API void clear_color(void);
        KAI_API void clear_depth(void);
        KAI_API void clear_stencil(void);
        KAI_API void clear_depth_stencil(void);

        const void * get_data(void) const {
            return allocator.get_data();
        }

    private:
        kai::StackAllocator allocator;
    };

    // Abstraction for both the GPU and rendering API
    struct RenderDevice {
        KAI_API static RenderDevice * get(void);

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
