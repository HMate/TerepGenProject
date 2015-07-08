#if !defined(TEREPGEN_TYPES_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include <stdint.h>

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

#define TEREPGEN_TYPES_H
#endif