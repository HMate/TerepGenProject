#if !defined(TEREPGEN_GRID_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include <memory>
#include <functional>

struct grid2D
{
    uint32 Dimension;
    std::shared_ptr<real32> Elements;
    
    grid2D()
    {
        this->Dimension = 0;
        Elements = nullptr;
    }
    
    grid2D(uint32 Dimension)
    {
        this->Dimension = Dimension;
        Elements = std::shared_ptr<real32>(new real32[Dimension * Dimension]);
    }
    
    bool32 operator==(const grid2D &OtherGrid)
    {
        bool32 Result = true;
        if(this->Dimension != OtherGrid.Dimension) Result = false;
        else if(this->Elements != OtherGrid.Elements) Result = false;
        return Result;
    }
    
    bool32 operator!=(const grid2D &OtherGrid)
    {
        return !(*this == OtherGrid);
    }
    
    grid2D& operator +=(grid2D& OtherGrid)
    {
        for(uint32 Row = 0;
            Row < Dimension;
            ++Row)
        {
            for(uint32 Column = 0;
                Column < Dimension;
                ++Column)
            {
                GetXY(Row, Column) += OtherGrid.GetXY(Row, Column);
            }
        }
        return *this;
    }
    
    real32& GetXY(int32 Row, int32 Column)
    {
        if(Row < 0) Row = 0;
        else if(Row > Dimension - 1) Row = Dimension - 1;
        if(Column < 0) Column = 0;
        else if(Column > Dimension - 1) Column = Dimension - 1;
        return Elements.get()[Row*Dimension + Column];
    }
    
    void ZeroOutGridPoints()
    {
        for(uint32 Row = 0;
            Row < Dimension;
            ++Row)
        {
            for(uint32 Column = 0;
                Column < Dimension;
                ++Column)
            {
                GetXY(Row, Column) = 0.0f;
            }
        }
    }
    
    struct GridRow
    {
        float* Elem;
        uint32 Dimension;
        GridRow(float *Elem, uint32 Dimension) : Elem(Elem), Dimension(Dimension){}
        float& operator[](int32 Index)
        {
            if(Index < 0) Index = 0;
            else if(Index > Dimension-1) Index = Dimension-1;
            return *(Elem+Index);
        }
    };
    
    // NOTE: GridRow& would be faster, but its dangerous,
    //       because it gives back the reference of a local variable. 
    // TODO: could it be fixed?
    GridRow operator[](int32 Index)
    {
        if(Index < 0) Index = 0;
        else if(Index > Dimension-1) Index = Dimension-1;
        float *Elem = &(Elements.get()[Index*Dimension]);
        return GridRow(Elem, Dimension);
    }
};


struct grid3D
{
    uint32 Dimension;
    // TODO: shared pointers should be vreated with make_shared, but its not trivial with arrays
    std::shared_ptr<real32> Elements;
    
    grid3D()
    {
        this->Dimension = 0;
        Elements = nullptr;
    }
    
    // ~grid3D()
    // {
        // OutputDebugStringA("[TEREPGEN_DEBUG] Grid3D being deleted\n");
    // }
    
    grid3D(uint32 Dimension)
    {
        this->Dimension = Dimension;
        // NOTE: sharedptr array needs custom deleter.
        Elements = std::shared_ptr<real32>(new real32[Dimension * Dimension * Dimension],
            [](real32 *E){delete[] E;});
    }
    
    bool32 operator==(const grid3D &OtherGrid)
    {
        bool32 Result = true;
        if(this->Dimension != OtherGrid.Dimension) Result = false;
        else if(this->Elements != OtherGrid.Elements) Result = false;
        return Result;
    }
    
    bool32 operator!=(const grid3D &OtherGrid)
    {
        return !(*this == OtherGrid);
    }
    
    grid3D& operator +=(grid3D& OtherGrid)
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
                    GetPRC(Plane, Row, Column) += OtherGrid.GetPRC(Plane, Row, Column);
                }
            }
        }
        return *this;
    }
    
    real32& GetPRC(uint32 Plane, uint32 Row, uint32 Column)
    {
        if(Row > Dimension - 1) Row = Dimension - 1;
        if(Column > Dimension - 1) Column = Dimension - 1;
        if(Plane > Dimension - 1) Plane = Dimension - 1;
        return Elements.get()[Plane*Dimension*Dimension + Row*Dimension + Column];
    }
    
    real32& GetPRC(int32 Plane, int32 Row, int32 Column)
    {
        if(Row < 0) Row = 0;
        else if(Row > Dimension - 1) Row = Dimension - 1;
        if(Column < 0) Column = 0;
        else if(Column > Dimension - 1) Column = Dimension - 1;
        if(Plane < 0) Plane = 0;
        else if(Plane > Dimension - 1) Plane = Dimension - 1;
        return Elements.get()[Plane*Dimension*Dimension + Row*Dimension + Column];
    }
    
    // TODO: Do I need this?
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
            return Elements.get()[PlaneWhole*Dimension*Dimension + RowWhole*Dimension + ColumnWhole];
        else if(RowRemainder < 0.0001f && ColumnRemainder < 0.0001f)
        {
            real32 Elem1 = Elements.get()[PlaneWhole*Dimension*Dimension + RowWhole*Dimension + ColumnWhole];
            real32 Elem2 = Elements.get()[(PlaneWhole+1)*Dimension*Dimension + RowWhole*Dimension + ColumnWhole];
        
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
    
    // NOTE: X == Plane
    //       Y == Row
    //       Z == Column
    // NOTE: Used to transform between the two systems some other way, but scratched it
    real32& GetXYZ(int32 X, int32 Y, int32 Z)
    {
        // return GetPRC(Y, Z, X); //old version
        return GetPRC(X, Y, Z);
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
    
    struct GridRow
    {
        float* Elem;
        uint32 Dimension;
        GridRow(float *Elem, uint32 Dimension) : Elem(Elem), Dimension(Dimension){}
        float& operator[](int32 Index)
        {
            if(Index < 0) Index = 0;
            else if(Index > Dimension-1) Index = Dimension-1;
            return *(Elem+Index);
        }
    };
    
    struct GridPlane
    {
        float* Plane;
        uint32 Dimension;
        GridPlane(float *Plane, uint32 Dimension) : Plane(Plane), Dimension(Dimension){}
        GridRow operator[](int32 RowIndex)
        {
            if(RowIndex < 0) RowIndex = 0;
            else if(RowIndex > Dimension-1) RowIndex = Dimension-1;
            float *Row = (Plane + RowIndex*Dimension);
            return GridRow(Row, Dimension);
        }
    };
    
    // NOTE: returning GridRow& would be faster, but its dangerous,
    //       because it gives back the reference of a local variable. 
    // TODO: could it be fixed?
    GridPlane operator[](int32 PlaneIndex)
    {
        if(PlaneIndex < 0) PlaneIndex = 0;
        else if(PlaneIndex > Dimension-1) PlaneIndex = Dimension-1;
        float *Elem = &(Elements.get()[PlaneIndex*Dimension*Dimension]);
        return GridPlane(Elem, Dimension);
    }
};




#define TEREPGEN_GRID_H
#endif