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

void set_mouse_button(kai::MouseButton button, bool value);
void set_scroll_delta(Int32 delta);

void set_gamepad_button(Uint32 controller, kai::GamepadButton button, bool value);
void set_gamepad_analog_axis(Uint32 controller, kai::GamepadButton analog_stick, kai::Vec2 value);
void set_gamepad_trigger_analog(Uint32 controller, kai::GamepadButton analog, Float32 value);

#endif /* KAI_INPUT_INTERNAL_H */
