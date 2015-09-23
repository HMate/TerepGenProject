
#include "terepgen_dx_renderer.h"

HRESULT dx_resource::Initialize(HWND Window, uint32 ScreenWidth, uint32 ScreenHeight)
{
    HRESULT HResult;
    
    // NOTE: Query hardware specs
    IDXGIFactory *Factory;
    IDXGIAdapter *Adapter;
    IDXGIOutput *AdapterOutput;
    DXGI_ADAPTER_DESC AdapterDesc;
    uint32 NumModes, Numerator, Denominator;
    size_t StringLength;
    
    HResult = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&Factory);
    if(FAILED(HResult))
        return HResult;
    HResult = Factory->EnumAdapters(0, &Adapter);
    if(FAILED(HResult))
        return HResult;
    HResult = Adapter->EnumOutputs(0, &AdapterOutput);
    if(FAILED(HResult))
        return HResult;
    HResult = AdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_ENUM_MODES_INTERLACED, &NumModes, NULL);
    if(FAILED(HResult))
        return HResult;
    
    DXGI_MODE_DESC *DisplayModeList = new DXGI_MODE_DESC[NumModes];
    if(!DisplayModeList)
        return E_ABORT;
    
    HResult = AdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_ENUM_MODES_INTERLACED, &NumModes, DisplayModeList);
    if(FAILED(HResult))
        return HResult;
    
    for(uint32 i = 0; i < NumModes; ++i)
    {
        if(DisplayModeList[i].Width == (uint32)ScreenWidth)
        {
            if(DisplayModeList[i].Height == (uint32)ScreenHeight)
            {
                Numerator = DisplayModeList[i].RefreshRate.Numerator;
                Denominator = DisplayModeList[i].RefreshRate.Denominator;
            }
        }
    }
    
    HResult = Adapter->GetDesc(&AdapterDesc);
    if(FAILED(HResult))
        return HResult;
    
    VideoCardMemory = (int32)(AdapterDesc.DedicatedVideoMemory / 1024 / 1024);
    wcstombs_s(&StringLength, VideoCardDescription, 128, AdapterDesc.Description, 128);
    
    delete[] DisplayModeList;
    AdapterOutput->Release();
    Adapter->Release();
    Factory->Release();
    
    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    ZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));
    SwapChainDesc.BufferCount = 1;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferDesc.Width = ScreenWidth;
    SwapChainDesc.BufferDesc.Height = ScreenHeight;
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0; // No VSync
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDesc.OutputWindow = Window;
    SwapChainDesc.SampleDesc.Count = 4;
    SwapChainDesc.Windowed = true;
    
    D3D_FEATURE_LEVEL FeatureLevels[4] = {D3D_FEATURE_LEVEL_11_1,
                                          D3D_FEATURE_LEVEL_11_0,
                                          D3D_FEATURE_LEVEL_10_1,
                                          D3D_FEATURE_LEVEL_10_0};
    D3D_FEATURE_LEVEL UsedFeatureLevel;
    
    uint32 DeviceFlags = NULL;
