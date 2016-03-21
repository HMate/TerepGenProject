#if !defined(TEREPGEN_DX_RENDERER_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

struct scene_constants
{
    DirectX::XMFLOAT4X4 ViewProjMx;
    DirectX::XMFLOAT4X4 ViewMx;
};

struct object_constants
{
    DirectX::XMFLOAT4X4 WorldMatrix;
    DirectX::XMFLOAT4 CameraDir;
};

#define ERRMSGBUFFERSIZE 256

struct dx_resource
{
    object_constants ObjectConstants;
    
    real32 ViewPortMinDepth = 0.0f;
    real32 ViewPortMaxDepth = 1.0f;
    
    real32 DefaultDepthValue = 1.0f;
    
    IDXGISwapChain *SwapChain = nullptr;
    ID3D11Device *Device = nullptr;
    ID3D11DeviceContext *DeviceContext = nullptr;
    
    ID3D11RenderTargetView *BackBuffer = nullptr;
    ID3D11DepthStencilView *DepthStencilView = nullptr;
    ID3D11Texture2D *DepthStencilBuffer = nullptr;
    ID3D11DepthStencilState *DepthStencilState = nullptr;
    
    ID3D11Buffer *ObjectConstantBuffer = nullptr;
    ID3D11Buffer *VertexBuffer = nullptr;  
    ID3D11RasterizerState *RSWireFrame = nullptr; 
    ID3D11RasterizerState *RSDefault = nullptr;
    
    ID3D11InputLayout *TerrainInputLayout = nullptr;
    ID3D11InputLayout *BackgroundInputLayout = nullptr;
    
    ID3D11VertexShader *TerrainVS = nullptr;
    ID3D11PixelShader *TerrainPS = nullptr;
    ID3D11PixelShader *LinePS = nullptr;
    ID3D11VertexShader *BackgroundVS = nullptr;
    ID3D11PixelShader *BackgroundPS = nullptr;
    
    ID3D11ShaderResourceView* GrassTexture = nullptr;
    ID3D11ShaderResourceView* RockTexture = nullptr;
    ID3D11ShaderResourceView* SkyTexture = nullptr;
    ID3D11SamplerState* TexSamplerState = nullptr;
    ID3D11SamplerState* CubeTexSamplerState = nullptr;
        
    int32 VideoCardMemory;
    char VideoCardDescription[128];
    uint32 MaxVertexCount;
    
    HRESULT Initialize(HWND Window, uint32 ScreenWidth, uint32 ScreenHeight);
    void Release();
    
    void ClearViews();
    void LoadResource(ID3D11Resource *Buffer, void *Resource, uint32 ResourceSize);
    void SetTransformations(v3 Translation);
    void SetDrawModeDefault(void);
    void SetDrawModeWireframe(void);
    void DrawBackground(v3 *Vertices, uint32 VertCount);
    void DrawTriangles(vertex *Vertices, uint32 VertexCount);
    void DrawLines(vertex *Vertices, uint32 VertCount);
    void DrawDebugTriangle();
    
    HRESULT Resize(uint32 ScreenWidth, uint32 ScreenHeight);
    char* GetDebugMessage(DWORD dwErrorMsgId);
};

struct camera
{
    const DirectX::XMFLOAT3 AbsUpDir = DirectX::XMFLOAT3(0, 1, 0);
    const DirectX::XMFLOAT3 AbsHorzDir = DirectX::XMFLOAT3(0, 0, 1);
    
    DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT3 TargetPos = DirectX::XMFLOAT3(0, 0, 1);
    DirectX::XMFLOAT3 UpDirection = DirectX::XMFLOAT3(0, 1, 0);
    real32 CameraSpeed = 60.0f;
    
    real32 Fov = 3.14f * 0.35f;
    // NOTE: Regardless of nearZ, the screen always 
    // have a distance of 1.0 from the eye with PerspectiveFovLH
    real32 NearZ = 3.0f;
    real32 FarZ = 10000.0f;
    
    real32 YawRadian = 0.0f; // Turnning left-right
    real32 PitchRadian = 0.0f; // Turning up-down
    
    DirectX::XMFLOAT4X4 ViewMx;
    DirectX::XMFLOAT4X4 ProjMx;
    DirectX::XMFLOAT4X4 ViewProjMx;
    
    ID3D11Buffer *SceneConstantBuffer;
    scene_constants SceneConstants;
    
    // NOTE: Camera pos is the screen rectangles position in renderspace
    v3 GetPos();
    v3 GetLookDirection();
    v3 GetUpDirection();
    void Initialize(dx_resource *DXResources, uint32 ScreenWidth, uint32 ScreenHeight, real32 CamSpeed);
    void Release();
    
    void Update(game_input *Input, real64 TimeDelta);
    void Resize(uint32 ScreenWidth, uint32 ScreenHeight);
};

#define TEREPGEN_DX_RENDERER_H
#endif