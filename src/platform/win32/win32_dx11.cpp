/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "win32_dx11.h"
#include <d3d11.h>
#include <dxgi.h>

#define VENDOR_ID_NVIDIA    0x10de
#define VENDOR_ID_AMD       0x1002
#define VENDOR_ID_INTEL     0x8086

void init_dx11(HWND window, Uint32 width, Uint32 height) {
    IDXGIFactory1 *factory;
    if(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(&factory)) == S_OK) {
        IDXGIAdapter1 *adapter = nullptr;
        IDXGIAdapter1 *best_adapter = nullptr;

        Uint32 i = 0;
        for(; factory->EnumAdapters1(i, &adapter) == S_OK; i++) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            // TODO: For now we obtain the first device that has VRAM and is one of the common vendors.
            // Eventually this setting needs to be exposed, so a user can set their preferred device themselves
            if(desc.DedicatedVideoMemory != 0 &&
               (desc.VendorId == VENDOR_ID_NVIDIA || desc.VendorId == VENDOR_ID_AMD ||
                desc.VendorId == VENDOR_ID_INTEL)) {
                best_adapter = adapter;
                best_adapter->AddRef();
                break;
            }

            adapter->Release();
        }

        if(adapter) {
            adapter->Release();
        }

        UINT flags = 0;
#ifdef KAI_DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };

        ID3D11Device *device;
        ID3D11DeviceContext *context;
        IDXGISwapChain *swap_chain;
        D3D_FEATURE_LEVEL set_level;

        if(D3D11CreateDevice(best_adapter,
                             best_adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
                             nullptr,
                             flags,
                             levels,
                             KAI_ARRAY_COUNT(levels),
                             D3D11_SDK_VERSION,
                             &device,
                             &set_level,
                             &context) != S_OK) {
            OutputDebugStringW(L"Error creating DX11 device!\n");
        }

        DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
        swap_chain_desc.BufferDesc.Width = width;
        swap_chain_desc.BufferDesc.Height = height;
        swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
        swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
        swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swap_chain_desc.SampleDesc.Count = 1;
        swap_chain_desc.SampleDesc.Quality = 0;
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.BufferCount = 2;
        swap_chain_desc.OutputWindow = window;
        swap_chain_desc.Windowed = TRUE;
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        if(factory->CreateSwapChain(device, &swap_chain_desc, &swap_chain) != S_OK) {
            OutputDebugStringW(L"Error creating swap chain!\n");
        }

        if(best_adapter) {
            best_adapter->Release();
        }

        factory->Release();
    }
}
