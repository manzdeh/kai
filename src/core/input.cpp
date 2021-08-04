/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "includes/input.h"
#include "input_internal.h"

static struct {
    Uint8 *keys;
    Uint32 index;
    Uint8 buffers[2][static_cast<Uint32>(kai::Key::count)];
} keyboard_manager;

static struct {
    Uint8 *mouse;
    Int32 delta;
    Uint32 index;
    Uint8 buffers[2][static_cast<Uint32>(kai::MouseButton::count)];
} mouse_manager;

void init_input(void) {
    memset(&keyboard_manager, 0, sizeof(keyboard_manager));
    memset(&mouse_manager, 0, sizeof(mouse_manager));

    keyboard_manager.keys = reinterpret_cast<Uint8 *>(&keyboard_manager.buffers[keyboard_manager.index]);

    mouse_manager.mouse = reinterpret_cast<Uint8 *>(&mouse_manager.buffers[mouse_manager.index]);
}

void swap_input_buffers(void) {
    keyboard_manager.index = !keyboard_manager.index;
    keyboard_manager.keys = reinterpret_cast<Uint8 *>(&keyboard_manager.buffers[keyboard_manager.index]);
    memset(keyboard_manager.keys, 0, sizeof(*keyboard_manager.keys) * static_cast<Uint32>(kai::Key::count));

    mouse_manager.index = !mouse_manager.index;
    mouse_manager.mouse = reinterpret_cast<Uint8 *>(&mouse_manager.buffers[mouse_manager.index]);
    memset(mouse_manager.mouse, 0, sizeof(*mouse_manager.mouse) * static_cast<Uint32>(kai::MouseButton::count));
}

void set_key(kai::Key key, bool value) {
    keyboard_manager.keys[static_cast<Uint32>(key)] = value;
}

void set_mouse_button(kai::MouseButton button, bool value, bool double_click) {
    Uint8 v = static_cast<Uint8>(value) | (static_cast<Uint8>(double_click) << 1);
    mouse_manager.mouse[static_cast<Uint32>(button)] = v;
}

void set_scroll_delta(Int32 delta) {
    mouse_manager.delta = delta;
}

bool kai::key_down(Key key) {
    return keyboard_manager.keys[static_cast<Uint32>(key)] != 0;
}

bool kai::key_up(Key key) {
    return keyboard_manager.keys[static_cast<Uint32>(key)] == 0;
}

bool kai::key_press(Key key) {
    Uint32 k = static_cast<Uint32>(key);
    return keyboard_manager.buffers[keyboard_manager.index][k] != 0 &&
        keyboard_manager.buffers[!keyboard_manager.index][k] == 0;
}

bool kai::key_release(Key key) {
    Uint32 k = static_cast<Uint32>(key);
    return keyboard_manager.buffers[keyboard_manager.index][k] == 0 &&
        keyboard_manager.buffers[!keyboard_manager.index][k] != 0;
}
