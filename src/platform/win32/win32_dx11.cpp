/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "win32_dx11.h"
#include "../platform.h"

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#define VENDOR_ID_NVIDIA    0x10de
#define VENDOR_ID_AMD       0x1002
#define VENDOR_ID_INTEL     0x8086

static struct {
    IDXGIFactory *factory;
    ID3D11Device *device;
    ID3D11DeviceContext *context;
    IDXGISwapChain *swap_chain;
    ID3D11RenderTargetView *rtv;
    ID3D11DepthStencilView *dsv;

    DX11Renderer renderer;
} dx11_state;

// At the moment and for the forseeable future the DX11 backend is the only
// one that'll exist, so we don't even bother checking the parameter
void * platform_get_backend_renderer(kai::RenderingBackend) {
    return &dx11_state.renderer;
}

static void init_dx11_state(void) {
    kai::Window *window = kai::get_window();

    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    swap_chain_desc.BufferDesc.Width = window->width;
    swap_chain_desc.BufferDesc.Height = window->height;
    swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.OutputWindow = static_cast<HWND>(window->platform_window);
    swap_chain_desc.Windowed = true;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    if(dx11_state.factory->CreateSwapChain(dx11_state.device, &swap_chain_desc, &dx11_state.swap_chain) != S_OK) {
        kai::log("Error: failed creating the swap chain!\n");
    }

    ID3D11Texture2D *back_buffer;
    dx11_state.swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&back_buffer));
    dx11_state.device->CreateRenderTargetView(back_buffer, nullptr, &dx11_state.rtv);

    D3D11_TEXTURE2D_DESC depth_desc = {};
    depth_desc.Width = swap_chain_desc.BufferDesc.Width;
    depth_desc.Height = swap_chain_desc.BufferDesc.Height;
    depth_desc.MipLevels = 1;
    depth_desc.ArraySize = 1;
    depth_desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    depth_desc.SampleDesc.Count = swap_chain_desc.SampleDesc.Count;
    depth_desc.SampleDesc.Quality = swap_chain_desc.SampleDesc.Quality;
    depth_desc.Usage = D3D11_USAGE_DEFAULT;
    depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depth_desc.CPUAccessFlags = 0;
    depth_desc.MiscFlags = 0;

    ID3D11Texture2D *depth_buffer;
    dx11_state.device->CreateTexture2D(&depth_desc, nullptr, &depth_buffer);

    D3D11_DEPTH_STENCIL_DESC ds_desc = {};
    ds_desc.DepthEnable = true;
    ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    ds_desc.DepthFunc = D3D11_COMPARISON_LESS;
    ds_desc.StencilEnable = false;

    ID3D11DepthStencilState *ds_state;
    dx11_state.device->CreateDepthStencilState(&ds_desc, &ds_state);
    dx11_state.context->OMSetDepthStencilState(ds_state, 1);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
    dsv_desc.Format = depth_desc.Format;
    dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsv_desc.Texture2D.MipSlice = 0;

    dx11_state.device->CreateDepthStencilView(depth_buffer, &dsv_desc, &dx11_state.dsv);
    dx11_state.context->OMSetRenderTargets(1, &dx11_state.rtv, dx11_state.dsv);

    ds_state->Release();
    depth_buffer->Release();
    back_buffer->Release();

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<Float32>(swap_chain_desc.BufferDesc.Width);
    viewport.Height = static_cast<Float32>(swap_chain_desc.BufferDesc.Height);
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1.0f;

    D3D11_RASTERIZER_DESC rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_BACK;
    rasterizer_desc.FrontCounterClockwise = true;
    rasterizer_desc.DepthBias = 0;
    rasterizer_desc.DepthBiasClamp = 0;
    rasterizer_desc.SlopeScaledDepthBias = 0;
    rasterizer_desc.DepthClipEnable = false;
    rasterizer_desc.ScissorEnable = false;
    rasterizer_desc.MultisampleEnable = false;
    rasterizer_desc.AntialiasedLineEnable = false;

    ID3D11RasterizerState *rasterizer_state;
    dx11_state.device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);

    dx11_state.context->RSSetViewports(1, &viewport);
    dx11_state.context->RSSetState(rasterizer_state);
    rasterizer_state->Release();

    dx11_state.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    {
        // TODO: Temporary setup to test the pipeline
        static const char vshader[] = {
            "struct VSInput {\n"
            "   float3 pos : POSITION;\n"
            "};\n"
            "struct VSOutput {\n"
            "   float4 pos : SV_Position;\n"
            "};\n"
            "VSOutput vs_main(VSInput input) {\n"
            "   VSOutput output;\n"
            "   output.pos = float4(input.pos, 1.0);\n"
            "   return output;\n"
            "}"
        };

        static const char pshader[] = {
            "struct PSInput {\n"
            "   float4 pos : SV_Position;\n"
            "};\n"
            "float4 ps_main(PSInput input) : SV_TARGET {\n"
            "   return float4(1.0, 0.0, 0.0, 1.0);\n"
            "}"
        };

        ID3DBlob *vertex_buffer, *pixel_buffer;

        auto compile_shader = [](const char *shader, Uint32 count, const char *entry, const char *version) {
            ID3DBlob *buffer = nullptr;
            ID3DBlob *error = nullptr;
            UINT compile_flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#ifdef KAI_DEBUG
            compile_flags |= D3DCOMPILE_DEBUG;
#endif
            D3DCompile(shader, count, nullptr, nullptr, nullptr, entry, version,
                       compile_flags, 0, &buffer, &error);

            if(error) {
                kai::log(static_cast<LPCSTR>(error->GetBufferPointer()));
                error->Release();
            }

            return buffer;
        };

        vertex_buffer = compile_shader(vshader, KAI_ARRAY_COUNT(vshader), "vs_main", "vs_5_0");
        pixel_buffer = compile_shader(pshader, KAI_ARRAY_COUNT(pshader), "ps_main", "ps_5_0");

        ID3D11VertexShader *vertex_shader;
        ID3D11PixelShader *pixel_shader;
        dx11_state.device->CreateVertexShader(vertex_buffer->GetBufferPointer(),
                                              vertex_buffer->GetBufferSize(),
                                              nullptr,
                                              &vertex_shader);
        dx11_state.device->CreatePixelShader(pixel_buffer->GetBufferPointer(),
                                             pixel_buffer->GetBufferSize(),
                                             nullptr,
                                             &pixel_shader);

        static const Float32 triangle[] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };

        D3D11_BUFFER_DESC bd = {};
        bd.ByteWidth = sizeof(Float32) * KAI_ARRAY_COUNT(triangle);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA srd = {};
        srd.pSysMem = triangle;

        ID3D11Buffer *vertex_data;
        dx11_state.device->CreateBuffer(&bd, &srd, &vertex_data);

        D3D11_INPUT_ELEMENT_DESC input_desc = {};
        input_desc.SemanticName = "POSITION";
        input_desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        input_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        input_desc.AlignedByteOffset = 0;

        ID3D11InputLayout *input_layout;
        dx11_state.device->CreateInputLayout(&input_desc, 1, vertex_buffer->GetBufferPointer(),
                                             vertex_buffer->GetBufferSize(), &input_layout);

        Uint32 stride = sizeof(Float32) * 3;
        Uint32 offset = 0;

        dx11_state.context->VSSetShader(vertex_shader, nullptr, 0);
        dx11_state.context->PSSetShader(pixel_shader, nullptr, 0);
        dx11_state.context->IASetInputLayout(input_layout);
        dx11_state.context->IASetVertexBuffers(0, 1, &vertex_data, &stride, &offset);

        vertex_shader->Release();
        pixel_shader->Release();
        input_layout->Release();
        vertex_buffer->Release();
        pixel_buffer->Release();
        vertex_data->Release();
    }
}

