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

struct Gamepad {
    kai::Vec2 left_analog;
    kai::Vec2 right_analog;

    Float32 rumble;

    Float32 left_trigger_analog;
    Float32 right_trigger_analog;

    // Gamepad state that is double-buffered
    struct {
        union {
            struct {
                Uint32 a: 1;
                Uint32 b: 1;
                Uint32 x: 1;
                Uint32 y: 1;
                Uint32 start: 1;
                Uint32 back: 1;
                Uint32 left_bumper: 1;
                Uint32 left_trigger: 1;
                Uint32 right_bumper: 1;
                Uint32 right_trigger: 1;
                Uint32 left_stick: 1;
                Uint32 right_stick: 1;
                Uint32 dpad_up: 1;
                Uint32 dpad_down: 1;
                Uint32 dpad_left: 1;
                Uint32 dpad_right: 1;
            };
            Uint32 buttons;
        };
    } buffers[2];
};

static struct {
    Gamepad gamepads[KAI_MAX_GAMEPADS];
    Uint32 index;
} gamepad_manager;

/******************** Internal ********************/
void init_input(void) {
    memset(&keyboard_manager, 0, sizeof(keyboard_manager));
    memset(&mouse_manager, 0, sizeof(mouse_manager));
    memset(&gamepad_manager, 0, sizeof(gamepad_manager));

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

    gamepad_manager.index = !gamepad_manager.index;
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

static KAI_FORCEINLINE bool is_controller_valid(Uint32 id) {
    return id >= 0 && id < KAI_MAX_GAMEPADS;
}

#define GAMEPAD_BUTTON_SWITCH_IMPL(button) \
    switch(button) { \
        case GamepadButton::a: GAMEPAD_FUNC_IMPL(a); \
        case GamepadButton::b: GAMEPAD_FUNC_IMPL(b); \
        case GamepadButton::x: GAMEPAD_FUNC_IMPL(x); \
        case GamepadButton::y: GAMEPAD_FUNC_IMPL(y); \
        case GamepadButton::start: GAMEPAD_FUNC_IMPL(start); \
        case GamepadButton::back: GAMEPAD_FUNC_IMPL(back); \
        case GamepadButton::left_bumper: GAMEPAD_FUNC_IMPL(left_bumper); \
        case GamepadButton::left_trigger: GAMEPAD_FUNC_IMPL(left_trigger); \
        case GamepadButton::right_bumper: GAMEPAD_FUNC_IMPL(right_bumper); \
        case GamepadButton::right_trigger: GAMEPAD_FUNC_IMPL(right_trigger); \
        case GamepadButton::left_stick: GAMEPAD_FUNC_IMPL(left_stick); \
        case GamepadButton::right_stick: GAMEPAD_FUNC_IMPL(right_stick); \
        case GamepadButton::dpad_up: GAMEPAD_FUNC_IMPL(dpad_up); \
        case GamepadButton::dpad_down: GAMEPAD_FUNC_IMPL(dpad_down); \
        case GamepadButton::dpad_left: GAMEPAD_FUNC_IMPL(dpad_left); \
        case GamepadButton::dpad_right: GAMEPAD_FUNC_IMPL(dpad_right); \
        default: \
            return false; \
    }

bool kai::gamepad_down(GamepadButton button, Uint32 controller) {
#define GAMEPAD_FUNC_IMPL(btn) return gamepad_manager.gamepads[controller].buffers[gamepad_manager.index].btn != 0

    KAI_ASSERT(is_controller_valid(controller));
    GAMEPAD_BUTTON_SWITCH_IMPL(button);

#undef GAMEPAD_FUNC_IMPL
}

bool kai::gamepad_up(GamepadButton button, Uint32 controller) {
#define GAMEPAD_FUNC_IMPL(btn) return gamepad_manager.gamepads[controller].buffers[gamepad_manager.index].btn == 0

    KAI_ASSERT(is_controller_valid(controller));
    GAMEPAD_BUTTON_SWITCH_IMPL(button);

#undef GAMEPAD_FUNC_IMPL
}

bool kai::gamepad_press(GamepadButton button, Uint32 controller) {
#define GAMEPAD_FUNC_IMPL(btn) return gamepad_manager.gamepads[controller].buffers[gamepad_manager.index].btn != 0 && \
    gamepad_manager.gamepads[controller].buffers[!gamepad_manager.index].btn == 0

    KAI_ASSERT(is_controller_valid(controller));
    GAMEPAD_BUTTON_SWITCH_IMPL(button);

#undef GAMEPAD_FUNC_IMPL
}

bool kai::gamepad_release(GamepadButton button, Uint32 controller) {
#define GAMEPAD_FUNC_IMPL(btn) return gamepad_manager.gamepads[controller].buffers[gamepad_manager.index].btn == 0 && \
    gamepad_manager.gamepads[controller].buffers[!gamepad_manager.index].btn != 0

    KAI_ASSERT(is_controller_valid(controller));
    GAMEPAD_BUTTON_SWITCH_IMPL(button);

#undef GAMEPAD_FUNC_IMPL
}

kai::Vec2 kai::gamepad_get_left_stick_pos(Uint32 controller) {
    KAI_ASSERT(is_controller_valid(controller));
    return gamepad_manager.gamepads[controller].left_analog;
}

kai::Vec2 kai::gamepad_get_right_stick_pos(Uint32 controller) {
    KAI_ASSERT(is_controller_valid(controller));
    return gamepad_manager.gamepads[controller].right_analog;
}

Float32 kai::gamepad_left_trigger(Uint32 controller) {
    KAI_ASSERT(is_controller_valid(controller));
    return gamepad_manager.gamepads[controller].left_trigger_analog;
}

Float32 kai::gamepad_right_trigger(Uint32 controller) {
    KAI_ASSERT(is_controller_valid(controller));
    return gamepad_manager.gamepads[controller].right_trigger_analog;
}

void kai::gamepad_set_rumble_intensity(Float32 value, Uint32 controller) {
    KAI_ASSERT(is_controller_valid(controller));
    kai::clamp(value, 0.0f, 1.0f);
    gamepad_manager.gamepads[controller].rumble = value;
}

#undef GAMEPAD_BUTTON_SWITCH_IMPL

#undef INPUT_DOWN_IMPL
#undef INPUT_UP_IMPL
#undef INPUT_PRESS_IMPL
#undef INPUT_RELEASE_IMPL
