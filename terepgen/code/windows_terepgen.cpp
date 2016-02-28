/*
    Terep generátor by Hidvégi Máté @2015

*/

#include <windows.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "external\stb\stb_image.h"

#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "terepgen.h"

global_variable bool32 GlobalRunning = true;
global_variable input GlobalInput;
global_variable bool32 Resize;
global_variable bool32 DrawTerrain1;   
global_variable bool32 DrawTerrain2;   
global_variable uint32 GlobalSeed;  
global_variable real32 Persistence;
global_variable LARGE_INTEGER GlobalPerfCountFrequency;  

struct win32_printer
{
    static void DebugPrint(char *Text)
    {
#if TEREPGEN_DEBUG
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s\n", Text);
        OutputDebugStringA(DebugBuffer);
#endif
    }
    
    static void DebugPrint(char *Text, uint32 Arg1)
    {
#if TEREPGEN_DEBUG
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s\n", Text);
        char DebugBuffer2[256];
        sprintf_s(DebugBuffer2, DebugBuffer, Arg1);
        OutputDebugStringA(DebugBuffer2);
#endif
    }
    
    static void DebugPrint(char *Text, real32 Arg1, real32 Arg2)
    {
#if TEREPGEN_DEBUG
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s\n", Text);
        char DebugBuffer2[256];
        sprintf_s(DebugBuffer2, DebugBuffer, Arg1, Arg2);
        OutputDebugStringA(DebugBuffer2);
#endif
    }
}; 

// NOTE: Requires GlobalPerfCountFrequency to be initialized
struct win32_clock
{
    LARGE_INTEGER Start;
    
    win32_clock()
    {
        Reset();
    }
    
    void Reset()
    {
        QueryPerformanceCounter(&Start);
    }
    
    real64 GetSecondsElapsed()
    {
        LARGE_INTEGER End;
        QueryPerformanceCounter(&End);
        real64 Result = ((real64)(End.QuadPart - Start.QuadPart) /
                        (real64)GlobalPerfCountFrequency.QuadPart);
        return Result;
    }
    
    void PrintMiliSeconds(char *PrintText)
    {
        real64 SecondsElapsed = GetSecondsElapsed();
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s %f ms\n",
            PrintText, SecondsElapsed * 1000.0);
        OutputDebugStringA(DebugBuffer);
    }
    
    void PrintSeconds(char *PrintText)
    {
        real64 SecondsElapsed = GetSecondsElapsed();
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s %f s\n",
            PrintText, SecondsElapsed);
        OutputDebugStringA(DebugBuffer);
    }
};

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

#include "terepgen.cpp"
// TODO: Should the render code be a separately compiled cpp?

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
                    if(GlobalInput.RenderMode > 1) 
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

int CALLBACK 
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    WNDCLASS WindowClass = {};
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = "TerepGenWindowClass";
    
    int32 MaxScreenWidth  = GetSystemMetrics(SM_CXSCREEN);
    int32 MaxScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    screen_info ScreenInfo;
    ScreenInfo.Width = MaxScreenWidth*3/4;//1280;
    ScreenInfo.Height = MaxScreenHeight*3/4;//800;
    
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
            HRESULT HResult = DXResources.Initialize(Window, ScreenInfo.Width, ScreenInfo.Height);
            if(FAILED(HResult))
            {
                char* ErrMsg = DXResources.GetDebugMessage(HResult);
#if TEREPGEN_DEBUG
                char DebugBuffer[256];
                sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Initialize error: %s\n", ErrMsg);
                OutputDebugStringA(DebugBuffer);
                MessageBox(NULL, DebugBuffer, NULL, MB_OK);
#endif
                DXResources.Release();
                return 1;
            }
            
            camera Camera;
            Camera.Initialize(&DXResources, ScreenInfo.Width, ScreenInfo.Height, 20.0f);
            
            Persistence = 0.4f;
            
            game_state *GameState = new game_state;
            GameState->Initialized = false;
            GameState->DXResources = &DXResources;
            
            QueryPerformanceFrequency(&GlobalPerfCountFrequency);
            win32_clock FrameClock;
            win32_clock WorldClock;
            LARGE_INTEGER FrameStartTime = Win32GetWallClock();
            LARGE_INTEGER WorldTime = FrameStartTime;
            
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
                        char DebugBuffer[256];
                        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Resize error: %s\n", ErrMsg);
                        OutputDebugStringA(DebugBuffer);
#endif
                        break;
                    }
                    Camera.Resize(ScreenInfo.Width, ScreenInfo.Height);
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
                
                real64 TimePassed = WorldClock.GetSecondsElapsed();
                WorldClock.Reset();
                
                Camera.Update(&GlobalInput, TimePassed);
                
                GameState->CameraPos = Camera.GetPos();
                GameState->CameraDir = Camera.GetLookDirection();
                
                v2 MouseInPixel = v2{(real32)MouseP.x, (real32)MouseP.y};
                real32 WorldScreenSizeY = 2.0f*Tan(Camera.Fov/2.0f);
                real32 WorldScreenSizeX = WorldScreenSizeY*(real32)ScreenInfo.Width/ScreenInfo.Height;
                v3 UpDir = Camera.GetUpDirection();
                v3 RightDir = Normalize(Cross(UpDir, GameState->CameraDir));
                v2 NormalizedMouse = MouseInPixel - v2{(real32)ScreenInfo.Width/2, (real32)ScreenInfo.Height/2};
                NormalizedMouse.X = NormalizedMouse.X / ScreenInfo.Width;
                NormalizedMouse.Y = -NormalizedMouse.Y / ScreenInfo.Height;
                
                v3 WorldMouse = GameState->CameraPos + (UpDir*NormalizedMouse.Y*WorldScreenSizeY) 
                    + (RightDir*NormalizedMouse.X*WorldScreenSizeX);
                    
                v3 CameraOrigo = GameState->CameraPos + Normalize(Cross(UpDir, RightDir));
                
                GameState->Seed = GlobalSeed;
                GameState->RenderMode = GlobalInput.RenderMode;
                
                UpdateGameState(GameState, WorldMouse, CameraOrigo);
                RenderGame(GameState, &Camera);
                
                FrameClock.PrintMiliSeconds("Frame time:");
                FrameClock.Reset();
            }
            
            delete GameState;
            Camera.Release();
            DXResources.Release();
        }
    }
    
    return 0;
}