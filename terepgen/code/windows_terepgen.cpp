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
global_variable bool32 Resize;
global_variable LARGE_INTEGER GlobalPerfCountFrequency;  

struct win32_printer
{
    static void Print(char *Text)
    {
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN] %s\n", Text);
        OutputDebugStringA(DebugBuffer);
    }
    
    static void Print(char *Text, real64 Arg1)
    {
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN] %s\n", Text);
        char DebugBuffer2[256];
        sprintf_s(DebugBuffer2, DebugBuffer, Arg1);
        OutputDebugStringA(DebugBuffer2);
    }
    
    static void Print(char *Text, uint32 Arg1)
    {
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN] %s\n", Text);
        char DebugBuffer2[256];
        sprintf_s(DebugBuffer2, DebugBuffer, Arg1);
        OutputDebugStringA(DebugBuffer2);
    }
    
    static void PerfPrint(char *Text, real64 Arg1)
    {
#if TEREPGEN_PERF
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_PERF] %s\n", Text);
        char DebugBuffer2[256];
        sprintf_s(DebugBuffer2, DebugBuffer, Arg1);
        OutputDebugStringA(DebugBuffer2);
#endif
    }
    
    static void PerfPrint(char *Text, real64 Arg1, uint32 Arg2)
    {
#if TEREPGEN_PERF
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_PERF] %s\n", Text);
        char DebugBuffer2[256];
        sprintf_s(DebugBuffer2, DebugBuffer, Arg1, Arg2);
        OutputDebugStringA(DebugBuffer2);
