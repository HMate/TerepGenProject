#if !defined(TEREPGEN_TYPES_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include <stdint.h>
#include <stddef.h>
#include <cmath>

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
#define Assert(Test) if(!(Test)){*(int *)0 = 0;}
#else
#define Assert(Test) 
#endif

#define KILOBYTE(Size) (1024LL*(Size))
#define MEGABYTE(Size) (1024LL*1024LL*(Size))
#define GIGABYTE(Size) (1024LL*1024LL*1024LL*(Size))

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
    
    Result.MouseLeftButton = false;
    Result.MouseRightButton = false;
    
    Result.MouseX = 0;
    Result.MouseY = 0;
    Result.OldMouseX = 0;
    Result.OldMouseY = 0;
    return Result;
}

#define TEREPGEN_TYPES_H
#endif