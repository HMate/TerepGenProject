/*
    Terep generátor by Hidvégi Máté @2015

*/

#include <windows.h>

#include "terepgen_types.h"
#include "terepgen_terrain.h"
#include "terepgen_dxresources.h"

global_variable bool32 GlobalRunning = true;
global_variable input GlobalInput;
global_variable bool32 Resize;
global_variable bool32 DrawTerrain1;   
global_variable bool32 DrawTerrain2;   
global_variable uint32 GlobalSeed;  
global_variable real32 Persistence;   
global_variable LARGE_INTEGER GlobalPerfCountFrequency;

internal screen_info 
GetWindowDimension(HWND Window)
{
    screen_info Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Height = ClientRect.bottom - ClientRect.top;
    Result.Width = ClientRect.right - ClientRect.left;

    return Result;
}
        
LRESULT CALLBACK
WindowProc(HWND Window, 
           UINT Message,
           WPARAM WParam,
           LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_DESTROY:
        {
#if TEREPGEN_DEBUG
        OutputDebugStringA("[TEREPGEN_DEBUG] message arrived: WM_DESTROY \n");
#endif    
            GlobalRunning = false;
        } break;

        case WM_CLOSE:
        {
            //TODO: Handle with message to the user?
#if TEREPGEN_DEBUG
        OutputDebugStringA("[TEREPGEN_DEBUG] message arrived: WM_CLOSE \n");
#endif        
            GlobalRunning = false;
        } break;
        
        case WM_SIZE:
        {
#if TEREPGEN_DEBUG
        OutputDebugStringA("[TEREPGEN_DEBUG] message arrived: WM_SIZE \n");
#endif    
            Resize = true;
        } break;
        
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32 KeyCode = (uint32)WParam;
            bool32 KeyIsUp = (LParam >> 31) & 1;
            bool32 KeyWasDown = (LParam >> 30) & 1;
            
            if(KeyIsUp == KeyWasDown)
            {
                if(KeyCode == 'W')
                {
                    GlobalInput.MoveForward = !GlobalInput.MoveForward;
                }
                else if(KeyCode == 'A')
                {
                    GlobalInput.MoveLeft = !GlobalInput.MoveLeft;
                }
                else if(KeyCode == 'S')
                {
                    GlobalInput.MoveBack = !GlobalInput.MoveBack;
                }
                else if(KeyCode == 'D')
                {
                    GlobalInput.MoveRight = !GlobalInput.MoveRight;
                }
                else if(KeyCode == 'E' || KeyCode == VK_SPACE)
                {
                    GlobalInput.MoveUp = !GlobalInput.MoveUp;
                }
                else if(KeyCode == 'Q')
                {
                    GlobalInput.MoveDown = !GlobalInput.MoveDown;
                }
                else if(KeyCode == VK_ADD)
                {
                    GlobalInput.SpeedUp = !GlobalInput.SpeedUp;
                }
                else if(KeyCode == VK_SUBTRACT)
                {
                    GlobalInput.SpeedDown = !GlobalInput.SpeedDown;
                }
                else if(KeyCode == VK_NUMPAD1 && !KeyIsUp)
                {
                   DrawTerrain1 = !DrawTerrain1;
                }
                else if((KeyCode == VK_NUMPAD2 || KeyCode == 'T') && !KeyIsUp)
                {
                   DrawTerrain2 = !DrawTerrain2;
                }
                else if(KeyCode == 'R' && !KeyIsUp)
                {
                    GlobalInput.RenderMode++;
                    if(GlobalInput.RenderMode > 2) 
                    {
                        GlobalInput.RenderMode = 0;
                    }
                }
                else if(KeyCode == VK_ESCAPE && !KeyIsUp)
                {
                   GlobalRunning = false;
                }
                else if(KeyCode == VK_UP && !KeyIsUp)
                {
                   GlobalSeed++;
                }
                else if(KeyCode == VK_DOWN && !KeyIsUp)
                {
                   GlobalSeed--;
                }
                else if(KeyCode == VK_LEFT && !KeyIsUp)
                {
                   Persistence -= 0.05f;
                }
                else if(KeyCode == VK_RIGHT && !KeyIsUp)
                {
                   Persistence += 0.05f;
                }
                
            }
        } break;
        
        default:
        {
            Result = DefWindowProc (Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

#include <thread>
//#include <chrono>

internal bool32
IsInLoadingBlocks(int32 LoadingBlocks[], uint32 ArraySize, uint32 BlockIndex)
{
    bool32 Result = false;
    for(size_t Index = 0; Index < ArraySize; Index++)
    {
        if(LoadingBlocks[Index] == BlockIndex) 
        {
            Result = true;
        }
    }
    return Result;
}

internal uint32
GetLoadingBlockIndex(int32 LoadingBlocks[], uint32 ArraySize, uint32 BlockIndex)
{
    uint32 Result = ArraySize;
    for(size_t Index = 0; Index < ArraySize; Index++)
    {
        if(LoadingBlocks[Index] == BlockIndex) 
        {
            Result = Index;
            break;
        }
    }
    return Result;
}

internal int32
GetEmptyThread(int32 LoadingBlocks[], uint32 ArraySize)
{
    for(size_t Index = 0; Index < ArraySize; Index++)
    {
        if(LoadingBlocks[Index] == -1) 
        {
            return Index;
        }
    }
    Assert(false);
    return -1;
}

internal void
ArrayCopyV3(v3 To[], v3 From[], uint32 Count)
{
    for(uint32 Idx = 0; Idx < Count; ++Idx)
    {
        To[Idx] = From[Idx];
    }
}

internal void
ArrayCopyTerrain3D(terrain3D To[], terrain3D From[], uint32 Count)
{
    for(uint32 Idx = 0; Idx < Count; ++Idx)
    {
        To[Idx] = From[Idx];
    }
}

struct grid_thread
{
    std::thread t;
    terrain3D Terrain;
    bool32 Loading = false;
    
    void InitGrid(uint32 Seed, real32 Persistence, v3 BlockPos, uint32 CubeSize)
    {
        Loading = true;
        t = std::thread(&terrain3D::Initialize, &(Terrain),
                Seed, Persistence, BlockPos, CubeSize);
        t.detach();
    }
    
    bool32 IsLoading()
    {
        if(Loading)
        {
            Loading = Terrain.Loaded ? false : true;
        }
        return Loading;
    }
};

struct world_grid
{
    const static uint32 BlockCount = 27;
    const static uint32 ThreadCount = 7;
    
    uint32 BlockDimension;
    uint32 BlockVertexCount;
    uint32 CubeSize = 4;
    real32 BlockSize;
    terrain3D TerrainBlocks[BlockCount];
    v3 LastCameraPos;
    v3 BlockPos[BlockCount];
    
    grid_thread Thread[ThreadCount];
    uint32 InitBlockIndex;
    uint32 ActiveThreadCount;
    int32 LoadingBlocks[ThreadCount];
    
    void Initialize(v3 Position, uint32 Seed, real32 Persistence)
    {
        BlockDimension = 65;
        BlockSize = 64.0f;
        
        v3 CentralBlockPos = Position / BlockSize;
        CentralBlockPos = v3{FloorReal32(CentralBlockPos.X), 
                             FloorReal32(CentralBlockPos.Y),
                             FloorReal32(CentralBlockPos.Z) + 2.0f};
        LastCameraPos = CentralBlockPos;
        
        uint32 PosIndex = 0;
        int32 Start = -1;
        int32 End = 2;
        for(int32 XIndex = Start; XIndex < End; ++XIndex)
        {
            for(int32 YIndex = Start; YIndex < End; ++YIndex)
            {
                for(int32 ZIndex = Start; ZIndex < End; ++ZIndex)
                {
                    BlockPos[PosIndex++] = CentralBlockPos +
                        v3{(real32)XIndex, (real32)YIndex, (real32)ZIndex};
                }
            }
        }
        Assert(PosIndex == BlockCount);
        
        InitBlockIndex = 0;
        ActiveThreadCount = 0;
        
        for(InitBlockIndex = 0; 
            InitBlockIndex < ThreadCount; 
            ++InitBlockIndex)
        {
            Thread[InitBlockIndex].InitGrid(Seed, Persistence, 
                BlockPos[InitBlockIndex] * BlockSize, CubeSize);
            LoadingBlocks[ActiveThreadCount] = InitBlockIndex;
            ActiveThreadCount++;
        }
        
        BlockVertexCount = BlockDimension*BlockDimension*BlockDimension*6;
    }
    
    void Update(v3 Position, uint32 Seed, real32 Persistence, terrain_render_mode RenderMode)
    {
        // TODO: Bring Update logic here from terrain3D?
        
        v3 CentralBlockPos = Position / BlockSize;
        CentralBlockPos = v3{FloorReal32(CentralBlockPos.X), 
                             FloorReal32(CentralBlockPos.Y),
                             FloorReal32(CentralBlockPos.Z) + 2.0f};
        
        if(LastCameraPos != CentralBlockPos)
        {
            LastCameraPos = CentralBlockPos;
            v3 UpdatedBlockPos[BlockCount];
            terrain3D UpdatedTerrainBlocks[BlockCount];
            uint32 PosIndex = 0;
            int32 Start = -1;
            int32 End = 2;
            for(int32 XIndex = Start; XIndex < End; ++XIndex)
            {
                for(int32 YIndex = Start; YIndex < End; ++YIndex)
                {
                    for(int32 ZIndex = Start; ZIndex < End; ++ZIndex)
                    {
                        // NOTE: If the block is already loaded, and still needed, 
                        //      we have to update the indices, in TerrrainBlocks,
                        //      so they are at their new index.
                        //      + We have to change the indices in loading blocks too
                        UpdatedBlockPos[PosIndex++] = CentralBlockPos +
                            v3{(real32)XIndex, (real32)YIndex, (real32)ZIndex};
                    }
                }
            }
            Assert(PosIndex == BlockCount);
            
            uint32 UpdatedBlockCount = PosIndex;
            for(PosIndex = 0; PosIndex < UpdatedBlockCount; ++PosIndex)
            {
                bool32 HasPos = false;
                uint32 OldIndex = BlockCount + 1;
                for(size_t Index = 0; Index < BlockCount;  ++Index)
                {
                    if(BlockPos[Index] == UpdatedBlockPos[PosIndex])
                    {
                        HasPos = true;
                        OldIndex = Index;
                        break;
                    }
                }
                
                if(HasPos)
                {
                    if(TerrainBlocks[OldIndex].Loaded)
                    {
                        // NOTE: If we already had this block, and its loaded, we just copy it.
                        UpdatedTerrainBlocks[PosIndex] = TerrainBlocks[OldIndex];
                    }
                    else
                    {
                        bool32 IsLoading = IsInLoadingBlocks(LoadingBlocks, ThreadCount, OldIndex);
                        if(IsLoading)
                        {
                            // NOTE: If its loading now we just memorize its new index.
                            uint32 LoadIdx = GetLoadingBlockIndex(LoadingBlocks, ThreadCount, OldIndex);
                            LoadingBlocks[LoadIdx] = PosIndex;
                        }
                        // NOTE: do nothing, if it isnt loading, and we wont even need it.
                    }
                }
            }
            
            ArrayCopyV3(BlockPos, UpdatedBlockPos, BlockCount);
            ArrayCopyTerrain3D(TerrainBlocks, UpdatedTerrainBlocks, BlockCount);
        }
        
        for(size_t BlockIndex = 0; 
            BlockIndex < BlockCount; 
            ++BlockIndex)
        {
            bool32 IsLoading = IsInLoadingBlocks(LoadingBlocks, ThreadCount, BlockIndex);
            uint32 LoadIdx = GetLoadingBlockIndex(LoadingBlocks, ThreadCount, BlockIndex);
            if(IsLoading && Thread[LoadIdx].Terrain.Loaded)
            {
                TerrainBlocks[BlockIndex] = Thread[LoadIdx].Terrain;
                LoadingBlocks[LoadIdx] = -1;
                --ActiveThreadCount;
            }
            
            if(TerrainBlocks[BlockIndex].Loaded)
            {                
                TerrainBlocks[BlockIndex].Update(Seed, Persistence, RenderMode, CubeSize);
            }
            else if(ActiveThreadCount < ThreadCount && !IsLoading)
            {
                // NOTE: Start loading block, if there is a free thread
                int32 ThreadIdx = GetEmptyThread(LoadingBlocks, ThreadCount);
                Thread[ThreadIdx].InitGrid(Seed, Persistence,
                    BlockPos[BlockIndex] * BlockSize, CubeSize);
                LoadingBlocks[ThreadIdx] = BlockIndex;
                ActiveThreadCount++;
            }
        }
    }
    
    void Draw(terrainRenderer *TRenderer)
    {
        for(size_t i = 0; i < BlockCount; ++i)
        {
            if(TerrainBlocks[i].Loaded)
            {
                TerrainBlocks[i].Draw(TRenderer);
            }
        }
    }
    
    ~world_grid()
    {
        while(ActiveThreadCount > 0)
        {
            for(size_t ThreadIdx = 0; 
                ThreadIdx < ThreadCount; 
                ++ThreadIdx)
            {
                bool32 IsLoading = LoadingBlocks[ThreadIdx] != -1;
                if(IsLoading && Thread[ThreadIdx].Terrain.Loaded)
                {
                    LoadingBlocks[ThreadIdx] = -1;
                    --ActiveThreadCount;
                }
            }
            // TODO: Maybe sleep would be better?
            std::this_thread::yield();
        }
    }
};

// win32_clock
// {
    // TODO
// };

inline LARGE_INTEGER
Win32GetWallClock(void)
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}    

inline real64
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    real64 Result = ((real64)(End.QuadPart - Start.QuadPart) /
                    (real64)GlobalPerfCountFrequency.QuadPart);
    return Result;
}

