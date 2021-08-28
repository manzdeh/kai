/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include <string.h>

#include "includes/render.h"
#include "render_internal.h"

void init_renderer(kai::RenderingBackend backend) {
#ifndef KAI_PLATFORM_WIN32
    if(backend == kai::RenderingBackend::dx11) {
        kai::log("Direct3D11 is not supported on this platform!\n");
        return;
    }
#endif

    platform_renderer_init_backend(backend);
}

void destroy_renderer(void) {
    platform_renderer_destroy_backend();
}

// -------------------------------------------------- CommandBuffer -------------------------------------------------- //
kai::CommandBuffer::CommandBuffer(Uint32 command_count) {
    // TODO: The StackAllocator should probably be extended to allow reallocs if specified

    // We add one to accommodate for the CommandEncoding::end that needs to be appended to the CommandBuffer
    allocator = kai::StackAllocator((command_count + 1) * sizeof(CommandEncodingData));
}

void kai::CommandBuffer::begin(void) {
    allocator.clear();
}

static KAI_FORCEINLINE void push_command(kai::StackAllocator &allocator, CommandEncoding encoding) {
    *(static_cast<CommandEncoding *>(allocator.alloc(sizeof(CommandEncoding)))) = encoding;
}

void kai::CommandBuffer::end(void) {
    push_command(allocator, CommandEncoding::end);
}

#define PUSH_TO_COMMAND_BUFFER(var) \
    do { \
        void *data = allocator.alloc(sizeof(var)); \
        memcpy(data, &var, sizeof(var)); \
    } while(0)

void kai::CommandBuffer::draw(Uint32 vertex_count, Uint32 starting_index) {
    CommandEncodingData::Draw command = {
        CommandEncoding::draw,
        vertex_count,
        starting_index
    };

    PUSH_TO_COMMAND_BUFFER(command);
}

void kai::CommandBuffer::bind_buffer(kai::RenderBuffer &buffer) {
    CommandEncodingData::BindBuffer command = {
        CommandEncoding::bind_buffer,
        &buffer
    };

    PUSH_TO_COMMAND_BUFFER(command);
}

void kai::CommandBuffer::clear_color(void) { push_command(allocator, CommandEncoding::clear_color); }
void kai::CommandBuffer::clear_depth(void) { push_command(allocator, CommandEncoding::clear_depth); }
void kai::CommandBuffer::clear_stencil(void) { push_command(allocator, CommandEncoding::clear_stencil); }
void kai::CommandBuffer::clear_depth_stencil(void) { push_command(allocator, CommandEncoding::clear_depth_stencil); }

#undef PUSH_TO_COMMAND_BUFFER

// -------------------------------------------------- RenderDevice  -------------------------------------------------- //
kai::RenderDevice * kai::RenderDevice::init_device(void) {
    return  platform_renderer_init_device(*get_engine_memory());
}

kai::RenderDevice * kai::RenderDevice::init_device(Uint32 id) {
    return platform_renderer_init_device(*get_engine_memory(), id);
}

kai::Window * kai::get_window(void) {
    return platform_get_kai_window();
}
