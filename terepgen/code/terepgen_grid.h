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
    
    real32& GetPRC(int32 Plane, int32 Row, int32 Column)
    {
        if(Row > Dimension - 1) Row = Dimension - 1;
        if(Column > Dimension - 1) Column = Dimension - 1;
        if(Plane > Dimension - 1) Plane = Dimension - 1;
        return Elements[Plane*Dimension*Dimension + Row*Dimension + Column];
    }
};

#define GRID_DIMENSION 13

struct static_grid3D
{
    int32 Dimension = GRID_DIMENSION;
    real32 Elements[GRID_DIMENSION * GRID_DIMENSION * GRID_DIMENSION];
};

        
internal real32& 
GetGridPRC(static_grid3D *Grid, int32 Plane, int32 Row, int32 Column)
{
    uint32 Dimension = Grid->Dimension;
    Assert(Plane < GRID_DIMENSION && Row < GRID_DIMENSION && Column < GRID_DIMENSION);
    return Grid->Elements[Plane*Dimension*Dimension + Row*Dimension + Column];
}

internal real32 
GetGridPRCWithInterpolate(static_grid3D *Grid, real32 Plane, real32 Row, real32 Column)
{
    uint32 Dimension = Grid->Dimension;
    Assert(Plane >= 0.0f && Plane <= (real32)(Dimension - 1));
    Assert(Row >= 0.0f && Row <= (real32)(Dimension - 1));
    Assert(Column >= 0.0f && Column <= (real32)(Dimension - 1));
    
    uint32 PlaneWhole = FloorUint32(Plane);
    real32 PlaneRemainder = Plane - (real32)PlaneWhole;
    uint32 RowWhole = FloorUint32(Row);
    real32 RowRemainder = Row - (real32)RowWhole;
    uint32 ColumnWhole = FloorUint32(Column);
    real32 ColumnRemainder = Column - (real32)ColumnWhole;
    
    // NOTE: If every parameter is whole number, we can just give back the grid value
    if(PlaneRemainder < 0.0001f && RowRemainder < 0.0001f && ColumnRemainder < 0.0001f)
        return Grid->Elements[PlaneWhole*Dimension*Dimension + RowWhole*Dimension + ColumnWhole];
    else if(PlaneRemainder < 0.0001f && RowRemainder < 0.0001f)
    {
        real32 Elem1 = Grid->Elements[PlaneWhole*Dimension*Dimension + RowWhole*Dimension + ColumnWhole];
        real32 Elem2 = Grid->Elements[PlaneWhole*Dimension*Dimension + RowWhole*Dimension + ColumnWhole + 1];
    
        real32 Result = Elem1 + ColumnRemainder * (Elem2 - Elem1);
        return Result;
    }
    else if(PlaneRemainder < 0.0001f)
    {
        real32 Elem1 = GetGridPRCWithInterpolate(Grid, Plane, (real32)RowWhole, Column);
        real32 Elem2 = GetGridPRCWithInterpolate(Grid, Plane, (real32)(RowWhole+1), Column);
        
        real32 Result = Elem1 + RowRemainder * (Elem2 - Elem1);
        return Result;
    }
    else
    {
        real32 Elem1 = GetGridPRCWithInterpolate(Grid, (real32)PlaneWhole, Row, Column);
        real32 Elem2 = GetGridPRCWithInterpolate(Grid, (real32)(PlaneWhole+1), Row, Column);
        
        real32 Result = Elem1 + PlaneRemainder * (Elem2 - Elem1);
        return Result;
    }
}

    
internal void 
ZeroOutGridPoints(static_grid3D *Grid)
{
    int32 Dimension = Grid->Dimension;
    for(int32 Plane = 0;
        Plane < Dimension;
        ++Plane)
    {
        for(int32 Row = 0;
            Row < Dimension;
            ++Row)
        {
            for(int32 Column = 0;
                Column < Dimension;
                ++Column)
            {
                Grid->Elements[Plane*Dimension*Dimension + Row*Dimension + Column] = 0.0f;
            }
        }
    }
}

#define TEREPGEN_GRID_H
#endif