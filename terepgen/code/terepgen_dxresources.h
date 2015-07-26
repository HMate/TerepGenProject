#if !defined(TEREPGEN_DXRESOURCES_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/
#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "terepgen_types.h"


#define ERRMSGBUFFERSIZE 256

struct dx_resource
{
    IDXGISwapChain *SwapChain = nullptr;
    ID3D11Device *Device = nullptr;
    ID3D11DeviceContext *DeviceContext = nullptr;
    ID3D11RenderTargetView *BackBuffer = nullptr;
    ID3D11VertexShader *VertexShader = nullptr;
    ID3D11PixelShader *PixelShader = nullptr;
    ID3D11InputLayout *InputLayout = nullptr;
    ID3D11DepthStencilView *DepthStencilView = nullptr;
    ID3D11Texture2D *DepthStencilBuffer = nullptr;
    ID3D11DepthStencilState *DepthStencilState = nullptr;   
    ID3D11RasterizerState *RasterizerState = nullptr;
        
    int32 VideoCardMemory;
    char VideoCardDescription[128];
    
    HRESULT Initialize(HWND Window, uint32 ScreenWidth, uint32 ScreenHeight)
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
        
        for(int32 i = 0; i < NumModes; ++i)
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
        
        // NOTE: Create RasterizerState
        // TODO: This should be per object, and maybe a default one here.
        D3D11_RASTERIZER_DESC RSDesc;
        ZeroMemory(&RSDesc, sizeof(D3D11_RASTERIZER_DESC));
        RSDesc.FillMode = D3D11_FILL_SOLID; //D3D11_FILL_WIREFRAME D3D11_FILL_SOLID 
        RSDesc.CullMode = D3D11_CULL_BACK; //D3D11_CULL_NONE D3D11_CULL_FRONT D3D11_CULL_BACK
        //RSDesc.DepthClipEnable = false;
        //RSDesc.FrontCounterClockwise = true; // NOTE: false means clockwise triangles
        Device->CreateRasterizerState(&RSDesc, &RasterizerState);
        
        DeviceContext->RSSetState(RasterizerState);
        
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
            {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };
        HResult = Device->CreateInputLayout(ElementDesc, ArrayCount(ElementDesc), 
                    BlobVs->GetBufferPointer(), BlobVs->GetBufferSize(), &InputLayout);
        BlobVs->Release();
        if(FAILED(HResult)) return HResult;
        DeviceContext->IASetInputLayout(InputLayout);
        
