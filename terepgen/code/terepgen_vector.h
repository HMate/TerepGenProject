#if !defined(TEREPGEN_VECTOR_H)
/*
    Terep generátor by Hidvégi Máté
*/


struct v3
{
    real32 X, Y, Z;
};

internal v3 
operator+(v3 A, v3 B)
{
    v3 Result = {A.X + B.X, A.Y + B.Y, A.Z + B.Z};
    return Result;
}

internal v3 
operator-(v3 A, v3 B)
{
    v3 Result = {A.X - B.X, A.Y - B.Y, A.Z - B.Z};
    return Result;
}

internal v3 
operator*(v3 A, real32 B)
{
    v3 Result = {A.X * B, A.Y * B, A.Z * B};
    return Result;
}

internal v3
operator*(real32 A, v3 B)
{
    return B * A;
}

internal v3 
operator/(v3 A, real32 B)
{
    v3 Result = {A.X / B, A.Y / B, A.Z / B};
    return Result;
}

internal bool32 
operator == (v3 A, v3 B)
{
    real32 delta = 0.0001f;
    if((A.X > B.X + delta) || (A.X < B.X - delta) ||
       (A.Y > B.Y + delta) || (A.Y < B.Y - delta) ||
       (A.Z > B.Z + delta) || (A.Z < B.Z - delta))
    {
        return false;
    }
    // if((A.Y > B.Y + delta) || (A.Y < B.Y - delta))
    // {
        // return false;
    // }
    // if((A.Z > B.Z + delta) || (A.Z < B.Z - delta))
    // {
        // return false;
    // }
    return true;
}

internal bool32 
operator != (v3 A, v3 B)
{
    return !(A == B);
}

internal v3 
Normalize(v3 Vec)
{
    real32 Length = Sqrt(Vec.X*Vec.X + Vec.Y*Vec.Y + Vec.Z*Vec.Z);
    v3 Result = v3{Vec.X/Length, Vec.Y/Length, Vec.Z/Length};
    return Result;
}

internal real32 
DotProduct(v3 A, v3 B)
{
    real32 Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z;
    return Result;
}

internal v3 
Cross(v3 A, v3 B)
{
    v3 Result;
    Result.X = A.Y*B.Z - A.Z*B.Y;
    Result.Y = A.Z*B.X - A.X*B.Z;
    Result.Z = A.X*B.Y - A.Y*B.X;
    return Result;
}


#define TEREPGEN_VECTOR_H
#endif