#if TEREPGEN_DEBUG
    OutputDebugStringA("[TEREPGEN_DEBUG]Device and SwapChain are created with DEBUG flag\n");
    // NOTE: D3D11_CREATE_DEVICE_DEBUGGABLE flag does not work before win8.1 
    DeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif
    HResult = D3D11CreateDeviceAndSwapChain(NULL,
                                           D3D_DRIVER_TYPE_HARDWARE,
                                           NULL,
                                           DeviceFlags,
                                           (D3D_FEATURE_LEVEL *)FeatureLevels,
                                           ArrayCount(FeatureLevels),
                                           D3D11_SDK_VERSION,
                                           &SwapChainDesc,
                                           &SwapChain,
                                           &Device,
                                           &UsedFeatureLevel,
                                           &DeviceContext);
    if(FAILED(HResult))
        return HResult;
    
    D3D11_VIEWPORT Viewport;
    ZeroMemory(&Viewport, sizeof(D3D11_VIEWPORT));

    Viewport.TopLeftX = 0.0f;
    Viewport.TopLeftY = 0.0f;
    Viewport.Width = (real32)ScreenWidth;
    Viewport.Height = (real32)ScreenHeight;
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    DeviceContext->RSSetViewports(1, &Viewport);
    
    // NOTE: Create Render Target View
    ID3D11Texture2D *BackBufferTexture;
    HResult = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&BackBufferTexture);
    if(FAILED(HResult))
        return HResult;
    
    HResult = Device->CreateRenderTargetView(BackBufferTexture, 0, &BackBuffer);
    BackBufferTexture->Release();
    if(FAILED(HResult))
        return HResult;        
    
    // NOTE: Create Depth Stencil
    // NOTE: SampleDesc have to be the same as the SwapChain's, or nothing will render 
    D3D11_TEXTURE2D_DESC DepthStencilDesc;
    DepthStencilDesc.Width     = ScreenWidth;
    DepthStencilDesc.Height    = ScreenHeight;
    DepthStencilDesc.MipLevels = 1;
    DepthStencilDesc.ArraySize = 1;
    DepthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthStencilDesc.SampleDesc.Count   = 4;
    DepthStencilDesc.SampleDesc.Quality = 0;
    DepthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
    DepthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
    DepthStencilDesc.CPUAccessFlags = 0; 
    DepthStencilDesc.MiscFlags      = 0;
    
    HResult = Device->CreateTexture2D(&DepthStencilDesc, nullptr, &DepthStencilBuffer);
    if(FAILED(HResult)) return HResult;
    HResult = Device->CreateDepthStencilView(DepthStencilBuffer, nullptr, &DepthStencilView);
    if(FAILED(HResult)) return HResult;
    
    DeviceContext->OMSetRenderTargets(1, &BackBuffer, DepthStencilView);
    
    // NOTE: Create DepthStencilState
    D3D11_DEPTH_STENCIL_DESC dsDesc;

    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable = false;
    dsDesc.StencilReadMask = 0xFF;
    dsDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Create depth stencil state
    Device->CreateDepthStencilState(&dsDesc, &DepthStencilState);
    DeviceContext->OMSetDepthStencilState(DepthStencilState, 1);
            
    // NOTE: Compile Shaders
    ID3D10Blob *BlobVs, *BlobPs;
    ID3D10Blob *BlobError = nullptr;
    
    HResult = D3DCompileFromFile(L"shaders.hlsl", 0, 0, "VShader", "vs_4_0", 0, 0, &BlobVs, &BlobError);
    if(FAILED(HResult))
    {
        if(BlobError != nullptr)
        {
            OutputDebugStringA((char*)BlobError->GetBufferPointer());
            MessageBox(Window, (char*)BlobError->GetBufferPointer(),	
                "Error in vertex shader", MB_OK | MB_ICONERROR);
            BlobError->Release();
        }
        return HResult;
    }
    Device->CreateVertexShader(BlobVs->GetBufferPointer(), BlobVs->GetBufferSize(), 0, &VertexShader);
    
    HResult = D3DCompileFromFile(L"shaders.hlsl", 0, 0, "PShader", "ps_4_0", 0, 0, &BlobPs, &BlobError);
    if(FAILED(HResult))
    {
        if(BlobError != nullptr)
        {
            OutputDebugStringA((char*)BlobError->GetBufferPointer());
            MessageBox(Window, (LPCSTR)BlobError->GetBufferPointer(),	
                "Error in pixel shader", MB_OK | MB_ICONERROR);
            BlobError->Release();
        }
        return HResult;
    }
    Device->CreatePixelShader(BlobPs->GetBufferPointer(), BlobPs->GetBufferSize(), 0, &PixelShader);
    BlobPs->Release();
    
    DeviceContext->VSSetShader(VertexShader, 0, 0);
    DeviceContext->PSSetShader(PixelShader, 0, 0);
    
    // NOTE: Create Input Layout
    D3D11_INPUT_ELEMENT_DESC ElementDesc[] = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    HResult = Device->CreateInputLayout(ElementDesc, ArrayCount(ElementDesc), 
                BlobVs->GetBufferPointer(), BlobVs->GetBufferSize(), &InputLayout);
    BlobVs->Release();
    if(FAILED(HResult)) return HResult;
    DeviceContext->IASetInputLayout(InputLayout);
    
    return HResult;
}

