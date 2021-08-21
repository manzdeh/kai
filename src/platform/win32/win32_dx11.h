/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#ifndef KAI_WIN32_DX11_H
#define KAI_WIN32_DX11_H

#include <windows.h>

#include "../../core/includes/kai.h"

void init_dx11(void);
void destroy_dx11(void);

DEFINE_RENDERER_BACKEND(DX11Renderer);

#endif /* KAI_WIN32_DX11_H */
