/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_WIN32_INPUT_H
#define KAI_WIN32_INPUT_H

#include <windows.h>

#include "../../core/includes/input.h"

kai::Key win32_get_kai_key_from_scancode(UINT win_scancode);
void win32_poll_keyboard_input(void);

void win32_init_gamepads(void);
void win32_destroy_gamepads(void);
void win32_update_gamepads(void);

#endif /* KAI_WIN32_INPUT_H */
