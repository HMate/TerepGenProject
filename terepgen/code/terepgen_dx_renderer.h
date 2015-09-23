#if !defined(TEREPGEN_DX_RENDERER_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

struct vertex
{
    // NOTE: position is in left handed coordinate system
    // +X points right initially, -X points left
    // +Y is vertical axis and points up 
    // -Z points through screen to user initially, +Z points toward screen 
    real32 X, Y, Z;
    real32 NX, NY, NZ;
    color Color;
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

#define ERRMSGBUFFERSIZE 256

struct dx_resource
{
    IDXGISwapChain *SwapChain = nullptr;
    ID3D11Device *Device = nullptr;
    ID3D11DeviceContext *DeviceContext = nullptr;
    ID3D11RenderTargetView *BackBuffer = nullptr;
    ID3D11InputLayout *InputLayout = nullptr;
    ID3D11DepthStencilView *DepthStencilView = nullptr;
    ID3D11Texture2D *DepthStencilBuffer = nullptr;
    ID3D11DepthStencilState *DepthStencilState = nullptr;
    
    ID3D11VertexShader *TerrainVS = nullptr;
    ID3D11PixelShader *TerrainPS = nullptr;
    ID3D11PixelShader *LinePS = nullptr;
        
    int32 VideoCardMemory;
    char VideoCardDescription[128];
    
    HRESULT Initialize(HWND Window, uint32 ScreenWidth, uint32 ScreenHeight);
    void Release();
    
    void LoadResource(ID3D11Resource *Buffer, void *Resource, uint32 ResourceSize);
    
    HRESULT Resize(uint32 ScreenWidth, uint32 ScreenHeight);
    char* GetDebugMessage(DWORD dwErrorMsgId);
};

struct terrain_renderer
{
    dx_resource *DXResource;
    object_constants ObjectConstants;
    ID3D11Buffer *ObjectConstantBuffer;
    ID3D11Buffer *VertexBuffer;  
    ID3D11RasterizerState *RSWireFrame = nullptr; 
    ID3D11RasterizerState *RSDefault = nullptr;
    
    ID3D11ShaderResourceView* GrassTexture = nullptr;
    ID3D11ShaderResourceView* RockTexture = nullptr;
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
    void DrawLines(vertex *Vertices, uint32 VertCount);
    void DrawDebugTriangle();
};


using namespace DirectX;

struct camera
{
    XMFLOAT3 Position = XMFLOAT3(0, 0, 0);
    XMFLOAT3 TargetPos = XMFLOAT3(0, 0, 1);
    XMFLOAT3 UpDirection = XMFLOAT3(0, 1, 0);
    real32 CameraSpeed = 60.0f;
    
    real32 Fov = 3.14f * 0.35f;
    real32 NearClipZ = 10.0f;
    real32 FarClipZ = 100000.0f;
    
    XMFLOAT4X4 ViewMx;
    XMFLOAT4X4 ProjMx;
    XMFLOAT4X4 ViewProjMx;
    
    ID3D11Buffer *SceneConstantBuffer;
    scene_constants SceneConstants;
    
    v3 GetPos();
    v3 GetLookDirection();
    void Initialize(dx_resource *DXResources, uint32 ScreenWidth, uint32 ScreenHeight, real32 CamSpeed);
    void Release();
    
    void Update(input *Input, real64 TimeDelta);
    void Resize(uint32 ScreenWidth, uint32 ScreenHeight);
};

#define TEREPGEN_DX_RENDERER_H
#endif