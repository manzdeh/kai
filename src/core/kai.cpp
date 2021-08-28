/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include <stdio.h>

#include "includes/kai.h"
#include "kai_internal.h"

#include "alloc.cpp"
#include "input.cpp"
#include "render.cpp"

#define STUB_NAME_HELPER(name) name##_stub
#define STUB_NAME(name) STUB_NAME_HELPER(name)
#define DEF_STUB_FUNC(name) KAI_GAME_CALLBACK_FUNC(STUB_NAME(name))

static DEF_STUB_FUNC(KAI_GAME_INIT_PROCNAME) {}
static DEF_STUB_FUNC(KAI_GAME_UPDATE_PROCNAME) {}
static DEF_STUB_FUNC(KAI_GAME_DESTROY_PROCNAME) {}

static struct {
    kai::GameCallbacks callbacks;
} game_manager = {
    // NOTE: The order needs to be the same as in the GameCallbacks struct.
    // C99's designated initializers are available in C++20, but the codebase
    // won't target that (maybe at a later date)
    STUB_NAME(KAI_GAME_INIT_PROCNAME),
    STUB_NAME(KAI_GAME_UPDATE_PROCNAME),
    STUB_NAME(KAI_GAME_DESTROY_PROCNAME)
};

#undef DEF_STUB_FUNC
#undef STUB_NAME
#undef STUB_NAME_HELPER

static kai::StackAllocator engine_memory;
static KaiLogProc log_func = nullptr;

static kai::RenderDevice *test_device;
static kai::CommandBuffer cmd_buffer;
static kai::RenderBuffer render_buffer;

void init_engine(void) {
    MemoryManager::init(kai::gibibytes(4));
    engine_memory = kai::StackAllocator(static_cast<Uint32>(kai::mebibytes(64)));
    init_input();

    init_renderer(kai::RenderingBackend::dx11);

    {
        // TODO: Temporary!
        test_device = kai::RenderDevice::init_device(); // TODO: This should be done by the game

        static const Float32 triangle[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f,  0.5f, 0.0f
        };

        kai::RenderBufferInfo info = {};
        info.data = triangle;
        info.byte_size = sizeof(triangle);
        info.stride = sizeof(Float32) * 3;
        info.type = kai::RenderBufferInfo::Type::vertex_buffer;
        info.resource_usage = kai::RenderResourceUsage::gpu_rw;

        test_device->create_buffer(info, render_buffer);

        cmd_buffer = kai::CommandBuffer(10);
        cmd_buffer.begin();
        cmd_buffer.bind_buffer(render_buffer);
        cmd_buffer.clear_color();
        cmd_buffer.clear_depth_stencil();
        cmd_buffer.draw(3);
        cmd_buffer.end();
    }

    // TODO: Log an error if the required callbacks haven't been found on the game's side
    platform_setup_game_callbacks(game_manager.callbacks);

    game_manager.callbacks.init();
}

bool tick_engine(void) {
    game_manager.callbacks.update();

#ifdef KAI_DEBUG
    if(kai::key_down(kai::Key::escape)) {
        return false;
    }
#endif

    swap_input_buffers();

    test_device->execute(cmd_buffer);
    test_device->present();

    return true; // TODO: Always returns true for now. Eventually we need to check if the game has sent a quit request
}

void destroy_engine(void) {
    game_manager.callbacks.destroy();
    destroy_renderer();
    engine_memory.destroy();
    MemoryManager::destroy();
}

void set_log_callback(KaiLogProc func) {
    log_func = func;
}

kai::StackAllocator * get_engine_memory(void) {
    return &engine_memory;
}

static void log_impl(const char *str, va_list vlist) {
    static char buf[999];
    vsnprintf(buf, sizeof(buf), str, vlist);
    printf(buf);
}

void kai::log(const char *str, ...) {
    va_list vlist;
    va_start(vlist, str);

    log_func ? log_func(str, vlist) :
        log_impl(str, vlist);

    va_end(vlist);
}
