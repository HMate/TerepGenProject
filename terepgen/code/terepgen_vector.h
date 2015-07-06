#if !defined(TEREPGEN_VECTOR_H)



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