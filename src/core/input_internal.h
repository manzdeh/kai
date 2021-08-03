/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_INPUT_INTERNAL_H
#define KAI_INPUT_INTERNAL_H

#include "includes/input.h"

void init_keyboard(void);
void swap_keyboard_buffers(void);

void set_key(kai::Key key, Bool32 value);

#endif /* KAI_INPUT_INTERNAL_H */