        return HResult;
    }

    void LoadResource(ID3D11Resource *Buffer, void *Resource, uint32 ResourceSize)
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
            std::string ErrMsg(GetDebugMessage(HResult));
            OutputDebugStringA(("[TEREPGEN_DEBUG] Load Resource failed: " + ErrMsg).c_str());
        }
        memcpy(MappedSubresource.pData, Resource, ResourceSize);                 
        DeviceContext->Unmap(Buffer, NULL);
        
        // DeviceContext->UpdateSubresource(Buffer, 0, NULL, Resource, 0, ResourceSize);
    } 
    
    void LoadFrameFirstVertexBuffer(ID3D11Resource *VBuffer, void *Resource, uint32 ResourceSize)
    {
        HRESULT HResult;
        D3D11_MAPPED_SUBRESOURCE MappedSubresource;
        HResult = DeviceContext->Map(VBuffer, NULL,
                        D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
        if(FAILED(HResult))
        {
            std::string ErrMsg(GetDebugMessage(HResult));
            OutputDebugStringA(("[TEREPGEN_DEBUG] Load Resource failed: " + ErrMsg).c_str());
        }
        memcpy(MappedSubresource.pData, Resource, ResourceSize);                 
        DeviceContext->Unmap(VBuffer, NULL);
    }
    
    void LoadVertexBuffer(ID3D11Resource *VBuffer, void *Resource, uint32 ResourceSize)
    {
        HRESULT HResult;
        D3D11_MAPPED_SUBRESOURCE MappedSubresource;
        // TODO: Conscutive calls should use this: D3D11_MAP_WRITE_NO_OVERWRITE
        HResult = DeviceContext->Map(VBuffer, NULL,
                        D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
        if(FAILED(HResult))
        {
            std::string ErrMsg(GetDebugMessage(HResult));
            OutputDebugStringA(("[TEREPGEN_DEBUG] Load Resource failed: " + ErrMsg).c_str());
        }
        memcpy(MappedSubresource.pData, Resource, ResourceSize);                 
        DeviceContext->Unmap(VBuffer, NULL);
    }

    void Release()
    {     
        if(SwapChain) SwapChain->SetFullscreenState(false, NULL);
        if(VertexShader) VertexShader->Release();
        if(PixelShader) PixelShader->Release();
        if(DepthStencilView) DepthStencilView->Release();
        if(DepthStencilBuffer) DepthStencilBuffer->Release();
        if(DepthStencilState) DepthStencilState->Release();
        if(RasterizerState) RasterizerState->Release();
        if(BackBuffer) BackBuffer->Release();
        if(Device) Device->Release();
        if(DeviceContext) DeviceContext->Release();
        if(SwapChain) SwapChain->Release();
    }
    
    HRESULT Resize(uint32 ScreenWidth, uint32 ScreenHeight)
    {      
        HRESULT HResult = S_OK;
        if(SwapChain && ScreenWidth && ScreenHeight)
        {
#if TEREPGEN_DEBUG
            OutputDebugStringA(("[TEREPGEN_DEBUG] Resize Width:" + std::to_string(ScreenWidth) + "\n").c_str());
            OutputDebugStringA(("[TEREPGEN_DEBUG] Resize Height:" + std::to_string(ScreenHeight) + "\n").c_str());
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
            Viewport.Width = ScreenWidth;
            Viewport.Height = ScreenHeight;
            Viewport.MinDepth = 0.0f;
            Viewport.MaxDepth = 1.0f;
            Viewport.TopLeftX = 0;
            Viewport.TopLeftY = 0;
            DeviceContext->RSSetViewports( 1, &Viewport);
        }
        return HResult;
    }
    
    char* GetDebugMessage(DWORD dwErrorMsgId)
    {
        DWORD ret;        // Temp space to hold a return value.
        HINSTANCE hInst;  // Instance handle for DLL.
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
};


struct scene_constants
{
    DirectX::XMFLOAT4X4 ViewProjMx;
    DirectX::XMFLOAT4X4 ViewMx;
};

struct object_constants
{
	DirectX::XMFLOAT4X4 WorldMatrix;
};

using namespace DirectX;

struct camera
{
    XMFLOAT3 Position = XMFLOAT3(0, 0, 0);
    XMFLOAT3 TargetPos = XMFLOAT3(0, 0, 1);
    XMFLOAT3 UpDirection = XMFLOAT3(0, 1, 0);
    real32 CameraSpeed = 1.6f;
    
    real32 Fov = 3.14f * 0.35f;
    
    XMFLOAT4X4 ViewMx;
    XMFLOAT4X4 ProjMx;
    XMFLOAT4X4 ViewProjMx;
    
    ID3D11Buffer *SceneConstantBuffer;
    scene_constants SceneConstants;
    
    void Initialize(dx_resource &DXResources, screen_info Screen)
    {   
        XMStoreFloat4x4(&ViewMx, XMMatrixLookAtLH(XMLoadFloat3(&Position),
            XMLoadFloat3(&TargetPos), XMLoadFloat3(&UpDirection)));            
#if TEREPGEN_DEBUG
        OutputDebugStringA(("[TEREPGEN_DEBUG] Camera Width:" + std::to_string(Screen.Width) + "\n").c_str());
        OutputDebugStringA(("[TEREPGEN_DEBUG] Camera Height:" + std::to_string(Screen.Height) + "\n").c_str());
        OutputDebugStringA(("[TEREPGEN_DEBUG] Camera ApectRatio:" + std::to_string((real32)Screen.Width/Screen.Height) + "\n").c_str());
#endif  
        XMStoreFloat4x4(&ProjMx, 
            XMMatrixPerspectiveFovLH(Fov, (real32)Screen.Width/Screen.Height, 1.0f, 2000.0f));
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
        
        DXResources.Device->CreateBuffer(&SceneCBDesc, &SceneCBufferData, 
                                   &SceneConstantBuffer);
        DXResources.DeviceContext->VSSetConstantBuffers(0, 1, &SceneConstantBuffer);
    }
    
    void Release()
    {
        SceneConstantBuffer->Release();
    }
    
    void Update(input &Input)
    {
        XMFLOAT3 dCameraPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
        XMVECTOR TargetDirection =  XMLoadFloat3(&TargetPos) - XMLoadFloat3(&Position);
        if(Input.SpeedUp) 
        {
            if(CameraSpeed < 0.001f) 
                CameraSpeed = 0.1f;
            else CameraSpeed *= 1.2f;
            Input.SpeedUp = false;
        }
        if(Input.SpeedDown) 
        {
            CameraSpeed *= 0.9f;
            Input.SpeedDown = false;
        }
        // NOTE: Gather where to move with camera
        if(Input.MoveForward) 
        {
            XMStoreFloat3(&dCameraPos, XMLoadFloat3(&dCameraPos) + (TargetDirection * CameraSpeed));
        }
        if(Input.MoveBack) 
        {
            XMStoreFloat3(&dCameraPos, XMLoadFloat3(&dCameraPos) - (TargetDirection * CameraSpeed));
        }
        if(Input.MoveLeft) 
        {
            XMStoreFloat3(&dCameraPos, 
                XMLoadFloat3(&dCameraPos) -
                XMVector3Cross( XMLoadFloat3(&UpDirection), TargetDirection) * CameraSpeed);
        }
        if(Input.MoveRight) 
        {
            XMStoreFloat3(&dCameraPos, 
                XMLoadFloat3(&dCameraPos) +
                XMVector3Cross( XMLoadFloat3(&UpDirection), TargetDirection) * CameraSpeed);
        }
        if(Input.MoveUp) 
        {
            XMStoreFloat3(&dCameraPos, 
                XMLoadFloat3(&dCameraPos) + XMLoadFloat3(&UpDirection) * CameraSpeed);
        }
        if(Input.MoveDown) 
        {
            XMStoreFloat3(&dCameraPos, 
                XMLoadFloat3(&dCameraPos) - XMLoadFloat3(&UpDirection) * CameraSpeed);
        }
        
        // NOTE: move camera in pressed directions
        XMStoreFloat3(&Position,
            XMLoadFloat3(&Position) + XMLoadFloat3(&dCameraPos));
        XMStoreFloat3(&TargetPos,
            XMLoadFloat3(&TargetPos) + XMLoadFloat3(&dCameraPos));
        
        bool32 MouseLeftIsDown = GetKeyState(VK_LBUTTON) & (1 << 15);
        bool32 MouseRightIsDown = GetKeyState(VK_RBUTTON) & (1 << 15);
        
        // NOTE: rotate camera
        auto dMouseX = Input.MouseX - Input.OldMouseX;
        auto dMouseY = Input.MouseY - Input.OldMouseY;
        if(MouseLeftIsDown && (dMouseX != 0 || dMouseY != 0))
        {
            XMVECTOR NewTargetDir = TargetDirection;
            NewTargetDir = NewTargetDir + 
                XMVector3Cross( XMLoadFloat3(&UpDirection), TargetDirection) * dMouseX + 
                XMLoadFloat3(&UpDirection) * dMouseY;
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
    
    void Resize(screen_info Screen)
    {
        if(Screen.Width && Screen.Height)
        {
#if TEREPGEN_DEBUG
            OutputDebugStringA(("[TEREPGEN_DEBUG] Camera Width:" + std::to_string(Screen.Width) + "\n").c_str());
            OutputDebugStringA(("[TEREPGEN_DEBUG] Camera Height:" + std::to_string(Screen.Height) + "\n").c_str());
            OutputDebugStringA(("[TEREPGEN_DEBUG] Camera ApectRatio:" + std::to_string((real32)Screen.Width/Screen.Height) + "\n").c_str());
#endif        
            XMStoreFloat4x4(&ProjMx, 
                XMMatrixPerspectiveFovLH(Fov, (real32)Screen.Width/Screen.Height, 1.0f, 2000.0f));
            XMStoreFloat4x4(&ViewProjMx,
                XMMatrixMultiplyTranspose(XMLoadFloat4x4(&ViewMx), XMLoadFloat4x4(&ProjMx)));
        }
    }
};

#define TEREPGEN_DXRESOURCES_H
#endif