int CALLBACK 
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    
    int32 A = (int32)-10.f;
    WNDCLASS WindowClass = {};
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = "TerepGenWindowClass";
    
	int32 MaxScreenWidth  = GetSystemMetrics(SM_CXSCREEN);
	int32 MaxScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    screen_info ScreenInfo;
    ScreenInfo.Width = 1280;
	ScreenInfo.Height = 800;
	// ScreenInfo.Width = 800;
	// ScreenInfo.Height = 600;
    
    if(RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(
                            0,
                            WindowClass.lpszClassName,
                            "TerepGen",
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                            200,
                            50,
                            ScreenInfo.Width,
                            ScreenInfo.Height,
                            0, 0,
                            Instance,
                            0);
        
        if(Window)
        {
            DrawTerrain1 = true;
            GlobalSeed = 1000;
            dx_resource DXResources;
            HRESULT HResult;
            HResult = DXResources.Initialize(Window, ScreenInfo.Width, ScreenInfo.Height);
            if(FAILED(HResult))
            {
                char* ErrMsg = DXResources.GetDebugMessage(HResult);
#if TEREPGEN_DEBUG
                OutputDebugStringA(("[TEREPGEN_DEBUG] Initialize error: " +
                    std::string(ErrMsg)).c_str());
#endif
                //MessageBox(NULL, DXGetErrorDescription(HResult), NULL, MB_OK);
                DXResources.Release();
                return 1;
            }
            
            camera Camera;
            Camera.Initialize(&DXResources, ScreenInfo);
            
            Persistence = 0.4f;
            
			// terrain Terrain;
            // Terrain.Color = color{1.0f, 1.0f, 1.0f, 1.0f};
            // Terrain.Initialize(GlobalSeed, Persistence);
                        
            world_grid WorldTerrain;
            WorldTerrain.Initialize(Camera.GetPos(), GlobalSeed, Persistence);
            
            terrainRenderer TRenderer;
            HResult = TRenderer.Initialize(&DXResources, WorldTerrain.BlockVertexCount);
            if(FAILED(HResult))
            {
                //MessageBox(NULL, DXGetErrorDescription(HResult), NULL, MB_OK);
                char* ErrMsg = DXResources.GetDebugMessage(HResult);
#if TEREPGEN_DEBUG
                OutputDebugStringA(("[TEREPGEN_DEBUG] terrainRenderer init error: " +
                    std::string(ErrMsg)).c_str());
#endif
                TRenderer.Release();
                Camera.Release();
                DXResources.Release();
                return 1;
            }
            
            LARGE_INTEGER FrameStartTime = Win32GetWallClock();
            LARGE_INTEGER WorldTime = FrameStartTime;
            QueryPerformanceFrequency(&GlobalPerfCountFrequency);
            
            while(GlobalRunning)
            {
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                
                if(Resize)
                {
                    ScreenInfo = GetWindowDimension(Window);
                    HResult = DXResources.Resize(ScreenInfo.Width, ScreenInfo.Height);
                    if(FAILED(HResult)) 
                    {
                        char* ErrMsg = DXResources.GetDebugMessage(HResult);
#if TEREPGEN_DEBUG
                        OutputDebugStringA(("[TEREPGEN_DEBUG] Resize error: " +
                            std::string(ErrMsg)).c_str());
#endif
                        break;
                    }
                    Camera.Resize(ScreenInfo);
                    Resize = false;
                }
        
                // NOTE: Update
                GlobalInput.OldMouseX = GlobalInput.MouseX;
                GlobalInput.OldMouseY = GlobalInput.MouseY;
                
                POINT MouseP;
                GetCursorPos(&MouseP);
                ScreenToClient(Window, &MouseP);
                if(GetActiveWindow() == Window)
                {
                    GlobalInput.MouseX = MouseP.x;
                    GlobalInput.MouseY = -MouseP.y;
                }
                
                LARGE_INTEGER NewTime = Win32GetWallClock();
                real64 TimePassed = Win32GetSecondsElapsed(WorldTime, NewTime);
                WorldTime.QuadPart = NewTime.QuadPart;
                
                Camera.Update(&GlobalInput, TimePassed);
                WorldTerrain.Update(Camera.GetPos(), GlobalSeed, Persistence, (terrain_render_mode)GlobalInput.RenderMode);
                
                // NOTE: Rendering
                DXResources.LoadResource(Camera.SceneConstantBuffer,
                              &Camera.SceneConstants, sizeof(Camera.SceneConstants));
                
                color BackgroundColor = {0.0f, 0.2f, 0.4f, 1.0f};
                //color BackgroundColor = {0.3f, 0.3f, 0.3f, 1.0f}; //grey
                DXResources.DeviceContext->ClearDepthStencilView(DXResources.DepthStencilView, 
                    D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
                DXResources.DeviceContext->ClearRenderTargetView(DXResources.BackBuffer, BackgroundColor.C);
                
                TRenderer.DrawAxis(100.0f);
                // TRenderer.DrawDebugTriangle();
                WorldTerrain.Draw(&TRenderer);
                // if(DrawTerrain2)Terrain3D.UpdateAndDrawPoints(DXResources, GlobalSeed, Persistence);
                
                DXResources.SwapChain->Present(0, 0);
                
                
                LARGE_INTEGER FrameEndTime = Win32GetWallClock();
                real64 SecondsElapsed = Win32GetSecondsElapsed(FrameStartTime, FrameEndTime);
                FrameStartTime.QuadPart = FrameEndTime.QuadPart;
#if TEREPGEN_DEBUG
                OutputDebugStringA(("[TEREPGEN_DEBUG] FPS:" + std::to_string(1.0/SecondsElapsed) + "\n").c_str());
#endif
            }
            
            Camera.Release();
            DXResources.Release();
        }
    }
    
    return 0;
}