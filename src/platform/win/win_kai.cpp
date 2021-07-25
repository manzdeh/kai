/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <stdlib.h>

#include "../../core/includes/kai.h"

// NOTE: Temporary
#define DEFAULT_WINDOW_WIDTH 1024
#define DEFAULT_WINDOW_HEIGHT 576

#define WIN_CHECK_ERROR(expr, msg) \
    do { \
        if(!(expr)) { \
            MessageBoxW(nullptr, msg, L"Error", MB_ICONERROR); \
            exit(-1); \
        } \
    } while(0)

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    switch(message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcW(window, message, w_param, l_param);
    }

    return (LRESULT)0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int) {
    WNDCLASSEXW window_class = {};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    window_class.lpszClassName = L"KAI_ENGINE_CLASS";

    ATOM id = RegisterClassExW(&window_class);
    KAI_ASSERT(id != 0);
#ifndef KAI_DEBUG
    KAI_IGNORED_VARIABLE(id);
#endif

    RECT client_rect = {};
    client_rect.left = 0;
    client_rect.top = 0;
    client_rect.right = DEFAULT_WINDOW_WIDTH;
    client_rect.bottom = DEFAULT_WINDOW_HEIGHT;

    DWORD window_flags = WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX | WS_SYSMENU | WS_VISIBLE;
    AdjustWindowRectEx(&client_rect, window_flags, FALSE, 0);

    HWND window = CreateWindowExW(0,
                                  window_class.lpszClassName,
                                  L"Kai Engine",
                                  window_flags,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  client_rect.right - client_rect.left,
                                  client_rect.bottom - client_rect.top,
                                  nullptr,
                                  nullptr,
                                  instance,
                                  nullptr);

    WIN_CHECK_ERROR(window, L"Could not create window!\n");

    MSG message;
    for(;;) {
        while(PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
            if(message.message == WM_QUIT) {
                goto exit_engine;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

exit_engine:
    DestroyWindow(window);

    return 0;
}
