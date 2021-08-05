/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_PLATFORM_H
#define KAI_PLATFORM_H

void * platform_alloc_mem_arena(size_t bytes, void *address = nullptr);
void platform_free_mem_arena(void *arena);

size_t platform_get_page_size(void);

void platform_get_rel_mouse_pos(Int32 &x, Int32 &y);

#endif /* KAI_PLATFORM_H */
