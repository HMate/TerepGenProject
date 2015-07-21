#if !defined(TEREPGEN_TYPES_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include <stdint.h>
#include <string>

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

#define ArrayCount(Array) ((sizeof(Array))/sizeof(*Array))
#define Assert(Test) if(!(Test)){*(int *)0 = 0;}

#define KILOBYTE(Size) ((Size)*1024)
#define MEGABYTE(Size) ((Size)*1024*1024)
#define GIGABYTE(Size) ((Size)*1024*1024*1024)

union color
{
    struct
    {
        float R, G, B, A;
    };
    real32 C[4];
};

struct screen_info
{
    uint32 Width;
    uint32 Height;
};

struct input
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
    
    int32 MouseX;
    int32 MouseY;
    int32 OldMouseX;
    int32 OldMouseY;
};



inline uint32 
FloorUint32(real32 Val)
{
    uint32 Result = (uint32)Val;
    return Result;
}

inline real32 Pow(real32 A, uint32 N)
{
    Assert(N>=0);
    if(N==0) return 1; 
    return A * Pow(A, N-1);
}

inline uint32 Pow2(uint32 N)
{
    Assert(N>=0);
    if(N==0) return 1; 
    return 2 * Pow2(N-1);
}

inline uint32 Log2(uint32 N)
{
    Assert(N>=1);
    if(N==1) return 0; 
    return 1 + Log2(N/2);
}

#define TEREPGEN_TYPES_H
#endif