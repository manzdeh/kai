/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "includes/keyboard.h"
#include "keyboard_internal.h"

static struct {
    Uint8 *keys;
    Uint32 index;
    Uint8 key_buffers[2][static_cast<Uint32>(kai::Key::count)];
} keyboard_manager;

void init_keyboard(void) {
    memset(keyboard_manager.key_buffers, 0, sizeof(keyboard_manager.key_buffers));
    keyboard_manager.index = 0;
    keyboard_manager.keys = (Uint8 *)&keyboard_manager.key_buffers[keyboard_manager.index];
}

void swap_keyboard_buffers(void) {
    keyboard_manager.index = !keyboard_manager.index;
    keyboard_manager.keys = (Uint8 *)&keyboard_manager.key_buffers[keyboard_manager.index];
}

bool kai::key_down(Key key) {
    return keyboard_manager.keys[static_cast<Uint32>(key)] != 0;
}

bool kai::key_up(Key key) {
    return keyboard_manager.keys[static_cast<Uint32>(key)] == 0;
}

bool kai::key_press(Key key) {
    Uint32 k = static_cast<Uint32>(key);
    return keyboard_manager.key_buffers[keyboard_manager.index][k] != 0 &&
        keyboard_manager.key_buffers[!keyboard_manager.index][k] == 0;
}

bool kai::key_release(Key key) {
    Uint32 k = static_cast<Uint32>(key);
    return keyboard_manager.key_buffers[keyboard_manager.index][k] == 0 &&
        keyboard_manager.key_buffers[!keyboard_manager.index][k] != 0;
}
