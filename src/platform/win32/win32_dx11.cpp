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

#define DX11_DEVICE_POOL_COUNT 4

struct DX11DeviceData {
    ID3D11Device *device;
    ID3D11DeviceContext *context;
    IDXGISwapChain *swap_chain;
    ID3D11RenderTargetView *render_target_view;
};

#define DX11_RENDER_PIPELINE_POOL_COUNT 32

struct DX11RenderPipelineData {
    ID3D11RasterizerState *rasterizer_state;
    ID3D11InputLayout *input_layout;
    ID3D11DepthStencilView *depth_stencil_view;
    D3D11_PRIMITIVE_TOPOLOGY topology;
    Float32 clear_color[4];
    Float32 depth_clear;
    Uint32 stencil_clear;
};

static struct {
    IDXGIFactory *factory;
    DX11RenderPipelineData *active_pipeline;
    DX11Renderer renderer;

    kai::PoolAllocator devices_pool;
    kai::PoolAllocator pipelines_pool;
} dx11_state;

static void dx11_state_setup(DX11Renderer &renderer) {
    DX11DeviceData *data = static_cast<DX11DeviceData *>(renderer.data);

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

    if(dx11_state.factory->CreateSwapChain(data->device, &swap_chain_desc, &data->swap_chain) != S_OK) {
        kai::log("Error: failed creating the swap chain!\n");
    }

    ID3D11Texture2D *back_buffer;
    data->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&back_buffer));
    data->device->CreateRenderTargetView(back_buffer, nullptr, &data->render_target_view);

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

        renderer.set_viewport(0, 0);

        kai::RenderPipelineInfo pipeline_info = {};
        pipeline_info.vertex_shader_source = vshader;
        pipeline_info.pixel_shader_source = pshader;
        pipeline_info.vertex_shader_entry = "vs_main";
        pipeline_info.pixel_shader_entry = "ps_main";
        pipeline_info.fill_mode = kai::RenderPipelineInfo::FillMode::solid;
        pipeline_info.cull_mode = kai::RenderPipelineInfo::CullMode::back;
        pipeline_info.topology = kai::RenderPipelineInfo::TopologyType::triangle_list;
        pipeline_info.front_ccw = true;
        pipeline_info.color_enable = true;
        pipeline_info.color_clear_values[0] = 0.25f;
        pipeline_info.color_clear_values[1] = 0.15f;
        pipeline_info.color_clear_values[2] = 0.35f;
        pipeline_info.color_clear_values[3] = 1.0f;
        pipeline_info.depth_enable = true;
        pipeline_info.depth_clear_value = 1.0f;
        pipeline_info.stencil_enable = false;

        kai::RenderInputLayoutInfo input_info = {};
        input_info.name = "POSITION";
        input_info.format = kai::RenderFormat::rgb_f32;

        kai::RenderPipeline pipeline;
        renderer.create_render_pipeline(pipeline_info, &input_info, 1, pipeline);
        renderer.set_render_pipeline(pipeline);

    }
}

static bool create_dx11_device(DX11Renderer &dx11_renderer, IDXGIAdapter *adapter = nullptr) {
    UINT flags = 0;
#ifdef KAI_DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };

    DX11DeviceData *data = static_cast<DX11DeviceData *>(dx11_state.devices_pool.alloc());

    bool success = D3D11CreateDevice(adapter,
                                     adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
                                     nullptr,
                                     flags,
                                     levels,
                                     KAI_ARRAY_COUNT(levels),
                                     D3D11_SDK_VERSION,
                                     &data->device,
                                     nullptr,
                                     &data->context) == S_OK;

    if(success) {
        dx11_renderer.data = data;
        dx11_state_setup(dx11_renderer);
    } else {
        dx11_state.devices_pool.free(data);
    }

    return success;
}

static KAI_FORCEINLINE void set_device_properties(DX11Renderer &device, const DXGI_ADAPTER_DESC &desc) {
    device.id = desc.DeviceId;
    device.backend = kai::RenderingBackend::dx11;
    WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK | WC_DEFAULTCHAR,
                        desc.Description, -1, device.name, sizeof(device.name), nullptr, nullptr);
}

kai::RenderDevice * platform_renderer_init_device(kai::StackAllocator &allocator) {
    kai::StackMarker marker;
    DX11Renderer *device = allocator.alloc<DX11Renderer, DX11Renderer>(&marker);

    if(create_dx11_device(*device)) {
        DX11DeviceData *data = static_cast<DX11DeviceData *>(device->data);

        IDXGIDevice *dxgi_device;
        IDXGIAdapter *adapter;

        data->device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&dxgi_device));

        DXGI_ADAPTER_DESC desc;
        dxgi_device->GetAdapter(&adapter);
        adapter->GetDesc(&desc);

        set_device_properties(*device, desc);

        adapter->Release();
        dxgi_device->Release();

        return device;
    }

    if(device) {
        allocator.free(marker);
    }

    return nullptr;
}

