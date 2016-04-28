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

inline int32
Abs(int32 Val)
{
    int32 Result = abs(Val);
    return Result;
}

inline real32
Abs(real32 Val)
{
    real32 Result = fabs(Val);
    return Result;
}

inline uint32 
FloorUint32(real32 Val)
{
    uint32 Result = (uint32)Val;
    return Result;
}

inline int32 
FloorInt32(real32 Val)
{
    int32 Result = (int32)floor(Val);
    return Result;
}

inline real32 
FloorReal32(real32 Val)
{
    real32 Result = (real32)floor(Val);
    return Result;
}

inline real32 
Pow(real32 A, uint32 N)
{
    Assert(N>=0);
    if(N==0) return 1; 
    return A * Pow(A, N-1);
}

inline uint32 
Pow2(uint32 N)
{
    Assert(N>=0);
    uint32 Result = 1 << N;
    return Result;
}

inline uint32 
Log2(uint32 N)
{
    Assert(N>=1);
    if(N==1) return 0; 
    return 1 + Log2(N/2);
}

inline real32 
Sqrt(real32 Val)
{
    real32 Result = sqrt(Val);
    return Result;
}

// NOTE: Cuberoot
inline int32
Cbrt(uint32 Val)
{
    int32 Result = (int32)cbrt(Val);
    return Result;
}

inline real32
Tan(real32 Val)
{
    real32 Result = tan(Val);
    return Result;
}

inline real32 
ClampReal32(real32 Value, real32 Min, real32 Max)
{
    real32 Result = Value;
    Result = (Result < Min) ? Min : Result;
    Result = (Result > Max) ? Max : Result;
    return Result;
}

inline real32
ModReal32(real32 Value, real32 Base)
{
    real32 Result = fmod(Value, Base);
    if(Result < 0.0f) 
        Result = Base + Result;
    return Result;
}

inline int32
Min(int32 A, int32 B)
{
    if(A < B)
    {
        return A;
    }
    else
    {
        return B;
    }
}

#define TEREPGEN_TYPES_H
#endif