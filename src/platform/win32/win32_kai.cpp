/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "win32_kai.h"

#include "../../core/includes/kai.h"
#include "../../core/input_internal.h"
#include "../platform.h"

#include "../../core/kai.cpp"
#include "win32_dx11.cpp"
#include "win32_fileio.cpp"
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

static struct {
    kai::Window window;
    HMODULE game_dll;
    size_t page_size;
    Bool32 rendering_backend_initialized;
} win32_state = {};

void win32_kai_log(const char *str, va_list vlist) {
    static char buf[999];
    vsnprintf(buf, sizeof(buf), str, vlist);
    OutputDebugStringA(buf);
}

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
        case WM_LBUTTONUP:
        case WM_LBUTTONDOWN:
            set_mouse_button(kai::MouseButton::left, message == WM_LBUTTONDOWN);
            break;
        case WM_MBUTTONUP:
        case WM_MBUTTONDOWN:
            set_mouse_button(kai::MouseButton::middle, message == WM_MBUTTONDOWN);
            break;
        case WM_RBUTTONUP:
        case WM_RBUTTONDOWN:
            set_mouse_button(kai::MouseButton::right, message == WM_RBUTTONDOWN);
            break;
        case WM_XBUTTONUP:
        case WM_XBUTTONDOWN: {
            kai::MouseButton b;
            if(get_xbutton(b)) {
                set_mouse_button(b, message == WM_XBUTTONDOWN);
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
    SYSTEM_INFO sys;
    GetSystemInfo(&sys);
    win32_state.page_size = static_cast<size_t>(sys.dwPageSize);

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

    WIN32_CHECK_ERROR(window, L"Could not create window!\n");

    GetClientRect(window, &client_rect);

    win32_state.window.platform_window = window;
    win32_state.window.width = client_rect.right;
    win32_state.window.height = client_rect.bottom;

    set_log_callback(win32_kai_log);

    win32_init_gamepads();

    init_engine();

    MSG message;
    bool running = true;
    while(running) {
        while(PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
            switch(message.message) {
                case WM_QUIT:
                    running = false;
                    break;
                default:
                    break;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        win32_poll_keyboard_input();
        win32_update_gamepads();

        running &= tick_engine();
    }

    destroy_engine();

    if(win32_state.game_dll) {
        FreeLibrary(win32_state.game_dll);
    }

    win32_destroy_gamepads();
    DestroyWindow(win32_get_window());

    return 0;
}

kai::Window * platform_get_kai_window(void) {
    return &win32_state.window;
}

HWND win32_get_window(void) {
    return static_cast<HWND>(win32_state.window.platform_window);
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

bool platform_setup_game_callbacks(kai::GameCallbacks &callbacks) {
    if(win32_state.game_dll) {
        FreeLibrary(win32_state.game_dll);
    }

    win32_state.game_dll = LoadLibraryW(L"game.dll");

    if(win32_state.game_dll) {
        auto load_proc = [dll = win32_state.game_dll](const char *name, void **callback_func) {
            FARPROC proc = GetProcAddress(dll, name);
            if(proc) {
                *callback_func = proc;
                return true;
            }

            return false;
        };

        bool retval = load_proc(KAI_TOKEN_TO_STRING(KAI_GAME_INIT_PROCNAME), reinterpret_cast<void **>(&callbacks.init));
        retval &= load_proc(KAI_TOKEN_TO_STRING(KAI_GAME_UPDATE_PROCNAME), reinterpret_cast<void **>(&callbacks.update));
        retval &= load_proc(KAI_TOKEN_TO_STRING(KAI_GAME_DESTROY_PROCNAME), reinterpret_cast<void **>(&callbacks.destroy));

        return retval;
    }

    return false;
}

// At the moment and for the forseeable future the DX11 backend is the only
// one that'll exist, so we don't even bother checking the parameter
void platform_renderer_init_backend(kai::RenderingBackend) {
    if(!win32_state.rendering_backend_initialized) {
        init_dx11();
        win32_state.rendering_backend_initialized = true;
    }
}

void platform_renderer_destroy_backend(void) {
    destroy_dx11();
}

void * kai::virtual_alloc(void *starting_address, size_t bytes, kai::PageAllocFlags page_flags, kai::PageProtection page_protection) {
    DWORD allocation_flags = [page_flags]() {
        DWORD flags = 0;
        if(page_flags & kai::ALLOC_RESERVE) {
            flags |= MEM_RESERVE;
        }

        if(page_flags & kai::ALLOC_COMMIT) {
            flags |= MEM_COMMIT;
        }

        return flags;
    }();

    DWORD protection = [page_protection]() {
        switch(page_protection) {
            case kai::PageProtection::execute: return PAGE_EXECUTE;
            case kai::PageProtection::execute_read: return PAGE_EXECUTE_READ;
            case kai::PageProtection::execute_read_write: return PAGE_EXECUTE_READWRITE;
            case kai::PageProtection::read: return PAGE_READONLY;
            case kai::PageProtection::read_write: return PAGE_READWRITE;
            case kai::PageProtection::no_access: return PAGE_NOACCESS;
            case kai::PageProtection::guard: return PAGE_GUARD;
            default: return 0;
        }
    }();

    return VirtualAlloc(starting_address, bytes, allocation_flags, protection);
}

void * kai::reserve_pages(void *starting_address, size_t page_count) {
    return kai::virtual_alloc(starting_address, kai::get_page_size() * page_count,
                              kai::ALLOC_RESERVE, kai::PageProtection::no_access);
}

void * kai::commit_pages(void *reserved_pages, size_t page_count) {
    return kai::virtual_alloc(reserved_pages, kai::get_page_size() * page_count,
                              kai::ALLOC_COMMIT, kai::PageProtection::read_write);
}

void kai::decommit_pages(void *pages, size_t page_count) {
    VirtualFree(pages, kai::get_page_size() * page_count, MEM_DECOMMIT);
}

void kai::virtual_free(void *pages) {
    VirtualFree(pages, 0, MEM_RELEASE);
}

size_t kai::get_page_size(void) {
    return win32_state.page_size;
}
