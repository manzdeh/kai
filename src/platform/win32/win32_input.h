/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_WIN32_INPUT_H
#define KAI_WIN32_INPUT_H

#include <windows.h>

#include "../../core/includes/input.h"

kai::Key win32_get_kai_key_from_scancode(UINT win_scancode, bool extended);

#endif /* KAI_WIN32_INPUT_H */
