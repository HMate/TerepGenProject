/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_terrain.h"

terrainRenderer::terrainRenderer()
{
    DXReleased = false;
    VertexBuffer = nullptr;
    ObjectConstantBuffer = nullptr;
}

HRESULT terrainRenderer::Initialize(dx_resource *DXResources, uint32 MaxVertexCount)
{
    DXReleased = false;
    this->DXResource = DXResources;
    this->MaxVertexCount = MaxVertexCount;
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
    return HResult; 
}

void terrainRenderer::SetTransformations(v3 Translation)
{
    ObjectConstants.WorldMatrix = XMFLOAT4X4(1, 0, 0, Translation.X,
                                             0, 1, 0, Translation.Y,
                                             0, 0, 1, Translation.Z,
                                             0, 0, 0, 1);
}

// TODO: Instead of this, use rsterizer state
void terrainRenderer::DrawWireframe(vertex *Vertices, uint32 VertCount)
{         
    DXResource->DeviceContext->RSSetState(RSWireFrame);
    
    DXResource->LoadResource(ObjectConstantBuffer, &ObjectConstants, sizeof(ObjectConstants));
    DXResource->DeviceContext->VSSetConstantBuffers(1, 1, &ObjectConstantBuffer); 
       
    uint32 stride = sizeof(vertex);
    uint32 offset = 0;
    DXResource->LoadVertexBuffer(VertexBuffer, Vertices, sizeof(vertex) * VertCount);    
        
    DXResource->DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    DXResource->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DXResource->DeviceContext->Draw(VertCount, 0);
}

//TODO: triangulization have holes
void terrainRenderer::DrawTriangles(vertex *Vertices, uint32 VertCount)
{           
    DXResource->DeviceContext->RSSetState(RSDefault);
    
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
    real32 Scale = 1.0f;
    vertex Result = {LocalPos.X, LocalPos.Y, LocalPos.Z, 1.0f, 
                     0.0f, 1.0f, 0.0f, 1.0f,
                     Color};
    return Result;
}

void terrainRenderer::DrawDebugTriangle()
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

void terrainRenderer::DrawAxis(real32 Size)
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

void terrainRenderer::Release()
{
    if(RSDefault) RSDefault->Release();
    if(RSWireFrame) RSWireFrame->Release();
    if(ObjectConstantBuffer) ObjectConstantBuffer->Release();
    if(VertexBuffer) VertexBuffer->Release();
    DXReleased = true;
}
   
terrainRenderer::~terrainRenderer()
{
    if(!DXReleased) Release();
}