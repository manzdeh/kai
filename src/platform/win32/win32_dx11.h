/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_WIN32_DX11_H
#define KAI_WIN32_DX11_H

#include <windows.h>

#include "../../core/includes/kai.h"

void init_dx11(HWND window, Uint32 width, Uint32 height);
void destroy_dx11(void);

#endif /* KAI_WIN32_DX11_H */
