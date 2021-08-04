/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#include "../../core/input_internal.h"
#include "../platform.h"

#include "../../core/kai.cpp"
#include "win32_input.cpp"

// NOTE: Temporary
#define DEFAULT_WINDOW_WIDTH 1024
#define DEFAULT_WINDOW_HEIGHT 576

#define WIN32_CHECK_ERROR(expr, msg) \
    do { \
        if(!(expr)) { \
            MessageBoxW(nullptr, msg, L"Error", MB_ICONERROR); \
            exit(-1); \
        } \
    } while(0)

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {

    auto get_xbutton = [w_param](kai::MouseButton &button) -> bool {
        UINT xbtn = GET_XBUTTON_WPARAM(w_param);
        if(xbtn & 0x1) {
            button = kai::MouseButton::x1;
            return true;
        } else if(xbtn & 0x2) {
            button = kai::MouseButton::x2;
            return true;
        }

        return false;
    };

    switch(message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_LBUTTONDOWN:
            set_mouse_button(kai::MouseButton::left, true, false);
            break;
        case WM_MBUTTONDOWN:
            set_mouse_button(kai::MouseButton::middle, true, false);
            break;
        case WM_RBUTTONDOWN:
            set_mouse_button(kai::MouseButton::right, true, false);
            break;
        case WM_XBUTTONDOWN: {
            kai::MouseButton b;
            if(get_xbutton(b)) {
                set_mouse_button(b, true, false);
            }
            break;
        }
        case WM_LBUTTONDBLCLK:
            set_mouse_button(kai::MouseButton::left, true, true);
            break;
        case WM_MBUTTONDBLCLK:
            set_mouse_button(kai::MouseButton::middle, true, true);
            break;
        case WM_RBUTTONDBLCLK:
            set_mouse_button(kai::MouseButton::right, true, true);
            break;
        case WM_XBUTTONDBLCLK: {
            kai::MouseButton b;
            if(get_xbutton(b)) {
                set_mouse_button(b, true, true);
            }
            break;
        }
        case WM_MOUSEWHEEL:
            set_scroll_delta(GET_WHEEL_DELTA_WPARAM(w_param) / WHEEL_DELTA);
            break;
        default:
            return DefWindowProcW(window, message, w_param, l_param);
    }

    return (LRESULT)0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int) {
    WNDCLASSEXW window_class = {};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
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

    WIN32_CHECK_ERROR(window, L"Could not create window!\n");

    init_engine();

    MSG message;
    for(;;) {
        while(PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
            switch(message.message) {
                case WM_QUIT:
                    goto exit_engine;
                default:
                    break;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        win32_poll_keyboard_input();

        tick_engine();
    }

exit_engine:
    destroy_engine();
    DestroyWindow(window);

    return 0;
}

void * platform_alloc_mem_arena(size_t bytes, void *address) {
#ifndef KAI_DEBUG
    KAI_IGNORED_VARIABLE(address);
#endif

    return VirtualAlloc(address, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void platform_free_mem_arena(void *arena) {
    if(arena) {
        VirtualFree(arena, 0, MEM_RELEASE);
    }
}

size_t platform_get_page_size(void) {
    SYSTEM_INFO sys;
    GetSystemInfo(&sys);

    return (size_t)sys.dwPageSize;
}
