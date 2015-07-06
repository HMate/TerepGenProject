/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_terrain.h"


void terrainRenderer::Initialize(dx_resource &DXResources, uint32 FinalVertexCount)
{
    DXReleased = false;
    this->FinalVertexCount = FinalVertexCount;
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
    
    D3D11_SUBRESOURCE_DATA ObjCBufferData;
    ObjCBufferData.pSysMem = &ObjectConstants;
    ObjCBufferData.SysMemPitch = 0;
    ObjCBufferData.SysMemSlicePitch = 0;
    
    DXResources.Device->CreateBuffer(&ObjectCBDesc, &ObjCBufferData, 
                               &ObjectConstantBuffer);
    DXResources.DeviceContext->VSSetConstantBuffers(1, 1, &ObjectConstantBuffer);  
    
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(BufferDesc));
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;  
    // TODO: Instead of FinalVertexCount, i should use the count from CreateRenderVertices here.
    // Its more precise, but then i cant change the number of vertices.
    BufferDesc.ByteWidth = sizeof(vertex) * FinalVertexCount;        
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;   
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    DXResources.Device->CreateBuffer(&BufferDesc, NULL, &VertexBuffer);
}

void terrainRenderer::DrawWireframe(dx_resource &DXResources, std::shared_ptr<vertex> Vertices)
{
    DXResources.LoadResource(ObjectConstantBuffer, &ObjectConstants, sizeof(ObjectConstants));
       
    uint32 stride = sizeof(vertex);
    uint32 offset = 0;
    DXResources.LoadResource(VertexBuffer, Vertices.get(), sizeof(vertex) * FinalVertexCount);    
    
    DXResources.DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    DXResources.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    DXResources.DeviceContext->Draw(FinalVertexCount, 0);
}

void terrainRenderer::DrawPoints(dx_resource &DXResources, std::shared_ptr<vertex> Vertices)
{ 
    
}

void terrainRenderer::Release()
{
    ObjectConstantBuffer->Release();
    VertexBuffer->Release();
    DXReleased = true;
}
   
terrainRenderer::~terrainRenderer()
{
    if(!DXReleased) Release();
}