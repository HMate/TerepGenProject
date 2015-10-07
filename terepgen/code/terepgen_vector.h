#if !defined(TEREPGEN_VECTOR_H)
/*
    Terep generátor by Hidvégi Máté
*/

struct v2
{
    real32 X, Y;
};

inline v2 
operator+(v2 A, v2 B)
{
    v2 Result = {A.X + B.X, A.Y + B.Y};
    return Result;
}

inline v2 
operator-(v2 A, v2 B)
{
    v2 Result = {A.X - B.X, A.Y - B.Y};
    return Result;
}

inline v2 
operator*(v2 A, real32 B)
{
    v2 Result = {A.X * B, A.Y * B};
    return Result;
}

inline v2
operator*(real32 A, v2 B)
{
    return B * A;
}

inline v2 
operator/(v2 A, real32 B)
{
    v2 Result = {A.X / B, A.Y / B};
    return Result;
}

inline bool32 
operator == (v2 A, v2 B)
{
    real32 delta = 0.0001f;
    if((A.X > B.X + delta) || (A.X < B.X - delta) ||
       (A.Y > B.Y + delta) || (A.Y < B.Y - delta))
    {
        return false;
    }
    return true;
}

inline bool32 
operator != (v2 A, v2 B)
{
    return !(A == B);
}

inline real32 Length(v2 Vec)
{
    real32 Result = Sqrt(Vec.X*Vec.X + Vec.Y*Vec.Y);
    return Result;
}

inline v2 
Normalize(v2 Vec)
{
    real32 Length = Sqrt(Vec.X*Vec.X + Vec.Y*Vec.Y);
    v2 Result = v2{Vec.X/Length, Vec.Y/Length};
    return Result;
}

inline real32 
DotProduct(v2 A, v2 B)
{
    real32 Result = A.X*B.X + A.Y*B.Y;
    return Result;
}


struct v3
{
    real32 X, Y, Z;
};

inline v3 
operator+(v3 A, v3 B)
{
    v3 Result = {A.X + B.X, A.Y + B.Y, A.Z + B.Z};
    return Result;
}

inline v3 
operator-(v3 A, v3 B)
{
    v3 Result = {A.X - B.X, A.Y - B.Y, A.Z - B.Z};
    return Result;
}

inline v3 
operator*(v3 A, real32 B)
{
    v3 Result = {A.X * B, A.Y * B, A.Z * B};
    return Result;
}

inline v3
operator*(real32 A, v3 B)
{
    return B * A;
}

inline v3 
operator/(v3 A, real32 B)
{
    v3 Result = {A.X / B, A.Y / B, A.Z / B};
    return Result;
}

inline bool32 
operator == (v3 A, v3 B)
{
    real32 delta = 0.0001f;
    if((A.X > B.X + delta) || (A.X < B.X - delta) ||
       (A.Y > B.Y + delta) || (A.Y < B.Y - delta) ||
       (A.Z > B.Z + delta) || (A.Z < B.Z - delta))
    {
        return false;
    }
    return true;
}

inline bool32 
operator != (v3 A, v3 B)
{
    return !(A == B);
}

inline real32
Length(v3 Vec)
{
    real32 Result = Sqrt(Vec.X*Vec.X + Vec.Y*Vec.Y + Vec.Z*Vec.Z);
    return Result;
}

inline v3 
Normalize(v3 Vec)
{
    real32 Len = Sqrt(Vec.X*Vec.X + Vec.Y*Vec.Y + Vec.Z*Vec.Z);
    v3 Result = v3{Vec.X/Len, Vec.Y/Len, Vec.Z/Len};
    return Result;
}

inline real32 
DotProduct(v3 A, v3 B)
{
    real32 Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z;
    return Result;
}

inline v3 
Cross(v3 A, v3 B)
{
    v3 Result;
    Result.X = A.Y*B.Z - A.Z*B.Y;
    Result.Y = A.Z*B.X - A.X*B.Z;
    Result.Z = A.X*B.Y - A.Y*B.X;
    return Result;
}



union v4
{
    struct
    {
        real32 X, Y, Z, W;
    };
    struct
    {
        real32 R, G, B, A;
    };
    real32 C[4];
};

inline v4 
operator+(v4 A, v4 B)
{
    v4 Result = {A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W};
    return Result;
}

inline v4 
operator-(v4 A, v4 B)
{
    v4 Result = {A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W};
    return Result;
}

inline v4 
operator*(v4 A, real32 B)
{
    v4 Result = {A.X * B, A.Y * B, A.Z * B, A.W * B};
    return Result;
}

inline v4
operator*(real32 A, v4 B)
{
    return B * A;
}

inline v4 
operator/(v4 A, real32 B)
{
    v4 Result = {A.X / B, A.Y / B, A.Z / B, A.W / B};
    return Result;
}

inline bool32 
operator == (v4 A, v4 B)
{
    real32 delta = 0.0001f;
    if((A.X > B.X + delta) || (A.X < B.X - delta) ||
       (A.Y > B.Y + delta) || (A.Y < B.Y - delta) ||
       (A.Z > B.Z + delta) || (A.Z < B.Z - delta) ||
       (A.W > B.W + delta) || (A.W < B.W - delta))
    {
        return false;
    }
    return true;
}

inline bool32 
operator != (v4 A, v4 B)
{
    return !(A == B);
}

inline real32 
Length(v4 Vec)
{
    real32 Result = Sqrt(Vec.X*Vec.X + Vec.Y*Vec.Y + Vec.Z*Vec.Z + Vec.W*Vec.W);
    return Result;
}

inline v4 
Normalize(v4 Vec)
{
    real32 Len = Length(Vec);
    v4 Result = v4{Vec.X/Len, Vec.Y/Len, Vec.Z/Len, Vec.Z/Len};
    return Result;
}

inline real32 
DotProduct(v4 A, v4 B)
{
    real32 Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z + A.W*B.W;
    return Result;
}


#define TEREPGEN_VECTOR_H
#endif