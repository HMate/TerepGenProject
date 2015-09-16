#if !defined(TEREPGEN_RENDERER_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

struct terrain_renderer
{
    dx_resource *DXResource;
    object_constants ObjectConstants;
    ID3D11Buffer *ObjectConstantBuffer;
    ID3D11Buffer *VertexBuffer;  
    ID3D11RasterizerState *RSWireFrame = nullptr; 
    ID3D11RasterizerState *RSDefault = nullptr;
    ID3D11ShaderResourceView* Texture = nullptr;
    ID3D11SamplerState* TexSamplerState = nullptr;
    uint32 MaxVertexCount;
    bool32 DXReleased;
    
    terrain_renderer();
    terrain_renderer(const terrain_renderer&) = delete;
    
    HRESULT Initialize(dx_resource *DXResources);
    ~terrain_renderer();
    void Release();
    
    void SetTransformations(v3 Translation);
    void SetDrawModeDefault(void);
    void SetDrawModeWireframe(void);
    void DrawTriangles(vertex *Vertices, uint32 VertexCount);
    void DrawDebugTriangle();
    void DrawAxis(real32 Size = 1.0f);
};

#define TEREPGEN_RENDERER_H
#endif