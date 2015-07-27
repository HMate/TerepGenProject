#if !defined(TEREPGEN_VECTOR_H)
/*
    Terep generátor by Hidvégi Máté
*/


struct v3
{
    real32 X, Y, Z;
    v3 operator+(v3 &Other)
    {
        v3 Result = {this->X + Other.X, this->Y + Other.Y, this->Z + Other.Z};
        return Result;
    }
    v3 operator-(v3 &Other)
    {
        v3 Result = {this->X - Other.X, this->Y - Other.Y, this->Z - Other.Z};
        return Result;
    }
    v3 operator*(const real32 &Other)
    {
        v3 Result = {this->X * Other, this->Y * Other, this->Z * Other};
        return Result;
    }
    v3 operator/(const real32 &Other)
    {
        v3 Result = {this->X / Other, this->Y / Other, this->Z / Other};
        return Result;
    }
    bool32 operator == (v3 &Other)
    {
        real32 delta = 0.0001f;
        if((this->X > Other.X + delta) || (this->X < Other.X - delta))
        {
            return false;
        }
        if((this->Y > Other.Y + delta) || (this->Y < Other.Y - delta))
        {
            return false;
        }
        if((this->Z > Other.Z + delta) || (this->Z < Other.Z - delta))
        {
            return false;
        }
        return true;
    }
};

internal 
v3 Normalize(v3 Vec)
{
    real32 Length = Sqrt(Vec.X*Vec.X + Vec.Y*Vec.Y + Vec.Z*Vec.Z);
    v3 Result = v3{Vec.X/Length, Vec.Y/Length, Vec.Z/Length};
    return Result;
}

internal
real32 DotProduct(v3 A, v3 B)
{
    real32 Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z;
    return Result;
}

internal 
v3 Cross(v3 A, v3 B)
{
    v3 Result;
    Result.X = A.Y*B.Z - A.Z*B.Y;
    Result.Y = A.Z*B.X - A.X*B.Z;
    Result.Z = A.X*B.Y - A.Y*B.X;
    return Result;
}

// v3 v3::operator+(v3 &Other)
// {
    // v3 Result = {this->X + Other.X, this->Y + Other.Y, this->Z + Other.Z};
    // return Result;
// }

// v3 v3::operator-(v3 &Other)
// {
    // v3 Result = {this->X - Other.X, this->Y - Other.Y, this->Z - Other.Z};
    // return Result;
// }


#define TEREPGEN_VECTOR_H
#endif