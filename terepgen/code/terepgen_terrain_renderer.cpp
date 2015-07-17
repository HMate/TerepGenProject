/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_terrain.h"


void terrainRenderer::Initialize(dx_resource &DXResources, uint32 FinalVertexCount)
{
    DXReleased = false;
    this->DXResource = DXResources;
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
    // NOTE: above 120MB vx buffer size dx11 crashes
    // NOTE: at vertex xize = 48 B, FinalVC = 2621440 is 120MB
    // TODO: Need to profile if drawing is faster with lower buffer size.
    // BufferDesc.ByteWidth = sizeof(vertex) * 2621440; // Max buffer size at 48B vertices
    BufferDesc.ByteWidth = sizeof(vertex) * FinalVertexCount;        
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;   
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    Assert(BufferDesc.ByteWidth <= MEGABYTE(120))

    DXResources.Device->CreateBuffer(&BufferDesc, NULL, &VertexBuffer);
}

void terrainRenderer::DrawWireframe(std::shared_ptr<vertex> Vertices)
{
    DXResource.LoadResource(ObjectConstantBuffer, &ObjectConstants, sizeof(ObjectConstants));
       
    uint32 stride = sizeof(vertex);
    uint32 offset = 0;
    DXResource.LoadResource(VertexBuffer, Vertices.get(), sizeof(vertex) * FinalVertexCount);    
             
    DXResource.DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    DXResource.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    DXResource.DeviceContext->Draw(FinalVertexCount, 0);
}

//TODO: Sometimes rednering acts weird, and triangles from specific angles dont appear
// maybe the order of vertices in buffer matter, and later triangles always appear ?
// maybe just the triangulization is bad?
void terrainRenderer::DrawTriangles(std::shared_ptr<vertex> Vertices)
{ 
    DXResource.LoadResource(ObjectConstantBuffer, &ObjectConstants, sizeof(ObjectConstants));
       
    uint32 stride = sizeof(vertex);
    uint32 offset = 0;
    DXResource.LoadResource(VertexBuffer, Vertices.get(), sizeof(vertex) * FinalVertexCount);    
    
    DXResource.DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    DXResource.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DXResource.DeviceContext->Draw(FinalVertexCount, 0);
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