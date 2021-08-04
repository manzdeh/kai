/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_INPUT_INTERNAL_H
#define KAI_INPUT_INTERNAL_H

#include "includes/input.h"

void init_input(void);
void swap_input_buffers(void);

void set_key(kai::Key key, bool value);

void set_mouse_button(kai::MouseButton button, bool value, bool double_click = false);
void set_scroll_delta(Int32 delta);

#endif /* KAI_INPUT_INTERNAL_H */
