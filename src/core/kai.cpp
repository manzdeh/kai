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

static KaiLogProc log_func = nullptr;

void init_engine(void) {
    MemoryManager::init(kai::gibibytes(4));
    init_input();

    kai::RenderDevice device;
    init_renderer(device, kai::RenderingBackend::dx11);
    kai::init_default_device(device);

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
    return true; // TODO: Always returns true for now. Eventually we need to check if the game has sent a quit request
}

void destroy_engine(void) {
    game_manager.callbacks.destroy();
    MemoryManager::destroy();
}

void set_log_callback(KaiLogProc func) {
    log_func = func;
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
