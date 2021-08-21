/**************************************************
 * Copyright (c) 2021 Amanch Esmailzadeh
 * See LICENSE for details
 **************************************************/

#include "win32_dx11.h"
#include "../platform.h"

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <malloc.h>

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
kai::Renderer * platform_get_backend_renderer(kai::RenderingBackend) {
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




    {
        // TODO: Temporary render pipeline test
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

        kai::Renderer *renderer = kai::get_renderer();
        renderer->set_viewport(0, 0);

        kai::RenderPipelineInfo pipeline_info = {};
        pipeline_info.vertex_shader_source = vshader;
        pipeline_info.pixel_shader_source = pshader;
        pipeline_info.vertex_shader_entry = "vs_main";
        pipeline_info.pixel_shader_entry = "ps_main";
        pipeline_info.fill_mode = kai::RenderPipelineInfo::FillMode::solid;
        pipeline_info.cull_mode = kai::RenderPipelineInfo::CullMode::back;
        pipeline_info.topology = kai::RenderPipelineInfo::TopologyType::triangle_list;
        pipeline_info.front_ccw = true;

        kai::RenderInputLayoutInfo input_info = {};
        input_info.name = "POSITION";
        input_info.format = kai::RenderFormat::rgb_f32;

        kai::RenderPipeline pipeline;
        renderer->create_render_pipeline(pipeline_info, &input_info, 1, pipeline);
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
        WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK | WC_DEFAULTCHAR,
                            desc.Description, -1, device.name, sizeof(device.name), nullptr, nullptr);

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

void DX11Renderer::set_viewport(Int32 x, Int32 y, Uint32 width, Uint32 height) {
    kai::Window *window = kai::get_window();
    width = (width > 0) ? width : window->width;
    height = (height > 0) ? height : window->height;

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = static_cast<Float32>(x);
    viewport.TopLeftY = static_cast<Float32>(y);
    viewport.Width = static_cast<Float32>(width);
    viewport.Height = static_cast<Float32>(height);
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1.0f;

    dx11_state.context->RSSetViewports(1, &viewport);
}

bool DX11Renderer::compile_shader(const char *shader_stream, kai::ShaderType type,
                                  const char *entry, void *out_id, void **blob) const {
    ID3DBlob *buffer = nullptr;
    ID3DBlob *error = nullptr;
    UINT compile_flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#ifdef KAI_DEBUG
    compile_flags |= D3DCOMPILE_DEBUG;
#endif

    const char *version = nullptr;
    switch(type) {
        case kai::ShaderType::vertex: version = "vs_5_0"; break;
        case kai::ShaderType::pixel: version = "ps_5_0"; break;
    }

    D3DCompile(shader_stream, strlen(shader_stream), nullptr, nullptr, nullptr, entry, version,
               compile_flags, 0, &buffer, &error);

    if(error) {
        kai::log(static_cast<LPCSTR>(error->GetBufferPointer()));
        error->Release();
        return false;
    }

    switch(type) {
        case kai::ShaderType::vertex: {
            ID3D11VertexShader *shader;
            dx11_state.device->CreateVertexShader(buffer->GetBufferPointer(),
                                                  buffer->GetBufferSize(),
                                                  nullptr,
                                                  &shader);

            if(out_id) {
                *reinterpret_cast<kai::VertexShaderID *>(out_id) = reinterpret_cast<kai::VertexShaderID>(shader);
            }

            break;
        }
        case kai::ShaderType::pixel: {
            ID3D11PixelShader *shader;
            dx11_state.device->CreatePixelShader(buffer->GetBufferPointer(),
                                                 buffer->GetBufferSize(),
                                                 nullptr,
                                                 &shader);

            if(out_id) {
                *reinterpret_cast<kai::PixelShaderID *>(out_id) = reinterpret_cast<kai::PixelShaderID>(shader);
            }

            break;
        }
    }

    if(blob && type == kai::ShaderType::vertex) {
        *blob = buffer;
    } else {
        buffer->Release();
    }

    return true;
}

static KAI_FORCEINLINE DXGI_FORMAT get_dxgi_format(kai::RenderFormat format) {
    switch(format) {
        case kai::RenderFormat::r_u8: return DXGI_FORMAT_R8_UINT;
        case kai::RenderFormat::rg_u8: return DXGI_FORMAT_R8G8_UINT;
        case kai::RenderFormat::rgba_u8: return DXGI_FORMAT_R8G8B8A8_UINT;
        case kai::RenderFormat::r_f32: return DXGI_FORMAT_R32_FLOAT;
        case kai::RenderFormat::rg_f32: return DXGI_FORMAT_R32G32_FLOAT;
        case kai::RenderFormat::rgb_f32: return DXGI_FORMAT_R32G32B32_FLOAT;
        case kai::RenderFormat::rgba_f32: return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case kai::RenderFormat::unknown: default: return DXGI_FORMAT_UNKNOWN;

    }
}

bool DX11Renderer::create_render_pipeline(const kai::RenderPipelineInfo &info, const kai::RenderInputLayoutInfo *input_layouts,
                                          Uint32 input_layout_count, kai::RenderPipeline &out_pipeline) {
    D3D11_RASTERIZER_DESC rasterizer_desc = {};
    rasterizer_desc.FillMode = [fill_mode = info.fill_mode]() {
        switch(fill_mode) {
            case kai::RenderPipelineInfo::FillMode::wireframe: return D3D11_FILL_WIREFRAME;
            case kai::RenderPipelineInfo::FillMode::solid: default: return D3D11_FILL_SOLID;
        }
    }();
    rasterizer_desc.CullMode = [cull_mode = info.cull_mode]() {
        switch(cull_mode) {
            case kai::RenderPipelineInfo::CullMode::front: return D3D11_CULL_FRONT;
            case kai::RenderPipelineInfo::CullMode::back: return D3D11_CULL_BACK;
            case kai::RenderPipelineInfo::CullMode::none: default: return D3D11_CULL_NONE;
        }
    }();
    rasterizer_desc.FrontCounterClockwise = info.front_ccw;
    rasterizer_desc.DepthBias = 0;
    rasterizer_desc.DepthBiasClamp = 0;
    rasterizer_desc.SlopeScaledDepthBias = 0;
    rasterizer_desc.DepthClipEnable = false;
    rasterizer_desc.ScissorEnable = false;
    rasterizer_desc.MultisampleEnable = false;
    rasterizer_desc.AntialiasedLineEnable = false;

    ID3D11RasterizerState *rasterizer_state;
    dx11_state.device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);

    dx11_state.context->RSSetState(rasterizer_state);
    rasterizer_state->Release();


    ID3DBlob *vertex_shader_buffer;
    if(!compile_shader(info.vertex_shader_source, kai::ShaderType::vertex, info.vertex_shader_entry,
                      &out_pipeline.vertex_shader, reinterpret_cast<void **>(&vertex_shader_buffer))) {
        kai::log("Vertex shader compilation failed!\n");
        return false;
    }
    if(!compile_shader(info.pixel_shader_source, kai::ShaderType::pixel, info.pixel_shader_entry,
                       &out_pipeline.pixel_shader)) {
        kai::log("Pixel shader compilation failed!\n");
        return false;
    }


    size_t input_size = sizeof(D3D11_INPUT_ELEMENT_DESC) * input_layout_count;
    D3D11_INPUT_ELEMENT_DESC *input_descs = static_cast<D3D11_INPUT_ELEMENT_DESC *>(_malloca(input_size));
    memset(input_descs, 0, input_size);

    for(Uint32 i = 0; i < input_layout_count; i++) {
        input_descs[i].SemanticName = input_layouts[i].name;
        input_descs[i].SemanticIndex = input_layouts[i].index;
        input_descs[i].Format = get_dxgi_format(input_layouts[i].format);
        input_descs[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        input_descs[i].AlignedByteOffset = (input_layouts[i].offset != KAI_INPUT_LAYOUT_APPEND) ?
            input_layouts[i].offset : D3D11_INPUT_PER_VERTEX_DATA;
    }

    ID3D11InputLayout *input_layout;
    dx11_state.device->CreateInputLayout(input_descs, input_layout_count, vertex_shader_buffer->GetBufferPointer(),
                                         vertex_shader_buffer->GetBufferSize(), &input_layout);

    vertex_shader_buffer->Release();

    D3D11_PRIMITIVE_TOPOLOGY topology = [topology = info.topology]() {
        switch(topology) {
            case kai::RenderPipelineInfo::TopologyType::point_list: return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            case kai::RenderPipelineInfo::TopologyType::line_list: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            case kai::RenderPipelineInfo::TopologyType::line_strip: return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case kai::RenderPipelineInfo::TopologyType::triangle_list: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case kai::RenderPipelineInfo::TopologyType::triangle_strip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            case kai::RenderPipelineInfo::TopologyType::undefined: default: return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
    }();

    // TODO: This should happen when the RenderPipeline is set as active
    dx11_state.context->IASetPrimitiveTopology(topology);
    dx11_state.context->VSSetShader(reinterpret_cast<ID3D11VertexShader *>(out_pipeline.vertex_shader), nullptr, 0);
    dx11_state.context->PSSetShader(reinterpret_cast<ID3D11PixelShader *>(out_pipeline.pixel_shader), nullptr, 0);
    dx11_state.context->IASetInputLayout(input_layout);

    input_layout->Release();


    // TODO: Temporary!
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

    Uint32 stride = sizeof(Float32) * 3;
    Uint32 offset = 0;
    dx11_state.context->IASetVertexBuffers(0, 1, &vertex_data, &stride, &offset);

    return true;
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