#endif
    }
    
    static void DebugPrint(char *Text)
    {
#if TEREPGEN_DEBUG
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s\n", Text);
        OutputDebugStringA(DebugBuffer);
#endif
    }
    
    static void DebugPrint(char *Text, char *Arg1)
    {
#if TEREPGEN_DEBUG
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s\n", Text);
        char DebugBuffer2[256];
        sprintf_s(DebugBuffer2, DebugBuffer, Arg1);
        OutputDebugStringA(DebugBuffer2);
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
    
    static void DebugPrint(char *Text, real32 Arg1)
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
    
    static void DebugPrint(char *Text, real64 Arg1)
    {
#if TEREPGEN_DEBUG
        char DebugBuffer[256];
        sprintf_s(DebugBuffer, "[TEREPGEN_PERF] %s\n", Text);
        char DebugBuffer2[256];
        sprintf_s(DebugBuffer2, DebugBuffer, Arg1);
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

internal void 
CalculateAvarageTime(win32_clock Clock, avarage_time *Avarage)
{
    real64 CurrentTime = Clock.GetSecondsElapsed();
    real64 LastMeasure = Avarage->MeasureCount;
    Avarage->MeasureCount += 1.0f;
    Avarage->AvgTime = 
        (LastMeasure/Avarage->MeasureCount)*Avarage->AvgTime + (CurrentTime / Avarage->MeasureCount);
}

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

internal FileHandle
PlatformOpenFileForRead(char *FileName)
{
    FileHandle Handle = CreateFile(FileName, GENERIC_READ,
        FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    Assert(Handle != INVALID_HANDLE_VALUE);
    return Handle;
}

internal FileHandle
PlatformOpenOrCreateFileForWrite(char *FileName)
{
    FileHandle Handle = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(Handle == INVALID_HANDLE_VALUE)
    {
        uint32 Error = GetLastError();
        if(Error == ERROR_FILE_NOT_FOUND)
        {
            // NOTE: The file didn't exist before, so now we create it
            Handle = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            Assert(Handle != INVALID_HANDLE_VALUE);
        }
    }
    return Handle;
}

internal bool32
FileIsEmpty(FileHandle Handle)
{
    bool32 IsEmpty = (0 == GetFileSize(Handle, NULL));
    return IsEmpty;
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
            win32_printer::DebugPrint("message arrived: WM_DESTROY");
            GlobalRunning = false;
        } break;

        case WM_CLOSE:
        {
            win32_printer::DebugPrint("message arrived: WM_CLOSE");
            GlobalRunning = false;
        } break;
        
        case WM_SIZE:
        {
            win32_printer::DebugPrint("message arrived: WM_SIZE");
            Resize = true;
        } break;
        
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert(!"Keyboard message arrived outside of loop!");
        }break;
        
        default:
        {
            Result = DefWindowProc (Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

internal void
Win32HandleMessages(game_input *Input)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:
            case WM_SYSKEYDOWN:
            {
                uint32 KeyCode = (uint32)Message.wParam;
                bool32 KeyWasDown = ((Message.lParam >> 30) & 1) == 1;
                bool32 KeyIsDown = ((Message.lParam >> 31) & 1) == 0;
                
                bool32 InLongState = (KeyIsDown == KeyWasDown);
                bool32 HadTransition = (KeyIsDown != KeyWasDown);
                
                if(HadTransition)
                {
                    if(KeyCode == 'W')
                    {
                        Input->MoveForward = KeyIsDown;
                    }
                    else if(KeyCode == 'A')
                    {
                        Input->MoveLeft = KeyIsDown;
                    }
                    else if(KeyCode == 'S')
                    {
                        Input->MoveBack = KeyIsDown;
                    }
                    else if(KeyCode == 'D')
                    {
                        Input->MoveRight = KeyIsDown;
                    }
                    else if(KeyCode == 'E' || KeyCode == VK_SPACE)
                    {
                        Input->MoveUp = KeyIsDown;
                    }
                    else if(KeyCode == 'Q')
                    {
                        Input->MoveDown = KeyIsDown;
                    }
                    else if(KeyCode == VK_ADD)
                    {
                        Input->SpeedUp = KeyIsDown;
                    }
                    else if(KeyCode == VK_SUBTRACT)
                    {
                        Input->SpeedDown = KeyIsDown;
                    }
                    else if(KeyCode == 'R' && KeyIsDown)
                    {
                        Input->RenderMode++;
                        if(Input->RenderMode > 1) 
                        {
                            Input->RenderMode = 0;
                        }
                    }
                    else if(KeyCode == 'B' && KeyIsDown)
                    {
                        Input->DeformerSign = -Input->DeformerSign;
                    }
                    else if(KeyCode == 'G' && KeyIsDown)
                    {
                        Input->ShowDebugGrid = !Input->ShowDebugGrid;
                    }
                    else if(KeyCode == 'H' && KeyIsDown)
                    {
                        Input->ShowDebugAxis = !Input->ShowDebugAxis;
                    }
                    else if(KeyCode == VK_ESCAPE && KeyIsDown)
                    {
                       GlobalRunning = false;
                    }
                    
                }
            } break;
        
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            }break;
        }
    }
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
            game_memory Memory;
            Memory.Size = GIGABYTE(1);
            Memory.Base = VirtualAlloc(NULL, Memory.Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if(Memory.Base == NULL)
            {
                win32_printer::DebugPrint("Cannot allocate enough memory");
                return 1;
            }
        
            dx_resource DXResources;
            HRESULT HResult = DXResources.Initialize(Window, ScreenInfo.Width, ScreenInfo.Height);
            if(FAILED(HResult))
            {
                char* ErrMsg = DXResources.GetDebugMessage(HResult);
                win32_printer::DebugPrint("Initialize error: %s", ErrMsg);
#if TEREPGEN_DEBUG
                char DebugBuffer[256];
                sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Initialize error: %s\n", ErrMsg);
                MessageBox(NULL, DebugBuffer, NULL, MB_OK);
#endif
                DXResources.Release();
                return 1;
            }
            
            camera Camera;
            Camera.Initialize(&DXResources, ScreenInfo.Width, ScreenInfo.Height, 20.0f);
            
            game_input Inputs[2] = {DefaultGameInput(), DefaultGameInput()};
            game_input *NewInput = &Inputs[0];
            game_input *OldInput = &Inputs[1];
            
            game_state *GameState = (game_state*)Memory.Base;
            GameState->Initialized = false;
            GameState->DXResources = &DXResources;
            
            QueryPerformanceFrequency(&GlobalPerfCountFrequency);
            win32_clock FrameClock;
            win32_clock WorldClock;
            LARGE_INTEGER FrameStartTime = Win32GetWallClock();
            LARGE_INTEGER WorldTime = FrameStartTime;
            
            while(GlobalRunning)
            {
                CopyInput(NewInput, OldInput);
                
                Win32HandleMessages(NewInput);
                
                if(Resize)
                {
                    ScreenInfo = GetWindowDimension(Window);
                    HResult = DXResources.Resize(ScreenInfo.Width, ScreenInfo.Height);
                    if(FAILED(HResult)) 
                    {
                        char* ErrMsg = DXResources.GetDebugMessage(HResult);
                        win32_printer::DebugPrint("Resize error: %s", ErrMsg);
                        break;
                    }
                    Camera.Resize(ScreenInfo.Width, ScreenInfo.Height);
                    Resize = false;
                }
        
                // NOTE: Update
                NewInput->OldMouseX = OldInput->MouseX;
                NewInput->OldMouseY = OldInput->MouseY;
                
                POINT MouseP;
                GetCursorPos(&MouseP);
                ScreenToClient(Window, &MouseP);
                if(GetActiveWindow() == Window)
                {
                    NewInput->MouseX = MouseP.x;
                    NewInput->MouseY = MouseP.y;
                    NewInput->MouseLeftButton = GetKeyState(VK_LBUTTON) & (1 << 15);
                    NewInput->MouseRightButton = GetKeyState(VK_RBUTTON) & (1 << 15);
                }
                
                GameState->dtForFrame = WorldClock.GetSecondsElapsed();
                WorldClock.Reset();
                
                UpdateAndRenderGame(&Memory, NewInput, &Camera, ScreenInfo);
                
                game_input *Temp = NewInput;
                NewInput = OldInput;
                OldInput = Temp;
                
                // FrameClock.PrintMiliSeconds("Frame time:");
                // win32_printer::PerfPrint("---------------------------");
                CalculateAvarageTime(FrameClock, &GameState->FrameAvg);
                if(GameState->FrameAvg.MeasureCount > 50.0f)
                {
                    // win32_printer::PerfPrint("Avg frame time: %f", GameState->FrameAvg.AvgTime * 1000.0);
                    GameState->FrameAvg.MeasureCount = 0.0f;
                    GameState->FrameAvg.AvgTime = 0.0f;
                }
                FrameClock.Reset();
            }
            
            SaveGameState(&Memory);
            
            Camera.Release();
            DXResources.Release();
        }
    }
    
    return 0;
}