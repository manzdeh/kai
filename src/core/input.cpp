/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "includes/input.h"
#include "input_internal.h"

#include "../platform/platform.h"

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

/******************** Internal ********************/
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

    mouse_manager.delta = 0;
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

#define INPUT_DOWN_IMPL(buf, val) return buf[static_cast<Uint32>(val)] != 0
#define INPUT_UP_IMPL(buf, val) return buf[static_cast<Uint32>(val)] == 0
#define INPUT_PRESS_IMPL(front, back, val) \
    Uint32 v = static_cast<Uint32>(val); \
    return front[v] != 0 && back[v] == 0
#define INPUT_RELEASE_IMPL(front, back, val) \
    Uint32 v = static_cast<Uint32>(val); \
    return front[v] == 0 && back[v] != 0

/******************** Keyboard ********************/
bool kai::key_down(Key key) {
    INPUT_DOWN_IMPL(keyboard_manager.keys, key);
}

bool kai::key_up(Key key) {
    INPUT_UP_IMPL(keyboard_manager.keys, key);
}

bool kai::key_press(Key key) {
    INPUT_PRESS_IMPL(keyboard_manager.keys,
                     reinterpret_cast<Uint8 *>(&keyboard_manager.buffers[!keyboard_manager.index]),
                     key);
}

bool kai::key_release(Key key) {
    INPUT_RELEASE_IMPL(keyboard_manager.keys,
                       reinterpret_cast<Uint8 *>(&keyboard_manager.buffers[!keyboard_manager.index]),
                       key);
}

/******************** Mouse ********************/
bool kai::mouse_down(MouseButton button) {
    INPUT_DOWN_IMPL(mouse_manager.mouse, button);
}

bool kai::mouse_up(MouseButton button) {
    INPUT_UP_IMPL(mouse_manager.mouse, button);
}

bool kai::mouse_click(MouseButton button) {
    INPUT_PRESS_IMPL(mouse_manager.mouse,
                     reinterpret_cast<Uint8 *>(&mouse_manager.buffers[!mouse_manager.index]),
                     button);
}

bool kai::mouse_release(MouseButton button) {
    INPUT_RELEASE_IMPL(mouse_manager.mouse,
                       reinterpret_cast<Uint8 *>(&mouse_manager.buffers[!mouse_manager.index]),
                       button);
}

bool kai::mouse_double_click(MouseButton button) {
    return mouse_manager.mouse[static_cast<Uint8>(button)] == 0x3; // First two bits are set for double-click
}

void kai::get_rel_mouse_pos(Int32 &x, Int32 &y) {
    platform_get_rel_mouse_pos(x, y);
}

Int32 kai::get_scroll_delta(void) {
    return mouse_manager.delta;
}

#undef INPUT_DOWN_IMPL
#undef INPUT_UP_IMPL
#undef INPUT_PRESS_IMPL
#undef INPUT_RELEASE_IMPL
