#if !defined(TEREPGEN_GRID_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#define GRID_DIMENSION 33

struct dynamic_grid3D
{
    uint32 Dimension;
    real32* Elements;
    
    dynamic_grid3D()
    {
        this->Dimension = 0;
        Elements = nullptr;
    }
    
    dynamic_grid3D(uint32 Dimension)
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
    
    real32& GetPRC(uint32 Plane, uint32 Row, uint32 Column)
    {
        if(Row > Dimension - 1) Row = Dimension - 1;
        if(Column > Dimension - 1) Column = Dimension - 1;
        if(Plane > Dimension - 1) Plane = Dimension - 1;
        return Elements[Plane*Dimension*Dimension + Row*Dimension + Column];
    }
};

struct grid3D
{
    uint32 Dimension;
    real32* Elements;
    
    grid3D()
    {
        this->Dimension = 0;
        Elements = nullptr;
    }
    
    grid3D(uint32 Dimension)
    {
        this->Dimension = Dimension;
        Elements = new real32[Dimension * Dimension * Dimension];
    }
    
    grid3D(const grid3D&) = delete;
    // grid3D operator=(const grid3D&) = delete;
    grid3D& operator=(grid3D& Other)
    {
        this->Dimension = Other.Dimension;
        this->Elements = Other.Elements;
        Other.Elements = nullptr;
        
        return *this;
    }
    
    ~grid3D()
    {
        if(Elements)
        {
            delete[] Elements;
            Elements = nullptr;
        }
    }
    
    real32& GetPRC(uint32 Plane, uint32 Row, uint32 Column)
    {
        if(Row > Dimension - 1) Row = Dimension - 1;
        if(Column > Dimension - 1) Column = Dimension - 1;
        if(Plane > Dimension - 1) Plane = Dimension - 1;
        return Elements[Plane*Dimension*Dimension + Row*Dimension + Column];
    }
    
    // TODO: Use interpolating like in random generator?
    real32 GetPRCWithInterpolate(real32 Plane, real32 Row, real32 Column)
    {
        Assert(Plane >= 0.0f && Plane <= (real32)(Dimension - 1));
        Assert(Row >= 0.0f && Row <= (real32)(Dimension - 1));
        Assert(Column >= 0.0f && Column <= (real32)(Dimension - 1));
        
        uint32 PlaneWhole = FloorUint32(Plane);
        real32 PlaneRemainder = Plane - (real32)PlaneWhole;
        uint32 RowWhole = FloorUint32(Row);
        real32 RowRemainder = Row - (real32)RowWhole;
        uint32 ColumnWhole = FloorUint32(Column);
        real32 ColumnRemainder = Column - (real32)ColumnWhole;
        
        if(PlaneRemainder < 0.0001f && RowRemainder < 0.0001f && ColumnRemainder < 0.0001f)
            return Elements[PlaneWhole*Dimension*Dimension + RowWhole*Dimension + ColumnWhole];
        else if(RowRemainder < 0.0001f && ColumnRemainder < 0.0001f)
        {
            real32 Elem1 = Elements[PlaneWhole*Dimension*Dimension + RowWhole*Dimension + ColumnWhole];
            real32 Elem2 = Elements[(PlaneWhole+1)*Dimension*Dimension + RowWhole*Dimension + ColumnWhole];
        
            real32 Result = Elem1 + PlaneRemainder * (Elem2 - Elem1);
            return Result;
        }
        else if(ColumnRemainder < 0.0001f)
        {
            real32 Elem1 = GetPRCWithInterpolate(Plane, (real32)RowWhole, Column);
            real32 Elem2 = GetPRCWithInterpolate(Plane, (real32)(RowWhole+1), Column);
            
            real32 Result = Elem1 + RowRemainder * (Elem2 - Elem1);
            return Result;
        }
        else
        {
            real32 Elem1 = GetPRCWithInterpolate(Plane, Row, (real32)ColumnWhole);
            real32 Elem2 = GetPRCWithInterpolate(Plane, Row, (real32)(ColumnWhole+1));
            
            real32 Result = Elem1 + ColumnRemainder * (Elem2 - Elem1);
            return Result;
        }
    }
    
    void ZeroOutGridPoints()
    {
        for(uint32 Plane = 0;
            Plane < Dimension;
            ++Plane)
        {
            for(uint32 Row = 0;
                Row < Dimension;
                ++Row)
            {
                for(uint32 Column = 0;
                    Column < Dimension;
                    ++Column)
                {
                    GetPRC(Plane, Row, Column) = 0.0f;
                }
            }
        }
    }
};




#define TEREPGEN_GRID_H
#endif