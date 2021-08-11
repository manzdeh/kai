/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_INPUT_H
#define KAI_INPUT_H

#include "math.h"
#include "types.h"
#include "utils.h"

#define KAI_MAX_GAMEPADS 4

namespace kai {
    enum class Key : Uint32 {
        none = 0,
        a,
        b,
        c,
        d,
        e,
        f,
        g,
        h,
        i,
        j,
        k,
        l,
        m,
        n,
        o,
        p,
        q,
        r,
        s,
        t,
        u,
        v,
        w,
        x,
        y,
        z,
        one,
        two,
        three,
        four,
        five,
        six,
        seven,
        eight,
        nine,
        zero,
        enter,
        escape,
        backspace,
        tab,
        space,
        minus,
        equals,
        left_bracket,
        right_bracket,
        backslash,
        semicolon,
        apostrophe,
        tilde,
        comma,
        period,
        slash,
        caps_lock,
        f1,
        f2,
        f3,
        f4,
        f5,
        f6,
        f7,
        f8,
        f9,
        f10,
        f11,
        f12,
        print_screen,
        scroll_lock,
        pause,
        insert,
        home,
        page_up,
        del,
        end,
        page_down,
        right,
        left,
        down,
        up,
        num_lock,
        num_divide,
        num_multiply,
        num_minus,
        num_plus,
        num_enter,
        num_1,
        num_2,
        num_3,
        num_4,
        num_5,
        num_6,
        num_7,
        num_8,
        num_9,
        num_0,
        num_period,
        left_control,
        left_shift,
        left_alt,
        left_gui,
        right_control,
        right_shift,
        right_alt,
        right_gui,

        count
    };

    KAI_API bool key_down(Key key);
    KAI_API bool key_up(Key key);
    KAI_API bool key_press(Key key);
    KAI_API bool key_release(Key key);

    enum class MouseButton : Uint32 {
        left,
        middle,
        right,
        x1,
        x2,

        count
    };

    KAI_API bool mouse_down(MouseButton button);
    KAI_API bool mouse_up(MouseButton button);
    KAI_API bool mouse_click(MouseButton button);
    KAI_API bool mouse_release(MouseButton button);
    KAI_API void get_rel_mouse_pos(Int32 &x, Int32 &y);
    KAI_API Int32 get_scroll_delta(void);

    enum class GamepadButton : Uint32 {
        a,
        b,
        x,
        y,
        start,
        back,
        left_bumper,
        left_trigger,
        right_bumper,
        right_trigger,
        left_stick,
        right_stick,
        dpad_up,
        dpad_down,
        dpad_left,
        dpad_right
    };

    enum class RumbleMotor : Uint32 {
        left,
        right,
        both
    };

    KAI_API bool gamepad_down(GamepadButton button, Uint32 controller = 0);
    KAI_API bool gamepad_up(GamepadButton button, Uint32 controller = 0);
    KAI_API bool gamepad_press(GamepadButton button, Uint32 controller = 0);
    KAI_API bool gamepad_release(GamepadButton button, Uint32 controller = 0);
    KAI_API kai::Vec2 gamepad_left_stick_pos(Uint32 controller = 0);
    KAI_API kai::Vec2 gamepad_right_stick_pos(Uint32 controller = 0);
    KAI_API Float32 gamepad_left_trigger(Uint32 controller = 0);
    KAI_API Float32 gamepad_right_trigger(Uint32 controller = 0);
    KAI_API void gamepad_set_rumble_intensity(Float32 value, RumbleMotor motor, Uint32 controller = 0);
}

#endif /* KAI_INPUT_H */
