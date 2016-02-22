#if !defined(TEREPGEN_GRID_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

struct dynamic_grid3D
{
    int32 Dimension;
    real32* Elements;
    
    dynamic_grid3D()
    {
        this->Dimension = 0;
        Elements = nullptr;
    }
    
    dynamic_grid3D(int32 Dimension)
    {
        this->Dimension = Dimension;
        Elements = new real32[Dimension * Dimension * Dimension];
    }
    
    dynamic_grid3D(const dynamic_grid3D&) = delete;
    dynamic_grid3D operator=(const dynamic_grid3D&) = delete;
    
    ~dynamic_grid3D()
    {
        if(Elements)
        {
            delete[] Elements;
            Elements = nullptr;
        }
    }
    
    real32& GetPRC(int32 X, int32 Y, int32 Z)
    {
        if(X > Dimension - 1) X = Dimension - 1;
        if(Y > Dimension - 1) Y = Dimension - 1;
        if(Z > Dimension - 1) Z = Dimension - 1;
        return Elements[X*Dimension*Dimension + Y*Dimension + Z];
    }
};

#define GRID_DIMENSION 13

struct static_grid3D
{
    int32 Dimension = GRID_DIMENSION;
    real32 Elements[GRID_DIMENSION * GRID_DIMENSION * GRID_DIMENSION];
};

        
internal real32 
GetGrid(static_grid3D *Grid, int32 X, int32 Y, int32 Z)
{
    uint32 Dim = Grid->Dimension;
    Assert(X < GRID_DIMENSION && Y < GRID_DIMENSION && Z < GRID_DIMENSION);
    return Grid->Elements[X*Dim*Dim + Y*Dim + Z];
}
        
internal void 
SetGrid(static_grid3D *Grid, int32 X, int32 Y, int32 Z, real32 Value)
{
    uint32 Dim = Grid->Dimension;
    Assert(X < GRID_DIMENSION && Y < GRID_DIMENSION && Z < GRID_DIMENSION);
    Grid->Elements[X*Dim*Dim + Y*Dim + Z] = Value;
}

internal real32 
GetGridWithInterpolate(static_grid3D *Grid, real32 X, real32 Y, real32 Z)
{
    uint32 Dim = Grid->Dimension;
    Assert(X >= 0.0f && X <= (real32)(Dim - 1));
    Assert(Y >= 0.0f && Y <= (real32)(Dim - 1));
    Assert(Z >= 0.0f && Z <= (real32)(Dim - 1));
    
    uint32 XFloor = FloorUint32(X);
    real32 XRemainder = X - (real32)XFloor;
    uint32 YFloor = FloorUint32(Y);
    real32 YRemainder = Y - (real32)YFloor;
    uint32 ZFloor = FloorUint32(Z);
    real32 ZRemainder = Z - (real32)ZFloor;
    
    // NOTE: If every parameter is whole number, we can just give back the grid value
    if(XRemainder < 0.0001f && YRemainder < 0.0001f && ZRemainder < 0.0001f)
        return Grid->Elements[XFloor*Dim*Dim + YFloor*Dim + ZFloor];
    else if(XRemainder < 0.0001f && YRemainder < 0.0001f)
    {
        real32 Elem1 = Grid->Elements[XFloor*Dim*Dim + YFloor*Dim + ZFloor];
        real32 Elem2 = Grid->Elements[XFloor*Dim*Dim + YFloor*Dim + ZFloor + 1];
    
        real32 Result = Elem1 + ZRemainder * (Elem2 - Elem1);
        return Result;
    }
    else if(XRemainder < 0.0001f)
    {
        real32 Elem1 = GetGridWithInterpolate(Grid, X, (real32)YFloor, Z);
        real32 Elem2 = GetGridWithInterpolate(Grid, X, (real32)(YFloor+1), Z);
        
        real32 Result = Elem1 + YRemainder * (Elem2 - Elem1);
        return Result;
    }
    else
    {
        real32 Elem1 = GetGridWithInterpolate(Grid, (real32)XFloor, Y, Z);
        real32 Elem2 = GetGridWithInterpolate(Grid, (real32)(XFloor+1), Y, Z);
        
        real32 Result = Elem1 + XRemainder * (Elem2 - Elem1);
        return Result;
    }
}
    
inline real32 
GetElement(static_grid3D *Grid, uint32 Index)
{
    Assert(Index < GRID_DIMENSION * GRID_DIMENSION * GRID_DIMENSION);
    real32 Result = Grid->Elements[Index];
    return Result;
}
    
internal void 
ZeroOutGridPoints(static_grid3D *Grid)
{
    int32 Dim = Grid->Dimension;
    for(int32 X = 0; X < Dim; ++X)
    {
        for(int32 Y = 0; Y < Dim; ++Y)
        {
            for(int32 Z = 0; Z < Dim; ++Z)
            {
                Grid->Elements[X*Dim*Dim + Y*Dim + Z] = 0.0f;
            }
        }
    }
}

#define TEREPGEN_GRID_H
#endif