kai::RenderDevice * platform_renderer_init_device(kai::StackAllocator &allocator, Uint32 id) {
    IDXGIAdapter *adapter = nullptr;

    kai::StackMarker marker;
    DX11Renderer *device = allocator.alloc<DX11Renderer, DX11Renderer>(&marker);

    bool created_device = false;

    Uint32 i = 0;
    DXGI_ADAPTER_DESC desc;
    for(; dx11_state.factory->EnumAdapters(i, &adapter) == S_OK; i++) {
        adapter->GetDesc(&desc);

        if(desc.DeviceId == id) {
            created_device = create_dx11_device(*device, adapter);
            break;
        }

        adapter->Release();
    }

    if(adapter) {
        adapter->Release();
    }

    if(created_device) {
        set_device_properties(*device, desc);
    } else {
        allocator.free(marker);
        return platform_renderer_init_device(allocator);
    }

    return device;
}

void DX11Renderer::destroy_device(void) {
    DX11DeviceData *d = static_cast<DX11DeviceData *>(data);

    d->device->Release();
    d->context->Release();
    d->swap_chain->Release();
    d->render_target_view->Release();

    dx11_state.devices_pool.free(d);

    memset(this, 0, sizeof(*this));
}

void DX11Renderer::execute(const kai::CommandBuffer &command_buffer) const {
    KAI_ASSERT(dx11_state.active_pipeline);

    Uint32 offset = 0;
    auto fetch_next_command = [data = command_buffer.get_data(), &offset](CommandEncoding &encoding, const void **address) {
        const Uint8 *buffer = static_cast<const Uint8 *>(data) + offset;
        encoding = *reinterpret_cast<const CommandEncoding *>(buffer);

        switch(encoding) {
            case CommandEncoding::draw:
                offset += sizeof(CommandEncodingData::Draw);
                break;
            case CommandEncoding::bind_buffer:
                offset += sizeof(CommandEncodingData::BindBuffer);
                break;

            // These don't have any data, only an id for the command to execute
            case CommandEncoding::clear_color:
            case CommandEncoding::clear_depth:
            case CommandEncoding::clear_stencil:
            case CommandEncoding::clear_depth_stencil:
                offset += sizeof(CommandEncoding);
                break;

            case CommandEncoding::end:
            default:
                return false;
        }

        *address = buffer;
        return true;
    };

    DX11DeviceData *d = static_cast<DX11DeviceData *>(data);
    DX11RenderPipelineData *p = dx11_state.active_pipeline;

    CommandEncoding encoding;
    const void *address;
    while(fetch_next_command(encoding, &address)) {
        switch(encoding) {
            case CommandEncoding::draw: {
                const auto *c = static_cast<const CommandEncodingData::Draw *>(address);
                d->context->Draw(c->count, c->start);
                break;
            }
            case CommandEncoding::bind_buffer: {
                const auto *c = static_cast<const CommandEncodingData::BindBuffer *>(address);
                Uint32 stride = c->buffer->stride;
                Uint32 off = 0;
                d->context->IASetVertexBuffers(0, 1, &static_cast<ID3D11Buffer *>(c->buffer->data), &stride, &off);
                break;
            }

            case CommandEncoding::clear_color:
                d->context->ClearRenderTargetView(d->render_target_view, p->clear_color);
                break;
            case CommandEncoding::clear_depth:
                d->context->ClearDepthStencilView(p->depth_stencil_view, D3D11_CLEAR_DEPTH,
                                                  p->depth_clear, 0);
                break;
            case CommandEncoding::clear_stencil:
                d->context->ClearDepthStencilView(p->depth_stencil_view, D3D11_CLEAR_STENCIL,
                                                  0.0f, static_cast<Uint8>(p->stencil_clear));
                break;
            case CommandEncoding::clear_depth_stencil:
                d->context->ClearDepthStencilView(p->depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                  p->depth_clear, static_cast<Uint8>(p->stencil_clear));
                break;

            default:
                continue;
        }
    }
}

void DX11Renderer::present(void) const {
    static_cast<DX11DeviceData *>(data)->swap_chain->Present(1, 0);
}

void DX11Renderer::set_viewport(Int32 x, Int32 y, Uint32 width, Uint32 height) const {
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

    static_cast<DX11DeviceData *>(data)->context->RSSetViewports(1, &viewport);
}

