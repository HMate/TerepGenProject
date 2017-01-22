#if !defined(TEREPGEN_MATH_H)
/*
    Terep generátor by Hidvégi Máté @2016
*/

#include <cmath>

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
#define TEREPGEN_MATH_H
#endif
