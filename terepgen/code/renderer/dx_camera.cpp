/*
    Terep generátor by Hidvégi Máté @2015

*/
#include "renderer.h"

v3 camera::GetPos()
{
    v3 Result{Position.x, Position.y, Position.z};
    return Result;
}

v3 camera::GetLookDirection()
{
    using namespace DirectX;
    v3 Result = {};
    XMVECTOR TargetDirection =  XMLoadFloat3(&TargetPos) - XMLoadFloat3(&Position);
    Result.X = XMVectorGetX(TargetDirection);
    Result.Y = XMVectorGetY(TargetDirection);
    Result.Z = XMVectorGetZ(TargetDirection);
    Result = Normalize(Result);
    return Result;
}

v3 camera::GetUpDirection()
{
    using namespace DirectX;
    v3 Result = {};
    XMVECTOR TargetDirection =  XMLoadFloat3(&UpDirection);
    Result.X = XMVectorGetX(TargetDirection);
    Result.Y = XMVectorGetY(TargetDirection);
    Result.Z = XMVectorGetZ(TargetDirection);
    Result = Normalize(Result);
    return Result;
}

void camera::SetViewMatrix()
{
    using namespace DirectX;
    XMStoreFloat4x4(&ViewMx, XMMatrixLookAtLH(XMLoadFloat3(&Position),
                    XMLoadFloat3(&TargetPos), XMLoadFloat3(&UpDirection)));
}
void camera::SetProjMatrix(uint32 ScreenWidth, uint32 ScreenHeight)
{
    using namespace DirectX;
    XMStoreFloat4x4(&ProjMx, 
        XMMatrixPerspectiveFovLH(Fov, (real32)ScreenWidth/ScreenHeight, NearZ, FarZ));
}
void camera::SetViewProjMatrix()
{
    using namespace DirectX;
    XMStoreFloat4x4(&ViewProjMx,
        XMMatrixMultiply(XMLoadFloat4x4(&ViewMx), XMLoadFloat4x4(&ProjMx)));
}

void camera::SetSceneConstants()
{
    using namespace DirectX;
    SceneConstants.ViewMx = ViewMx;
    SceneConstants.ViewProjMx = ViewProjMx;
}

void camera::Initialize(dx_resource *DXResources, uint32 ScreenWidth, uint32 ScreenHeight, real32 CamSpeed)
{   
    using namespace DirectX;
    
    AbsUpDir = DirectX::XMFLOAT3(0, 1, 0);
    AbsHorzDir = DirectX::XMFLOAT3(0, 0, 1);
    Position = DirectX::XMFLOAT3(0, 0, 0);
    TargetPos = DirectX::XMFLOAT3(0, 0, 1);
    UpDirection = DirectX::XMFLOAT3(0, 1, 0);
    
    CameraSpeed = CamSpeed;
    Fov = 3.14f * 0.35f;
    NearZ = 3.0f;
    FarZ = 3000.0f;
    
    YawRadian = 0.0f;
    PitchRadian = 0.0f; 
    
    logger::DebugPrint("Camera Width: %d", ScreenWidth);
    logger::DebugPrint("Camera Height: %d", ScreenHeight);
    logger::DebugPrint("Camera ApectRatio: %f", (real32)ScreenWidth/ScreenHeight);
    
    SetViewMatrix();
    SetProjMatrix(ScreenWidth, ScreenHeight);
    SetViewProjMatrix();
    
    SetSceneConstants();
    
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
    if(SceneConstantBuffer)
    {
        SceneConstantBuffer->Release();
        SceneConstantBuffer = nullptr;
    }
}

void camera::Update(game_input *Input, real64 TimeDelta)
{
    using namespace DirectX;
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
    
    // NOTE: rotate camera
    real32 dMouseX = (real32)(Input->MouseX - Input->OldMouseX);
    real32 dMouseY = (real32)(Input->MouseY - Input->OldMouseY);
    if(Input->MouseLeftButton)
    {
        YawRadian += dMouseX/100.0f;
        PitchRadian += dMouseY/100.0f;
        
        //logger::DebugPrint("Yaw: %f Pitch: %f", YawRadian, PitchRadian);

        XMVECTOR NewTargetDir = XMLoadFloat3(&AbsHorzDir);
        XMVECTOR NewUpDir = XMLoadFloat3(&AbsUpDir);
        XMVECTOR LeftDir = XMVector3Normalize(XMVector3Cross( XMLoadFloat3(&AbsUpDir), XMLoadFloat3(&AbsHorzDir)));
        
        XMMATRIX VerticalRotation = XMMatrixRotationNormal(LeftDir, PitchRadian);
        XMMATRIX HorzRotation = XMMatrixRotationNormal(XMLoadFloat3(&AbsUpDir), YawRadian);
                
        NewTargetDir = XMVector3Transform(XMLoadFloat3(&AbsHorzDir), VerticalRotation);
        NewUpDir = XMVector3Transform(XMLoadFloat3(&AbsUpDir), VerticalRotation);
        
        XMStoreFloat3(&TargetPos,
            XMVector3Transform(NewTargetDir, HorzRotation) + XMLoadFloat3(&Position));
        XMStoreFloat3(&UpDirection, XMVector3Transform(NewUpDir, HorzRotation));
    }
    
    SetViewMatrix();
    SetViewProjMatrix();
    SetSceneConstants();
}
    
void camera::Resize(uint32 ScreenWidth, uint32 ScreenHeight)
{
    using namespace DirectX;
    if(ScreenWidth && ScreenHeight)
    {
        logger::DebugPrint("Camera Width: %d", ScreenWidth);
        logger::DebugPrint("Camera Height: %d", ScreenHeight);
        logger::DebugPrint("Camera ApectRatio: %f", (real32)ScreenWidth/ScreenHeight);
        
        SetProjMatrix(ScreenWidth, ScreenHeight);
        SetViewProjMatrix();
    }
}