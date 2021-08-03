/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "win32_input.h"

// TODO: Test this and fill out missing values
kai::Key win32_get_kai_key_from_scancode(UINT win_scancode, bool extended) {
#define MAP_TO_KAI_KEY(value, kai_key) case value: return kai::Key::kai_key

    if(!extended) {
        switch(win_scancode) {
            MAP_TO_KAI_KEY(0x1e, a);
            MAP_TO_KAI_KEY(0x30, b);
            MAP_TO_KAI_KEY(0x2e, c);
            MAP_TO_KAI_KEY(0x20, d);
            MAP_TO_KAI_KEY(0x12, e);
            MAP_TO_KAI_KEY(0x21, f);
            MAP_TO_KAI_KEY(0x22, g);
            MAP_TO_KAI_KEY(0x23, h);
            MAP_TO_KAI_KEY(0x17, i);
            MAP_TO_KAI_KEY(0x24, j);
            MAP_TO_KAI_KEY(0x25, k);
            MAP_TO_KAI_KEY(0x26, l);
            MAP_TO_KAI_KEY(0x32, m);
            MAP_TO_KAI_KEY(0x31, n);
            MAP_TO_KAI_KEY(0x18, o);
            MAP_TO_KAI_KEY(0x19, p);
            MAP_TO_KAI_KEY(0x10, q);
            MAP_TO_KAI_KEY(0x13, r);
            MAP_TO_KAI_KEY(0x1f, s);
            MAP_TO_KAI_KEY(0x14, t);
            MAP_TO_KAI_KEY(0x16, u);
            MAP_TO_KAI_KEY(0x2f, v);
            MAP_TO_KAI_KEY(0x11, w);
            MAP_TO_KAI_KEY(0x2d, x);
            MAP_TO_KAI_KEY(0x15, y);
            MAP_TO_KAI_KEY(0x2c, z);
            MAP_TO_KAI_KEY(0x02, one);
            MAP_TO_KAI_KEY(0x03, two);
            MAP_TO_KAI_KEY(0x04, three);
            MAP_TO_KAI_KEY(0x05, four);
            MAP_TO_KAI_KEY(0x06, five);
            MAP_TO_KAI_KEY(0x07, six);
            MAP_TO_KAI_KEY(0x08, seven);
            MAP_TO_KAI_KEY(0x09, eight);
            MAP_TO_KAI_KEY(0x0a, nine);
            MAP_TO_KAI_KEY(0x0b, zero);
            MAP_TO_KAI_KEY(0x1c, enter);
            MAP_TO_KAI_KEY(0x01, escape);
            MAP_TO_KAI_KEY(0x0e, backspace);
            MAP_TO_KAI_KEY(0x0f, tab);
            MAP_TO_KAI_KEY(0x39, space);
            MAP_TO_KAI_KEY(0x0c, minus);
            MAP_TO_KAI_KEY(0x0d, equals);
            MAP_TO_KAI_KEY(0x1a, left_bracket);
            MAP_TO_KAI_KEY(0x1b, right_bracket);
            MAP_TO_KAI_KEY(0x2b, backslash);
            MAP_TO_KAI_KEY(0x27, semicolon);
            MAP_TO_KAI_KEY(0x28, apostrophe);
            MAP_TO_KAI_KEY(0x29, tilde);
            MAP_TO_KAI_KEY(0x33, comma);
            MAP_TO_KAI_KEY(0x34, period);
            MAP_TO_KAI_KEY(0x35, slash);
            MAP_TO_KAI_KEY(0x3a, caps_lock);
            MAP_TO_KAI_KEY(0x3b, f1);
            MAP_TO_KAI_KEY(0x3c, f2);
            MAP_TO_KAI_KEY(0x3d, f3);
            MAP_TO_KAI_KEY(0x3e, f4);
            MAP_TO_KAI_KEY(0x3f, f5);
            MAP_TO_KAI_KEY(0x40, f6);
            MAP_TO_KAI_KEY(0x41, f7);
            MAP_TO_KAI_KEY(0x42, f8);
            MAP_TO_KAI_KEY(0x43, f9);
            MAP_TO_KAI_KEY(0x44, f10);
            MAP_TO_KAI_KEY(0x57, f11);
            MAP_TO_KAI_KEY(0x58, f12);
            MAP_TO_KAI_KEY(0x54, print_screen);
            MAP_TO_KAI_KEY(0x46, scroll_lock);
            MAP_TO_KAI_KEY(0x45, pause);

            MAP_TO_KAI_KEY(0x37, num_multiply);
            MAP_TO_KAI_KEY(0x4a, num_minus);
            MAP_TO_KAI_KEY(0x4e, num_plus);
            MAP_TO_KAI_KEY(0x4f, num_1);
            MAP_TO_KAI_KEY(0x50, num_2);
            MAP_TO_KAI_KEY(0x51, num_3);
            MAP_TO_KAI_KEY(0x4b, num_4);
            MAP_TO_KAI_KEY(0x4c, num_5);
            MAP_TO_KAI_KEY(0x4d, num_6);
            MAP_TO_KAI_KEY(0x47, num_7);
            MAP_TO_KAI_KEY(0x48, num_8);
            MAP_TO_KAI_KEY(0x49, num_9);
            MAP_TO_KAI_KEY(0x52, num_0);
            MAP_TO_KAI_KEY(0x53, num_period);
            MAP_TO_KAI_KEY(0x1d, left_control);
            MAP_TO_KAI_KEY(0x2a, left_shift);
            MAP_TO_KAI_KEY(0x38, left_alt);
            //MAP_TO_KAI_KEY(0x, left_gui);
            MAP_TO_KAI_KEY(0x36, right_shift);
            //MAP_TO_KAI_KEY(0x, right_gui);
        }
    } else {
        switch(win_scancode) {
            MAP_TO_KAI_KEY(0x52, insert);
            MAP_TO_KAI_KEY(0x47, home);
            MAP_TO_KAI_KEY(0x49, page_up);
            MAP_TO_KAI_KEY(0x53, del);
            MAP_TO_KAI_KEY(0x4f, end);
            MAP_TO_KAI_KEY(0x51, page_down);
            MAP_TO_KAI_KEY(0x4d, right);
            MAP_TO_KAI_KEY(0x4b, left);
            MAP_TO_KAI_KEY(0x50, down);
            MAP_TO_KAI_KEY(0x48, up);
            MAP_TO_KAI_KEY(0x45, num_lock);
            MAP_TO_KAI_KEY(0x35, num_divide);
            MAP_TO_KAI_KEY(0x1d, right_control);
            MAP_TO_KAI_KEY(0x38, right_alt);
            MAP_TO_KAI_KEY(0xc1, num_enter);
        }
    }

#undef MAP_TO_KAI_KEY

    return kai::Key::none;
}
