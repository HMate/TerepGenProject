#if !defined(TEREPGEN_TYPES_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include <stdint.h>
#include <stddef.h>
#include <crtdbg.h>
#include <stdio.h>

#define global_variable static
#define local_persist static
#define internal static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

// NOTE: This doesnt work, if the array is a pointer, (like when the array is a function parameter)
// because then sizeof gives the size of a pointer. But works when the array is part of a struct.
#define ArrayCount(Array) (sizeof(Array)/sizeof(*Array))

#if TEREPGEN_DEBUG
    #define Assert(Test) if(!(Test)){*(int*)0 = 2;}
#else
    #define Assert(Test) 
#endif

#define InvalidCodePath Assert(!"InvalidCodePath");

#define KILOBYTE(Size) (1024LL*(Size))
#define MEGABYTE(Size) (1024LL*1024LL*(Size))
#define GIGABYTE(Size) (1024LL*1024LL*1024LL*(Size))

struct game_memory
{
    uint64 PermanentStorageSize;
    void *PermanentStorage;
    
    uint64 TransientStorageSize;
    void *TransientStorage;
};

struct memory_arena
{
    uint64 TotalSize;
    uint64 Used;
    uint8* Base;
    uint32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    uint64 Used;
};

internal void
InitializeArena(memory_arena *Arena, uint8* Base, uint64 Size)
{
    Arena->TotalSize = Size;
    Arena->Used = 0;
    Arena->Base = Base;
    Arena->TempCount = 0;
};

#define PushStruct(Arena, type) (type *)PushElement_((Arena), sizeof(type))
#define PushArray(Arena, type, Size) (type *)PushElement_((Arena), sizeof(type)*(Size))
internal void *
PushElement_(memory_arena *Arena, uint32 Size)
{
    Assert(Arena->Used + Size < Arena->TotalSize);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    
    return Result;
}

internal temporary_memory 
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Temp;
    Temp.Arena = Arena;
    Temp.Used = Arena->Used;
    Arena->TempCount++;
    
    return Temp;
}

internal void
EndTemporaryMemory(temporary_memory *Temp)
{
    memory_arena *Arena = Temp->Arena;
    Assert(Arena->Used >= Temp->Used);
    Arena->Used = Temp->Used;
    
    Assert(Arena->TempCount > 0);
    Arena->TempCount--;
}

internal void
CheckMemoryArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

#include "terepgen_math.h"
#include "terepgen_vector.h"

struct screen_info
{
    int32 Width;
    int32 Height;
};

struct game_input
{
    bool32 MoveLeft;
    bool32 MoveRight;
    bool32 MoveForward;
    bool32 MoveBack;
    bool32 MoveUp;
    bool32 MoveDown;
    bool32 SpeedUp;
    bool32 SpeedDown;
    
    uint32 RenderMode;
    real32 DeformerSign;
    bool32 ShowDebugGrid;
    bool32 ShowDebugAxis;
    
    bool32 MouseLeftButton;
    bool32 MouseRightButton;
    
    int32 MouseX;
    int32 MouseY;
    int32 OldMouseX;
    int32 OldMouseY;
};

internal game_input 
DefaultGameInput()
{
    game_input Result;
    Result.MoveLeft = false;
    Result.MoveRight = false;
    Result.MoveForward = false;
    Result.MoveBack = false;
    Result.MoveUp = false;
    Result.MoveDown = false;
    Result.SpeedUp = false;
    Result.SpeedDown = false;
    
    Result.RenderMode = 0;
    Result.DeformerSign = 1.0f;
    Result.ShowDebugGrid = false;
    Result.ShowDebugAxis = false;
    
    Result.MouseLeftButton = false;
    Result.MouseRightButton = false;
    
    Result.MouseX = 0;
    Result.MouseY = 0;
    Result.OldMouseX = 0;
    Result.OldMouseY = 0;
    return Result;
}

internal void
CopyInput(game_input* NewInput, game_input *OldInput)
{
    NewInput->MoveLeft = OldInput->MoveLeft;
    NewInput->MoveRight = OldInput->MoveRight;
    NewInput->MoveForward = OldInput->MoveForward;
    NewInput->MoveBack = OldInput->MoveBack;
    NewInput->MoveUp = OldInput->MoveUp;
    NewInput->MoveDown = OldInput->MoveDown;
    NewInput->SpeedUp = OldInput->SpeedUp;
    NewInput->SpeedDown = OldInput->SpeedDown;
    
    NewInput->RenderMode = OldInput->RenderMode;
    NewInput->DeformerSign = OldInput->DeformerSign;
    NewInput->ShowDebugGrid = OldInput->ShowDebugGrid;
    NewInput->ShowDebugAxis = OldInput->ShowDebugAxis;
}


struct vertex
{
    // NOTE: position is in left handed coordinate system
    // +X points right initially, -X points left
    // +Y is vertical axis and points up 
    // -Z points through screen to user initially, +Z points toward screen 
    real32 X, Y, Z;
    real32 NX, NY, NZ;
    v4 Color;
};

vertex Vertex(v3 Pos, v3 Norm, v4 Color);
#define TEREPGEN_TYPES_H
#endif