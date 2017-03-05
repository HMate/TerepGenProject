/*
    Terep generátor by Hidvégi Máté @2015

*/

#include <windows.h>
#include <stdio.h>

#include "terepgen.h"

global_variable bool32 GlobalRunning = true;
global_variable bool32 GlobalResize;
global_variable LARGE_INTEGER GlobalPerfCountFrequency;  


void TerminateGame()
{
    GlobalRunning = false;
}

void logger::Print(char *Text)
{
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN] %s\n", Text);
    OutputDebugStringA(DebugBuffer);
}
    
void logger::Print(char *Text, real64 Arg1)
{
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN] %s\n", Text);
    char DebugBuffer2[256];
    sprintf_s(DebugBuffer2, DebugBuffer, Arg1);
    OutputDebugStringA(DebugBuffer2);
}

void logger::Print(char *Text, uint32 Arg1)
{
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN] %s\n", Text);
    char DebugBuffer2[256];
    sprintf_s(DebugBuffer2, DebugBuffer, Arg1);
    OutputDebugStringA(DebugBuffer2);
}

void logger::PerfPrint(char *Text, real64 Arg1)
{
#if TEREPGEN_PERF
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_PERF] %s\n", Text);
    char DebugBuffer2[256];
    sprintf_s(DebugBuffer2, DebugBuffer, Arg1);
    OutputDebugStringA(DebugBuffer2);
#endif
}

void logger::PerfPrint(char *Text, real64 Arg1, uint32 Arg2)
{
#if TEREPGEN_PERF
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_PERF] %s\n", Text);
    char DebugBuffer2[256];
    sprintf_s(DebugBuffer2, DebugBuffer, Arg1, Arg2);
    OutputDebugStringA(DebugBuffer2);
#endif
}

void logger::DebugPrint(char *Text)
{
#if TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s\n", Text);
    OutputDebugStringA(DebugBuffer);
#endif
}

void logger::DebugPrint(char *Text, char *Arg1)
{
#if TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s\n", Text);
    char DebugBuffer2[256];
    sprintf_s(DebugBuffer2, DebugBuffer, Arg1);
    OutputDebugStringA(DebugBuffer2);
#endif
}

void logger::DebugPrint(char *Text, uint32 Arg1)
{
#if TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s\n", Text);
    char DebugBuffer2[256];
    sprintf_s(DebugBuffer2, DebugBuffer, Arg1);
    OutputDebugStringA(DebugBuffer2);
#endif
}

void logger::DebugPrint(char *Text, uint32 Arg1, char *Arg2)
{
#if TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s\n", Text);
    char DebugBuffer2[256];
    sprintf_s(DebugBuffer2, DebugBuffer, Arg1, Arg2);
    OutputDebugStringA(DebugBuffer2);
#endif
}

void logger::DebugPrint(char *Text, real32 Arg1)
{
#if TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s\n", Text);
    char DebugBuffer2[256];
    sprintf_s(DebugBuffer2, DebugBuffer, Arg1);
    OutputDebugStringA(DebugBuffer2);
#endif
}

void logger::DebugPrint(char *Text, real32 Arg1, real32 Arg2)
{
#if TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s\n", Text);
    char DebugBuffer2[256];
    sprintf_s(DebugBuffer2, DebugBuffer, Arg1, Arg2);
    OutputDebugStringA(DebugBuffer2);
#endif
}

void logger::DebugPrint(char *Text, real64 Arg1)
{
#if TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_PERF] %s\n", Text);
    char DebugBuffer2[256];
    sprintf_s(DebugBuffer2, DebugBuffer, Arg1);
    OutputDebugStringA(DebugBuffer2);
#endif
}

// NOTE: Requires GlobalPerfCountFrequency to be initialized    
void timer::Reset()
{
    LARGE_INTEGER Current;
    QueryPerformanceCounter(&Current);
    Start = Current.QuadPart;
}
    
real64 timer::GetSecondsElapsed()
{
    LARGE_INTEGER End;
    QueryPerformanceCounter(&End);
    real64 Result = ((real64)(End.QuadPart - Start) /
                    (real64)GlobalPerfCountFrequency.QuadPart);
    return Result;
}

void timer::PrintMiliSeconds(char *PrintText)
{
    real64 SecondsElapsed = GetSecondsElapsed();
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s %f ms\n",
        PrintText, SecondsElapsed * 1000.0);
    OutputDebugStringA(DebugBuffer);
}

void timer::PrintSeconds(char *PrintText)
{
    real64 SecondsElapsed = GetSecondsElapsed();
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] %s %f s\n",
        PrintText, SecondsElapsed);
    OutputDebugStringA(DebugBuffer);
}

