#if !defined(TEREPGEN_DXRESOURCES_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/
#include <d3dcompiler.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "terepgen_types.h"

using namespace DirectX;

struct scene_constants
{
    DirectX::XMFLOAT4X4 ViewProjMx;
};

struct object_constants
{
	DirectX::XMFLOAT4X4 WorldMatrix;
};

struct dx_resource
{
    IDXGISwapChain *SwapChain = nullptr;
    ID3D11Device *Device = nullptr;
    ID3D11DeviceContext *DeviceContext = nullptr;
    ID3D11RenderTargetView *BackBuffer = nullptr;
    ID3D11VertexShader *VertexShader = nullptr;
    ID3D11PixelShader *PixelShader = nullptr;
    ID3D11InputLayout *InputLayout = nullptr;
    
    HRESULT Initialize(HWND Window, uint32 ScreenWidth, uint32 ScreenHeight)
    {
        HRESULT HResult;
        
        DXGI_SWAP_CHAIN_DESC SwapChainDesc;
        ZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));
        SwapChainDesc.BufferCount = 1;
        SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        SwapChainDesc.OutputWindow = Window;
        SwapChainDesc.SampleDesc.Count = 4;
        SwapChainDesc.Windowed = true;
        
        D3D_FEATURE_LEVEL FeatureLevels[4] = {D3D_FEATURE_LEVEL_11_1,
                                              D3D_FEATURE_LEVEL_11_0,
                                              D3D_FEATURE_LEVEL_10_1,
                                              D3D_FEATURE_LEVEL_10_0};
        D3D_FEATURE_LEVEL UsedFeatureLevel;
        
        HResult = D3D11CreateDeviceAndSwapChain(NULL,
                                               D3D_DRIVER_TYPE_HARDWARE,
                                               NULL,
                                               NULL,
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
        
        ID3D11Texture2D *BackBufferTexture;
        HResult = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&BackBufferTexture);
        if(FAILED(HResult))
            return HResult;
        
        HResult = Device->CreateRenderTargetView(BackBufferTexture, 0, &BackBuffer);
        BackBufferTexture->Release();
        if(FAILED(HResult))
            return HResult;
        
        DeviceContext->OMSetRenderTargets(1, &BackBuffer, 0);
        
        D3D11_VIEWPORT Viewport;
        ZeroMemory(&Viewport, sizeof(D3D11_VIEWPORT));

        Viewport.TopLeftX = 0;
        Viewport.TopLeftY = 0;
        Viewport.Width = ScreenWidth;
        Viewport.Height = ScreenHeight;
        DeviceContext->RSSetViewports(1, &Viewport);
        
        ID3D10Blob *BlobVs, *BlobPs;
        ID3D10Blob *BlobError = nullptr;
        
        HResult = D3DCompileFromFile(L"shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, &BlobVs, &BlobError);
        if(FAILED(HResult))
        {
            if(BlobError != nullptr)
            {
                OutputDebugStringA((char*)BlobError->GetBufferPointer());
                BlobError->Release();
                return HResult;
            }
        }
        Device->CreateVertexShader(BlobVs->GetBufferPointer(), BlobVs->GetBufferSize(), 0, &VertexShader);
        
        HResult = D3DCompileFromFile(L"shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, &BlobPs, &BlobError);
        if(FAILED(HResult))
        {
            if(BlobError != nullptr)
            {
                OutputDebugStringA((char*)BlobError->GetBufferPointer());
                BlobError->Release();
                return HResult;
            }
        }
        Device->CreatePixelShader(BlobPs->GetBufferPointer(), BlobPs->GetBufferSize(), 0, &PixelShader);
        BlobPs->Release();
        
        DeviceContext->VSSetShader(VertexShader, 0, 0);
        DeviceContext->PSSetShader(PixelShader, 0, 0);
        
        
        D3D11_INPUT_ELEMENT_DESC ElementDesc[] = 
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };
        HResult = Device->CreateInputLayout(ElementDesc, ArrayCount(ElementDesc), 
                    BlobVs->GetBufferPointer(), BlobVs->GetBufferSize(), &InputLayout);
        BlobVs->Release();
        if(FAILED(HResult))
            return HResult;
        DeviceContext->IASetInputLayout(InputLayout);
        
        return HResult;
    }

    void LoadResource(ID3D11Buffer *Buffer, void *Resource, uint32 ResourceSize)
    {
        D3D11_MAPPED_SUBRESOURCE MappedSubresource;
        DeviceContext->Map(Buffer, NULL,
                        D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
        memcpy(MappedSubresource.pData, Resource, ResourceSize);                 
        DeviceContext->Unmap(Buffer, NULL);
    } 

    void Release()
    {     
        if(VertexShader) VertexShader->Release();
        if(PixelShader) PixelShader->Release();
        if(SwapChain) SwapChain->Release();
        if(BackBuffer) BackBuffer->Release();
        if(Device) Device->Release();
        if(DeviceContext) DeviceContext->Release();
    }
    
    void Resize(uint32 ScreenWidth, uint32 ScreenHeight)
    {
        if (SwapChain)
        {
            DeviceContext->OMSetRenderTargets(0, 0, 0);

            BackBuffer->Release();

            SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
                                            
            ID3D11Texture2D* BackBufferTexture;
            SwapChain->GetBuffer(0, __uuidof( ID3D11Texture2D), (void**) &BackBufferTexture);
            Device->CreateRenderTargetView(BackBufferTexture, NULL, &BackBuffer);
            
            BackBufferTexture->Release();

            DeviceContext->OMSetRenderTargets(1, &BackBuffer, NULL );

            D3D11_VIEWPORT ViewPort;
            ViewPort.Width = ScreenWidth;
            ViewPort.Height = ScreenHeight;
            ViewPort.MinDepth = 0.0f;
            ViewPort.MaxDepth = 1.0f;
            ViewPort.TopLeftX = 0;
            ViewPort.TopLeftY = 0;
            DeviceContext->RSSetViewports( 1, &ViewPort);
        }
    }
};

struct camera
{
    XMFLOAT3 Position = XMFLOAT3(0, 0, 0);
    XMFLOAT3 TargetPos = XMFLOAT3(0, 0, 1);
    XMFLOAT3 UpDirection = XMFLOAT3(0, 1, 0);
    real32 CameraSpeed = 1.6f;
    
    XMFLOAT4X4 ViewMx;
    XMFLOAT4X4 ProjMx;
    XMFLOAT4X4 ViewProjMx;
    
    ID3D11Buffer *SceneConstantBuffer;
    scene_constants SceneConstants;
    
    void Initialize(dx_resource &DXResources, screen_info Screen)
    {   
        XMStoreFloat4x4(&ViewMx, XMMatrixLookAtLH(XMLoadFloat3(&Position),
            XMLoadFloat3(&TargetPos), XMLoadFloat3(&UpDirection)));
        XMStoreFloat4x4(&ProjMx, 
            XMMatrixPerspectiveFovLH(45, Screen.Width/Screen.Height, 0.1f, 2000.0f));
        XMStoreFloat4x4(&ViewProjMx,
            XMMatrixMultiplyTranspose(XMLoadFloat4x4(&ViewMx),
                                      XMLoadFloat4x4(&ProjMx)));
        
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
        }
        if(Input.SpeedDown) 
        {
            CameraSpeed *= 0.9f;
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
        
        SceneConstants.ViewProjMx = ViewProjMx;
    }
};

#define TEREPGEN_DXRESOURCES_H
#endif