static bool create_dx11_device(IDXGIAdapter *adapter = nullptr) {
    UINT flags = 0;
#ifdef KAI_DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };

    bool success = D3D11CreateDevice(adapter,
                                     adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
                                     nullptr,
                                     flags,
                                     levels,
                                     KAI_ARRAY_COUNT(levels),
                                     D3D11_SDK_VERSION,
                                     &dx11_state.device,
                                     nullptr,
                                     &dx11_state.context) == S_OK;

    if(success) {
        init_dx11_state();
    }

    return success;
}

void DX11Renderer::init_default_device(kai::RenderDevice &device) {
    if(!dx11_state.device && create_dx11_device()) {
        IDXGIDevice *dxgi_device;
        IDXGIAdapter *adapter;
        dx11_state.device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&dxgi_device));

        DXGI_ADAPTER_DESC desc;
        dxgi_device->GetAdapter(&adapter);
        adapter->GetDesc(&desc);

        device.id = desc.DeviceId;
        device.backend = kai::RenderingBackend::dx11;

        adapter->Release();
        dxgi_device->Release();
    }
}

void DX11Renderer::init_device(kai::RenderDevice &device, Uint32 id) {
    if(!dx11_state.device) {
        IDXGIAdapter *adapter = nullptr;

        bool created_device = false;

        Uint32 i = 0;
        for(; dx11_state.factory->EnumAdapters(i, &adapter) == S_OK; i++) {
            DXGI_ADAPTER_DESC desc;
            adapter->GetDesc(&desc);

            if(desc.DeviceId == id) {
                created_device = create_dx11_device(adapter);
                break;
            }

            adapter->Release();
        }

        if(adapter) {
            adapter->Release();
        }

        if(created_device) {
            device.id = id;
            device.backend = kai::RenderingBackend::dx11;
        } else {
            init_default_device(device);
        }
    }
}

void init_dx11(void) {
    if(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void **>(&dx11_state.factory)) != S_OK) {
        kai::log("Error: failed creating the IDXGIFactory!\n");
    }
}

void destroy_dx11(void) {
#define DESTROY_IF_NEEDED(item) \
    if(dx11_state.item) { \
        dx11_state.item->Release(); \
        dx11_state.item = nullptr; \
    }

    DESTROY_IF_NEEDED(factory);
    DESTROY_IF_NEEDED(device);
    DESTROY_IF_NEEDED(context);
    DESTROY_IF_NEEDED(swap_chain);
    DESTROY_IF_NEEDED(rtv);
    DESTROY_IF_NEEDED(dsv);

#undef DESTROY_IF_NEEDED
}