bool DX11Renderer::compile_shader(const char *shader_stream, kai::ShaderType type,
                                  const char *entry, void *out_id, void **bytecode) const {
    DX11DeviceData *d = static_cast<DX11DeviceData *>(data);

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
            d->device->CreateVertexShader(buffer->GetBufferPointer(),
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
            d->device->CreatePixelShader(buffer->GetBufferPointer(),
                                         buffer->GetBufferSize(),
                                         nullptr,
                                         &shader);

            if(out_id) {
                *reinterpret_cast<kai::PixelShaderID *>(out_id) = reinterpret_cast<kai::PixelShaderID>(shader);
            }

            break;
        }
    }

    if(bytecode && type == kai::ShaderType::vertex) {
        *bytecode = buffer;
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
                                          Uint32 input_layout_count, kai::RenderPipeline &out_pipeline) const {

    DX11DeviceData *d = static_cast<DX11DeviceData *>(data);
    DX11RenderPipelineData *dx11_pipeline = static_cast<DX11RenderPipelineData *>(dx11_state.pipelines_pool.alloc());

    if(!dx11_pipeline) {
        kai::log("Could not create a new render pipeline object!\n");
        return false;
    }

    if(info.depth_enable) {
        DXGI_SWAP_CHAIN_DESC swap_chain_desc;
        d->swap_chain->GetDesc(&swap_chain_desc);

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

        ID3D11Texture2D *depth_buffer;
        d->device->CreateTexture2D(&depth_desc, nullptr, &depth_buffer);

        D3D11_DEPTH_STENCIL_DESC ds_desc = {};
        ds_desc.DepthEnable = true;
        ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        ds_desc.DepthFunc = D3D11_COMPARISON_LESS;
        ds_desc.StencilEnable = info.stencil_enable; // TODO: Set up the stencil state as well

        ID3D11DepthStencilState *ds_state;
        d->device->CreateDepthStencilState(&ds_desc, &ds_state);
        d->context->OMSetDepthStencilState(ds_state, 1);

        D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
        dsv_desc.Format = depth_desc.Format;
        dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsv_desc.Texture2D.MipSlice = 0;

        d->device->CreateDepthStencilView(depth_buffer, &dsv_desc, &dx11_pipeline->depth_stencil_view);

        ds_state->Release();
        depth_buffer->Release();

        dx11_pipeline->depth_clear = info.depth_clear_value;
        kai::clamp(dx11_pipeline->depth_clear, 0.0f, 1.0f);
    } else {
        dx11_pipeline->depth_clear = 0.0f;
    }

    if(info.stencil_enable) {
        dx11_pipeline->stencil_clear = info.stencil_clear_value;
        kai::clamp<Uint32>(dx11_pipeline->stencil_clear, 0, 0xff);
    } else {
        dx11_pipeline->stencil_clear = 0;
    }

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

    d->device->CreateRasterizerState(&rasterizer_desc, &dx11_pipeline->rasterizer_state);


    ID3DBlob *vs_bytecode;
    if(!compile_shader(info.vertex_shader_source, kai::ShaderType::vertex, info.vertex_shader_entry,
                      &out_pipeline.vertex_shader, reinterpret_cast<void **>(&vs_bytecode))) {
        kai::log("Vertex shader compilation failed!\n");
        return false;
    }
    if(!compile_shader(info.pixel_shader_source, kai::ShaderType::pixel, info.pixel_shader_entry,
                       &out_pipeline.pixel_shader)) {
        kai::log("Pixel shader compilation failed!\n");
        return false;
    }

    out_pipeline.data = dx11_pipeline;


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

    d->device->CreateInputLayout(input_descs, input_layout_count, vs_bytecode->GetBufferPointer(),
                                 vs_bytecode->GetBufferSize(), &dx11_pipeline->input_layout);

    vs_bytecode->Release();

    dx11_pipeline->topology = [topology = info.topology]() {
        switch(topology) {
            case kai::RenderPipelineInfo::TopologyType::point_list: return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            case kai::RenderPipelineInfo::TopologyType::line_list: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            case kai::RenderPipelineInfo::TopologyType::line_strip: return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case kai::RenderPipelineInfo::TopologyType::triangle_list: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case kai::RenderPipelineInfo::TopologyType::triangle_strip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            case kai::RenderPipelineInfo::TopologyType::undefined: default: return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
    }();

    memcpy(&dx11_pipeline->clear_color, info.color_clear_values, sizeof(dx11_pipeline->clear_color));
    for(Uint32 i = 0; i < KAI_ARRAY_COUNT(dx11_pipeline->clear_color); i++) {
        kai::clamp(dx11_pipeline->clear_color[i], 0.0f, 1.0f);
    }

    return true;
}

void DX11Renderer::destroy_render_pipeline(kai::RenderPipeline &pipeline) {
    DX11RenderPipelineData *p = static_cast<DX11RenderPipelineData *>(pipeline.data);

    p->rasterizer_state->Release();
    p->input_layout->Release();
    p->depth_stencil_view->Release();
    reinterpret_cast<ID3D11VertexShader *>(pipeline.vertex_shader)->Release();
    reinterpret_cast<ID3D11PixelShader *>(pipeline.pixel_shader)->Release();

    dx11_state.pipelines_pool.free(p);

    memset(&pipeline, 0, sizeof(pipeline));
}

void DX11Renderer::set_render_pipeline(const kai::RenderPipeline &pipeline) const {
    DX11DeviceData *d = static_cast<DX11DeviceData *>(data);
    DX11RenderPipelineData *p = static_cast<DX11RenderPipelineData *>(pipeline.data);

    d->context->RSSetState(p->rasterizer_state);
    d->context->VSSetShader(reinterpret_cast<ID3D11VertexShader *>(pipeline.vertex_shader), nullptr, 0);
    d->context->PSSetShader(reinterpret_cast<ID3D11PixelShader *>(pipeline.pixel_shader), nullptr, 0);
    d->context->IASetInputLayout(p->input_layout);
    d->context->IASetPrimitiveTopology(p->topology);

    d->context->OMSetRenderTargets(1, &d->render_target_view, p->depth_stencil_view);

    dx11_state.active_pipeline = p;
}

bool DX11Renderer::create_buffer(const kai::RenderBufferInfo &info, kai::RenderBuffer &out_buffer) const {
    D3D11_BUFFER_DESC buffer_desc = {};

    buffer_desc.ByteWidth = static_cast<UINT>(info.byte_size);
    buffer_desc.Usage = [usage = info.resource_usage]() {
        switch(usage) {
            case kai::RenderResourceUsage::gpu_r: return D3D11_USAGE_IMMUTABLE;
            case kai::RenderResourceUsage::cpu_w_gpu_r: return D3D11_USAGE_DYNAMIC;
            case kai::RenderResourceUsage::cpu_rw_gpu_rw: return D3D11_USAGE_STAGING;
            case kai::RenderResourceUsage::gpu_rw: default: return D3D11_USAGE_DEFAULT;
        }
    }();
    buffer_desc.BindFlags = [type = info.type]() {
        switch(type) {
            case kai::RenderBufferInfo::Type::index_buffer: return D3D11_BIND_INDEX_BUFFER;
            case kai::RenderBufferInfo::Type::constant_buffer: return D3D11_BIND_CONSTANT_BUFFER;
            case kai::RenderBufferInfo::Type::vertex_buffer: default: return D3D11_BIND_VERTEX_BUFFER;
        }
    }();
    buffer_desc.CPUAccessFlags = [usage = info.cpu_usage]() {
        switch(usage) {
            case kai::RenderCPUUsage::read: return D3D11_CPU_ACCESS_READ;
            case kai::RenderCPUUsage::write: return D3D11_CPU_ACCESS_WRITE;
            case kai::RenderCPUUsage::read_write: return static_cast<D3D11_CPU_ACCESS_FLAG>(D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE);
            case kai::RenderCPUUsage::none: default: return static_cast<D3D11_CPU_ACCESS_FLAG>(0);
        }
    }();

    D3D11_SUBRESOURCE_DATA subresource = {};
    subresource.pSysMem = info.data;

    ID3D11Buffer *buffer_data;
    if(static_cast<DX11DeviceData *>(data)->device->CreateBuffer(&buffer_desc, &subresource, &buffer_data) != S_OK) {
        return false;
    }

    out_buffer.data = buffer_data;
    out_buffer.stride = info.stride;

    return true;
}

void DX11Renderer::destroy_buffer(kai::RenderBuffer &buffer) {
    static_cast<ID3D11Buffer *>( buffer.data )->Release();
    memset(&buffer, 0, sizeof(buffer));
}

void init_dx11(void) {
    if(!dx11_state.factory) {
        if(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void **>(&dx11_state.factory)) != S_OK) {
            kai::log("Error: failed creating the IDXGIFactory!\n");
            return;
        }

        dx11_state.devices_pool = kai::PoolAllocator(sizeof(DX11DeviceData), DX11_DEVICE_POOL_COUNT);
        dx11_state.pipelines_pool = kai::PoolAllocator(sizeof(DX11RenderPipelineData), DX11_RENDER_PIPELINE_POOL_COUNT);
    }
}

void destroy_dx11(void) {
#define DESTROY_IF_NEEDED(item) \
    if(dx11_state.item) { \
        dx11_state.item->Release(); \
        dx11_state.item = nullptr; \
    }

    DESTROY_IF_NEEDED(factory);

    dx11_state.devices_pool.destroy();
    dx11_state.pipelines_pool.destroy();

#undef DESTROY_IF_NEEDED
}