void timer::CalculateAverageTime(avarage_time *Avarage)
{
    real64 CurrentTime = GetSecondsElapsed();
    real64 LastMeasure = Avarage->MeasureCount;
    Avarage->MeasureCount += 1.0f;
    Avarage->AvgTime = 
        (LastMeasure/Avarage->MeasureCount)*Avarage->AvgTime + (CurrentTime / Avarage->MeasureCount);
}

FileHandle
PlatformOpenFileForRead(char *FileName)
{
    FileHandle Handle = CreateFile(FileName, GENERIC_READ,
        FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    Assert(Handle != INVALID_HANDLE_VALUE);
    return Handle;
}

FileHandle
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

// NOTE: Gives back number of bytes read
uint32
PlatformReadFile(FileHandle Handle, void *Dest, uint32 Size)
{
    uint32 BytesRead;
    ReadFile(Handle, Dest, Size, (LPDWORD)&BytesRead, NULL);
    return BytesRead;
}

uint32
PlatformWriteFile(FileHandle Handle, void *Source, uint32 Size)
{
    uint32 BytesWritten;
    WriteFile(Handle, Source, Size, (LPDWORD)&BytesWritten, NULL);
    return BytesWritten;
}

uint32 
PlatformSetFilePosition(FileHandle Handle, uint32 PositionFromStart)
{
    uint32 Result = SetFilePointer(Handle, PositionFromStart, NULL, FILE_BEGIN);
    return Result;
}

uint32 
PlatformIncrementFilePosition(FileHandle Handle, uint32 Step)
{
    uint32 Result = SetFilePointer(Handle, Step, NULL, FILE_CURRENT);
    return Result;
}

void
PlatformCloseFile(FileHandle Handle)
{
    CloseHandle(Handle);
}

void
PlatformDeleteFile(char* FileName)
{
    DeleteFile(FileName);
}

void
PlatformRenameFile(char* OldName, char* NewFileName)
{
    MoveFile(OldName, NewFileName);
}

bool32
FileIsEmpty(FileHandle Handle)
{
    bool32 IsEmpty = (0 == GetFileSize(Handle, NULL));
    return IsEmpty;
}

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
            logger::DebugPrint("message arrived: WM_DESTROY");
            TerminateGame();
        } break;

        case WM_CLOSE:
        {
            logger::DebugPrint("message arrived: WM_CLOSE");
            TerminateGame();
        } break;
        
        case WM_SIZE:
        {
            logger::DebugPrint("message arrived: WM_SIZE");
            GlobalResize = true;
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
                       TerminateGame();
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
    ScreenInfo.Width = MaxScreenWidth*3/4;
    ScreenInfo.Height = MaxScreenHeight*3/4;
    
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
            // NOTE: Allocate game memory at start. 
            // We dont allow to use more than this amount during the game
            game_memory Memory;
            Memory.PermanentStorageSize = MEGABYTE(512);
            Memory.TransientStorageSize = MEGABYTE(512);
            uint64 MemoryTotalSize = Memory.PermanentStorageSize + Memory.TransientStorageSize;
            Memory.PermanentStorage = VirtualAlloc(NULL, MemoryTotalSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            Memory.TransientStorage = (uint8*)Memory.PermanentStorage + Memory.PermanentStorageSize;
            if(Memory.PermanentStorage == NULL)
            {
                logger::DebugPrint("Cannot allocate enough memory");
                return 1;
            }
            game_state *GameState = (game_state*)Memory.PermanentStorage;
                    
            game_input Inputs[2] = {DefaultGameInput(), DefaultGameInput()};
            game_input *NewInput = &Inputs[0];
            game_input *OldInput = &Inputs[1];
            
            QueryPerformanceFrequency(&GlobalPerfCountFrequency);
            timer FrameClock;
            timer WorldClock;
            
            while(GlobalRunning)
            {
                CopyInput(NewInput, OldInput);
                
                Win32HandleMessages(NewInput);
                ScreenInfo = GetWindowDimension(Window);
                        
                // NOTE: Update Mouse inputs
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
                
                UpdateAndRenderGame(&Memory, NewInput, ScreenInfo, GlobalResize);
                
                game_input *Temp = NewInput;
                NewInput = OldInput;
                OldInput = Temp;
                
                GlobalResize = false;
                
                // FrameClock.PrintMiliSeconds("Frame time:");
                // logger::PerfPrint("---------------------------");
                FrameClock.CalculateAverageTime(&GameState->FrameAvg);
                if(GameState->FrameAvg.MeasureCount > 50.0f)
                {
                    // logger::PerfPrint("Avg frame time: %f", GameState->FrameAvg.AvgTime * 1000.0);
                    GameState->FrameAvg.MeasureCount = 0.0f;
                    GameState->FrameAvg.AvgTime = 0.0f;
                }
                FrameClock.Reset();
            }
            
            SaveGameState(&Memory);
        }
    }
    
    return 0;
}