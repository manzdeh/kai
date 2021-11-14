/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_PLATFORM_H
#define KAI_PLATFORM_H

#include "../core/includes/kai.h"
#include "../core/render_internal.h"

void * platform_alloc_mem_arena(size_t bytes, void *address = nullptr);
void platform_free_mem_arena(void *arena);

void platform_get_rel_mouse_pos(Int32 &x, Int32 &y);

void platform_set_rumble_intensity(Float32 left_motor, Float32 right_motor, Uint32 controller);

bool platform_setup_game_callbacks(kai::GameCallbacks &callbacks);

kai::Window * platform_get_kai_window(void);

void platform_renderer_init_backend(kai::RenderingBackend backend);
void platform_renderer_destroy_backend(void);
kai::RenderDevice * platform_renderer_init_device(kai::StackAllocator &allocator);
kai::RenderDevice * platform_renderer_init_device(kai::StackAllocator &allocator, Uint32 id);

#endif /* KAI_PLATFORM_H */