void dx_resource::LoadResource(ID3D11Resource *Buffer, void *Resource, uint32 ResourceSize)
{
    // NOTE: This kind of resource mapping is optimized for per frame updating 
    //      for resources with D3D11_USAGE_DYNAMIC
    // SOURCE: https://msdn.microsoft.com/en-us/library/windows/desktop/ff476259%28v=vs.85%29.aspx
    HRESULT HResult;
    D3D11_MAPPED_SUBRESOURCE MappedSubresource;
    HResult = DeviceContext->Map(Buffer, NULL,
                    D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
    if(FAILED(HResult))
    {
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Load Resource failed: %s\n", GetDebugMessage(HResult));
        OutputDebugStringA(DebugBuffer);
    }
    memcpy(MappedSubresource.pData, Resource, ResourceSize);                 
    DeviceContext->Unmap(Buffer, NULL);
    
    // DeviceContext->UpdateSubresource(Buffer, 0, NULL, Resource, 0, ResourceSize);
} 

void dx_resource::LoadFrameFirstVertexBuffer(ID3D11Resource *VBuffer, void *Resource, uint32 ResourceSize)
{
    HRESULT HResult;
    D3D11_MAPPED_SUBRESOURCE MappedSubresource;
    HResult = DeviceContext->Map(VBuffer, NULL,
                    D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
    if(FAILED(HResult))
    {
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Load Resource failed: %s\n", GetDebugMessage(HResult));
        OutputDebugStringA(DebugBuffer);
    }
    memcpy(MappedSubresource.pData, Resource, ResourceSize);                 
    DeviceContext->Unmap(VBuffer, NULL);
}

void dx_resource::LoadVertexBuffer(ID3D11Resource *VBuffer, void *Resource, uint32 ResourceSize)
{
    HRESULT HResult;
    D3D11_MAPPED_SUBRESOURCE MappedSubresource;
    // TODO: Conscutive calls should use this: D3D11_MAP_WRITE_NO_OVERWRITE
    HResult = DeviceContext->Map(VBuffer, NULL,
                    D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
    if(FAILED(HResult))
    {
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Load Resource failed: %s\n", GetDebugMessage(HResult));
        OutputDebugStringA(DebugBuffer);
    }
    memcpy(MappedSubresource.pData, Resource, ResourceSize);                 
    DeviceContext->Unmap(VBuffer, NULL);
}

void dx_resource::Release()
{     
    if(SwapChain) 
    {
        SwapChain->SetFullscreenState(false, NULL);
    }
    if(VertexShader) 
    {
        VertexShader->Release();
        VertexShader = nullptr;
    }
    if(PixelShader) 
    {
        PixelShader->Release();
        PixelShader = nullptr;
    }
    if(DepthStencilView) 
    {
        DepthStencilView->Release();
        DepthStencilView = nullptr;
    }
    if(DepthStencilBuffer) 
    {
        DepthStencilBuffer->Release();
        DepthStencilBuffer = nullptr;
    }
    if(DepthStencilState) 
    {
        DepthStencilState->Release();
        DepthStencilState = nullptr;
    }
    if(BackBuffer) 
    {
        BackBuffer->Release();
        BackBuffer = nullptr;
    }
    if(Device) 
    {
        Device->Release();
        Device = nullptr;
    }
    if(DeviceContext) 
    {
        DeviceContext->Release();
        DeviceContext = nullptr;
    }
    if(SwapChain) 
    {
        SwapChain->Release();
        SwapChain = nullptr;
    }
}

HRESULT dx_resource::Resize(uint32 ScreenWidth, uint32 ScreenHeight)
{      
    HRESULT HResult = S_OK;
    if(SwapChain && ScreenWidth && ScreenHeight)
    {
#if TEREPGEN_DEBUG
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Resize Width: %d\n", ScreenWidth);
        OutputDebugStringA(DebugBuffer);
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Resize Height: %d\n", ScreenHeight);
        OutputDebugStringA(DebugBuffer);
#endif  
        DeviceContext->OMSetRenderTargets(0, 0, 0);

        BackBuffer->Release();

        SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
                                        
        ID3D11Texture2D* BackBufferTexture;
        HResult = SwapChain->GetBuffer(0, __uuidof( ID3D11Texture2D), (void**) &BackBufferTexture);
        if(FAILED(HResult)) return HResult;
        HResult = Device->CreateRenderTargetView(BackBufferTexture, NULL, &BackBuffer);
        BackBufferTexture->Release();
        if(FAILED(HResult)) return HResult;
        
        if(DepthStencilBuffer) DepthStencilBuffer->Release();
        if(DepthStencilState) DepthStencilState->Release();
        D3D11_TEXTURE2D_DESC DepthStencilDesc;
        DepthStencilDesc.Width     = ScreenWidth;
        DepthStencilDesc.Height    = ScreenHeight;
        DepthStencilDesc.MipLevels = 1;
        DepthStencilDesc.ArraySize = 1;
        DepthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
        DepthStencilDesc.SampleDesc.Count   = 4;
        DepthStencilDesc.SampleDesc.Quality = 0;
        DepthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
        DepthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
        DepthStencilDesc.CPUAccessFlags = 0; 
        DepthStencilDesc.MiscFlags      = 0;
        
        HResult = Device->CreateTexture2D(&DepthStencilDesc, nullptr, &DepthStencilBuffer);
        if(FAILED(HResult)) return HResult;
        HResult = Device->CreateDepthStencilView(DepthStencilBuffer, nullptr, &DepthStencilView);
        if(FAILED(HResult)) return HResult;

        DeviceContext->OMSetRenderTargets(1, &BackBuffer, DepthStencilView);

        D3D11_VIEWPORT Viewport;
        Viewport.Width = (real32)ScreenWidth;
        Viewport.Height = (real32)ScreenHeight;
        Viewport.MinDepth = 0.0f;
        Viewport.MaxDepth = 1.0f;
        Viewport.TopLeftX = 0.0f;
        Viewport.TopLeftY = 0.0f;
        DeviceContext->RSSetViewports( 1, &Viewport);
    }
    return HResult;
}

char* dx_resource::GetDebugMessage(DWORD dwErrorMsgId)
{
    DWORD ret;        // Temp space to hold a return value.
    char* pBuffer;   // Buffer to hold the textual error description.
    
    ret = FormatMessage(  
        FORMAT_MESSAGE_ALLOCATE_BUFFER | // The function will allocate space for pBuffer.
        FORMAT_MESSAGE_FROM_SYSTEM | // System wide message.
        FORMAT_MESSAGE_IGNORE_INSERTS, // No inserts.
        NULL, // Message is not in a module.
        dwErrorMsgId, // Message identifier.
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language.
        (LPTSTR)&pBuffer, // Buffer to hold the text string.
        ERRMSGBUFFERSIZE, // The function will allocate at least this much for pBuffer.
        NULL // No inserts.
    );
    
    return pBuffer;
}

// ****
// **** RENDERER
// ****

terrain_renderer::terrain_renderer()
{
    DXReleased = false;
    VertexBuffer = nullptr;
    ObjectConstantBuffer = nullptr;
}

internal HRESULT 
LoadJPGFromFile(dx_resource *DXResources, char *Filename, ID3D11ShaderResourceView **ShaderResView)
{
    HRESULT HResult = E_FAIL;
    
    int32 ImgHeight;
    int32 ImgWidth;
    int32 SourcePxByteSize;
    uint32 DestPxByteSize = 4;
    uint8 *LoadedBitmap = stbi_load(Filename, &ImgWidth, &ImgHeight, &SourcePxByteSize, DestPxByteSize);
    if(LoadedBitmap)
    {
        // NOTE: Get image info
        D3D11_TEXTURE2D_DESC TextureDesc;
        TextureDesc.Height = ImgHeight;
        TextureDesc.Width = ImgWidth;
        TextureDesc.MipLevels = 0;
        TextureDesc.ArraySize = 1;
        TextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        TextureDesc.SampleDesc.Count = 1;
        TextureDesc.SampleDesc.Quality = 0;
        TextureDesc.Usage = D3D11_USAGE_DEFAULT;
        TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        TextureDesc.CPUAccessFlags = 0;
        TextureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        
        ID3D11Texture2D* Tex;
        HResult = DXResources->Device->CreateTexture2D(&TextureDesc, NULL, &Tex);
        if(FAILED(HResult)) 
        {
            stbi_image_free(LoadedBitmap);
            return HResult;
        }
        
        // NOTE: Should use UpdateSubresource, if the resource isnt loaded every frame
        uint32 RowPitch = ImgWidth * DestPxByteSize;
        DXResources->DeviceContext->UpdateSubresource(Tex, 0, NULL, LoadedBitmap, RowPitch, 0);
        
        D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc;
        SrvDesc.Format = TextureDesc.Format;
        SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        SrvDesc.Texture2D.MostDetailedMip = 0;
        SrvDesc.Texture2D.MipLevels = (unsigned int)-1;
        
        HResult = DXResources->Device->CreateShaderResourceView(Tex, &SrvDesc, ShaderResView);
        if(FAILED(HResult))
        {
            stbi_image_free(LoadedBitmap);
            return HResult;
        }

        // NOTE: Generate mipmaps for this texture.
        DXResources->DeviceContext->GenerateMips(*ShaderResView);
        
        stbi_image_free(LoadedBitmap);
    }
    
    return HResult;
}

HRESULT terrain_renderer::Initialize(dx_resource *DXResources)
{
    DXReleased = false;
    this->DXResource = DXResources;
    this->MaxVertexCount = RENDER_BLOCK_VERTEX_COUNT;
    ObjectConstants.WorldMatrix = XMFLOAT4X4(1, 0, 0, 0,
                                             0, 1, 0, 0,
                                             0, 0, 1, 0,
                                             0, 0, 0, 1);
                                           
    D3D11_BUFFER_DESC ObjectCBDesc = {};
    ObjectCBDesc.ByteWidth = sizeof( object_constants );
    ObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
    ObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ObjectCBDesc.MiscFlags = 0;
    ObjectCBDesc.StructureByteStride = 0;
    
    // D3D11_SUBRESOURCE_DATA ObjCBufferData;
    // ObjCBufferData.pSysMem = &ObjectConstants;
    // ObjCBufferData.SysMemPitch = 0;
    // ObjCBufferData.SysMemSlicePitch = 0;
    
    HRESULT HResult;
    HResult = DXResources->Device->CreateBuffer(&ObjectCBDesc, NULL, &ObjectConstantBuffer);
    if(FAILED(HResult)) return HResult;

    // NOTE: Create RasterizerStates
    D3D11_RASTERIZER_DESC RSDescDefault;
    ZeroMemory(&RSDescDefault, sizeof(D3D11_RASTERIZER_DESC));
    RSDescDefault.FillMode = D3D11_FILL_SOLID;
    RSDescDefault.CullMode = D3D11_CULL_BACK;
    HResult = DXResources->Device->CreateRasterizerState(&RSDescDefault, &RSDefault);
    if(FAILED(HResult)) return HResult;
    
    D3D11_RASTERIZER_DESC RSDescWireFrame;
    ZeroMemory(&RSDescWireFrame, sizeof(D3D11_RASTERIZER_DESC));
    RSDescWireFrame.FillMode = D3D11_FILL_WIREFRAME;
    RSDescWireFrame.CullMode = D3D11_CULL_NONE;
    HResult = DXResources->Device->CreateRasterizerState(&RSDescWireFrame, &RSWireFrame);
    if(FAILED(HResult)) return HResult;
    
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(BufferDesc));
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;  
    // TODO: Instead of MaxVertexCount, i should use the count from CreateRenderVertices here.
    // Its more precise, but then i cant change the number of vertices.
    // NOTE: above 120MB vx buffer size dx11 crashes
    // NOTE: at vertex xize = 48 B, MaxVC = 2621440 is 120MB
    // TODO: Need to profile if drawing is faster with lower buffer size.
    // BufferDesc.ByteWidth = sizeof(vertex) * 2621440; // Max buffer size at 48B vertices
    BufferDesc.ByteWidth = sizeof(vertex) * MaxVertexCount;        
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;   
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    Assert(BufferDesc.ByteWidth <= MEGABYTE(120))
#if TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] VertBuff Max Vertex Count: %d\n", MaxVertexCount);
    OutputDebugStringA(DebugBuffer);
#endif  

    HResult = DXResources->Device->CreateBuffer(&BufferDesc, NULL, &VertexBuffer);
    if(FAILED(HResult)) return HResult;
        
    HResult = LoadJPGFromFile(DXResources, "grass.jpg", &GrassTexture);
    if(FAILED(HResult)) return HResult;
    HResult = LoadJPGFromFile(DXResources, "lichen_rock_by_darlingstock.jpg", &RockTexture);
    if(FAILED(HResult)) return HResult;
    
    D3D11_SAMPLER_DESC SampDesc;
	ZeroMemory( &SampDesc, sizeof(SampDesc) );
	SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
    SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SampDesc.MinLOD = 0;
    SampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    HResult = DXResources->Device->CreateSamplerState(&SampDesc, &TexSamplerState);
    if(FAILED(HResult))
    {
        return HResult;
    }
    
    DXResources->DeviceContext->PSSetShaderResources(0, 1, &GrassTexture);
    DXResources->DeviceContext->PSSetShaderResources(1, 1, &RockTexture);
    DXResources->DeviceContext->PSSetSamplers(0, 1, &TexSamplerState);
    
    return HResult; 
}

void terrain_renderer::SetTransformations(v3 Translation)
{
    ObjectConstants.WorldMatrix = XMFLOAT4X4(1, 0, 0, Translation.X,
                                             0, 1, 0, Translation.Y,
                                             0, 0, 1, Translation.Z,
                                             0, 0, 0, 1);
}

void terrain_renderer::SetDrawModeDefault(void)
{
    DXResource->DeviceContext->RSSetState(RSDefault);
}

void terrain_renderer::SetDrawModeWireframe(void)
{
    DXResource->DeviceContext->RSSetState(RSWireFrame);
}

void terrain_renderer::DrawTriangles(vertex *Vertices, uint32 VertCount)
{
    DXResource->LoadResource(ObjectConstantBuffer, &ObjectConstants, sizeof(ObjectConstants));
    DXResource->DeviceContext->VSSetConstantBuffers(1, 1, &ObjectConstantBuffer); 
       
    uint32 stride = sizeof(vertex);
    uint32 offset = 0;
    DXResource->LoadVertexBuffer(VertexBuffer, Vertices, sizeof(vertex) * VertCount); 
    
    DXResource->DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    DXResource->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DXResource->DeviceContext->Draw(VertCount, 0);
}

internal vertex
Get3DVertex(v3 LocalPos, color Color)
{
//    real32 Scale = 1.0f;
    vertex Result = {LocalPos.X, LocalPos.Y, LocalPos.Z, 
                     0.0f, 1.0f, 0.0f,
                     Color};
    return Result;
}

void terrain_renderer::DrawDebugTriangle()
{       
    DXResource->DeviceContext->RSSetState(RSDefault);
    
    const uint32 FalseCount = 3;
    color Color{1.0f, 0.0f, 0.0f, 1.0f};
    vertex FalseVertices[FalseCount]={Get3DVertex(v3{1.0f , 0.55f, 1.0f}, Color),
                                      Get3DVertex(v3{-0.8f, -0.7f, 1.0f}, Color),
                                      Get3DVertex(v3{-1.0f, 0.0f , 1.0f}, Color)};
                                      
    DXResource->LoadResource(ObjectConstantBuffer, &ObjectConstants, sizeof(ObjectConstants));
    DXResource->DeviceContext->VSSetConstantBuffers(1, 1, &ObjectConstantBuffer); 
       
    uint32 stride = sizeof(vertex);
    uint32 offset = 0;
    DXResource->LoadVertexBuffer(VertexBuffer, FalseVertices, sizeof(vertex) * FalseCount);    
    
    DXResource->DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    DXResource->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DXResource->DeviceContext->Draw(FalseCount, 0);
}

void terrain_renderer::DrawAxis(real32 Size)
{      
    DXResource->DeviceContext->RSSetState(RSDefault);
    
    const uint32 VertCount = 6;
    color Red{1.0f, 0.0f, 0.0f, 1.0f},
          Green{0.0f, 1.0f, 0.0f, 1.0f}, 
          Blue{0.0f, 0.0f, 1.0f, 1.0f};
    vertex AxisVertices[VertCount]={Get3DVertex(v3{ 1.0f*Size,  0.0f,  0.0f}, Red),
                                    Get3DVertex(v3{-1.0f*Size,  0.0f,  0.0f}, Red),
                                    Get3DVertex(v3{ 0.0f,  1.0f*Size,  0.0f}, Green),
                                    Get3DVertex(v3{ 0.0f, -1.0f*Size,  0.0f}, Green),
                                    Get3DVertex(v3{ 0.0f,  0.0f,  1.0f*Size}, Blue),
                                    Get3DVertex(v3{ 0.0f,  0.0f, -1.0f*Size}, Blue)};
                                      
    DXResource->LoadResource(ObjectConstantBuffer, &ObjectConstants, sizeof(ObjectConstants));
    DXResource->DeviceContext->VSSetConstantBuffers(1, 1, &ObjectConstantBuffer); 
       
    uint32 stride = sizeof(vertex);
    uint32 offset = 0;
    DXResource->LoadFrameFirstVertexBuffer(VertexBuffer, AxisVertices, sizeof(vertex) * VertCount);    
    
    DXResource->DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    DXResource->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    DXResource->DeviceContext->Draw(VertCount, 0);
}

void terrain_renderer::Release()
{
    if(RSDefault) RSDefault->Release();
    if(RSWireFrame) RSWireFrame->Release();
    if(ObjectConstantBuffer) ObjectConstantBuffer->Release();
    if(VertexBuffer) VertexBuffer->Release();
    DXReleased = true;
}
   
terrain_renderer::~terrain_renderer()
{
    if(!DXReleased) Release();
}


// ****
// **** CAMERA
// ****

using namespace DirectX;

v3 camera::GetPos()
{
    v3 Result{Position.x, Position.y, Position.z};
    return Result;
}

v3 camera::GetLookDirection()
{
    v3 Result = {};
    XMVECTOR TargetDirection =  XMLoadFloat3(&TargetPos) - XMLoadFloat3(&Position);
    Result.X = XMVectorGetX(TargetDirection);
    Result.Y = XMVectorGetY(TargetDirection);
    Result.Z = XMVectorGetZ(TargetDirection);
    return Result;
}

void camera::Initialize(dx_resource *DXResources, uint32 ScreenWidth, uint32 ScreenHeight, real32 CamSpeed)
{   
    CameraSpeed = CamSpeed;
    XMStoreFloat4x4(&ViewMx, XMMatrixLookAtLH(XMLoadFloat3(&Position),
        XMLoadFloat3(&TargetPos), XMLoadFloat3(&UpDirection)));            
#if TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Camera Width: %d\n", ScreenWidth);
    OutputDebugStringA(DebugBuffer);
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Camera Height: %d\n", ScreenHeight);
    OutputDebugStringA(DebugBuffer);
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Camera ApectRatio: %f\n", (real32)ScreenWidth/ScreenHeight);
    OutputDebugStringA(DebugBuffer);
#endif  
    XMStoreFloat4x4(&ProjMx, 
        XMMatrixPerspectiveFovLH(Fov, (real32)ScreenWidth/ScreenHeight, NearClipZ, FarClipZ));
    XMStoreFloat4x4(&ViewProjMx,
        XMMatrixMultiplyTranspose(XMLoadFloat4x4(&ViewMx),
                                  XMLoadFloat4x4(&ProjMx)));
    
    XMFLOAT4X4 VMxTranspose;
    XMStoreFloat4x4(&VMxTranspose, XMMatrixTranspose(XMLoadFloat4x4(&ViewMx)));
    SceneConstants.ViewMx = VMxTranspose;
    SceneConstants.ViewProjMx = ViewProjMx;
    
    D3D11_BUFFER_DESC SceneCBDesc = {};
    SceneCBDesc.ByteWidth = sizeof( scene_constants );
    SceneCBDesc.Usage = D3D11_USAGE_DYNAMIC;
    SceneCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    SceneCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    SceneCBDesc.MiscFlags = 0;
    SceneCBDesc.StructureByteStride = 0;
    
    D3D11_SUBRESOURCE_DATA SceneCBufferData;
    SceneCBufferData.pSysMem = &SceneConstants;
    SceneCBufferData.SysMemPitch = 0;
    SceneCBufferData.SysMemSlicePitch = 0;
    
    DXResources->Device->CreateBuffer(&SceneCBDesc, &SceneCBufferData, 
                               &SceneConstantBuffer);
    DXResources->DeviceContext->VSSetConstantBuffers(0, 1, &SceneConstantBuffer);
}

void camera::Release()
{
    SceneConstantBuffer->Release();
}

void camera::Update(input *Input, real64 TimeDelta)
{
    XMFLOAT3 dCameraPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
    XMVECTOR TargetDirection =  XMLoadFloat3(&TargetPos) - XMLoadFloat3(&Position);
    if(Input->SpeedUp) 
    {
        if(CameraSpeed < 0.001f) 
            CameraSpeed = 0.1f;
        else CameraSpeed *= 1.2f;
        Input->SpeedUp = false;
    }
    if(Input->SpeedDown) 
    {
        CameraSpeed *= 0.9f;
        Input->SpeedDown = false;
    }
    real32 MoveDelta = CameraSpeed * (real32)TimeDelta;
    // NOTE: Gather where to move with camera
    if(Input->MoveForward) 
    {
        XMStoreFloat3(&dCameraPos, XMLoadFloat3(&dCameraPos) + (TargetDirection * MoveDelta));
    }
    if(Input->MoveBack) 
    {
        XMStoreFloat3(&dCameraPos, XMLoadFloat3(&dCameraPos) - (TargetDirection * MoveDelta));
    }
    if(Input->MoveLeft) 
    {
        XMStoreFloat3(&dCameraPos, 
            XMLoadFloat3(&dCameraPos) -
            XMVector3Cross( XMLoadFloat3(&UpDirection), TargetDirection) * MoveDelta);
    }
    if(Input->MoveRight) 
    {
        XMStoreFloat3(&dCameraPos, 
            XMLoadFloat3(&dCameraPos) +
            XMVector3Cross( XMLoadFloat3(&UpDirection), TargetDirection) * MoveDelta);
    }
    if(Input->MoveUp) 
    {
        XMStoreFloat3(&dCameraPos, 
            XMLoadFloat3(&dCameraPos) + XMLoadFloat3(&UpDirection) * MoveDelta);
    }
    if(Input->MoveDown) 
    {
        XMStoreFloat3(&dCameraPos, 
            XMLoadFloat3(&dCameraPos) - XMLoadFloat3(&UpDirection) * MoveDelta);
    }
    
    // NOTE: move camera in pressed directions
    XMStoreFloat3(&Position,
        XMLoadFloat3(&Position) + XMLoadFloat3(&dCameraPos));
    XMStoreFloat3(&TargetPos,
        XMLoadFloat3(&TargetPos) + XMLoadFloat3(&dCameraPos));
    
    bool32 MouseLeftIsDown = GetKeyState(VK_LBUTTON) & (1 << 15);
    //bool32 MouseRightIsDown = GetKeyState(VK_RBUTTON) & (1 << 15);
    
    // NOTE: rotate camera
    auto dMouseX = Input->MouseX - Input->OldMouseX;
    auto dMouseY = Input->MouseY - Input->OldMouseY;
    if(MouseLeftIsDown && (dMouseX != 0 || dMouseY != 0))
    {
        XMVECTOR NewTargetDir = TargetDirection;
        NewTargetDir = NewTargetDir + 
            XMVector3Cross( XMLoadFloat3(&UpDirection), TargetDirection) * (real32)dMouseX + 
            XMLoadFloat3(&UpDirection) * (real32)dMouseY;
        // NOTE: Divide by 30 is just to slow down the rotation/frame
        real32 Angle = XMScalarACos( XMVectorGetX(
            XMVector3Dot(NewTargetDir, TargetDirection) /
            (XMVector3Length(NewTargetDir) * XMVector3Length(TargetDirection)))) / 30.0f;
                
        XMVECTOR Axis = XMVector3Cross(NewTargetDir, TargetDirection);
        
        XMStoreFloat3(&TargetPos,
            XMVector3Transform(TargetDirection, XMMatrixRotationAxis(Axis, -Angle)) +
                XMLoadFloat3(&Position));
    }
              
    XMStoreFloat4x4(&ViewMx, XMMatrixLookAtLH(XMLoadFloat3(&Position),
                    XMLoadFloat3(&TargetPos), XMLoadFloat3(&UpDirection)));
    XMStoreFloat4x4(&ViewProjMx,
        XMMatrixMultiplyTranspose(XMLoadFloat4x4(&ViewMx), XMLoadFloat4x4(&ProjMx)));
        
    XMFLOAT4X4 VMxTranspose;
    XMStoreFloat4x4(&VMxTranspose, XMMatrixTranspose(XMLoadFloat4x4(&ViewMx)));
    SceneConstants.ViewMx = VMxTranspose;
    SceneConstants.ViewProjMx = ViewProjMx;
}
    
void camera::Resize(uint32 ScreenWidth, uint32 ScreenHeight)
{
    if(ScreenWidth && ScreenHeight)
    {
#if TEREPGEN_DEBUG
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Camera Width: %d\n", ScreenWidth);
        OutputDebugStringA(DebugBuffer);
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Camera Height: %d\n", ScreenHeight);
        OutputDebugStringA(DebugBuffer);
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Camera ApectRatio: %f\n", (real32)ScreenWidth/ScreenHeight);
        OutputDebugStringA(DebugBuffer);
#endif        
        XMStoreFloat4x4(&ProjMx, 
            XMMatrixPerspectiveFovLH(Fov, (real32)ScreenWidth/ScreenHeight, NearClipZ, FarClipZ));
        XMStoreFloat4x4(&ViewProjMx,
            XMMatrixMultiplyTranspose(XMLoadFloat4x4(&ViewMx), XMLoadFloat4x4(&ProjMx)));
    }
}