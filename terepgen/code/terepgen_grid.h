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
    std::shared_ptr<real32> Elements;
    
    grid3D()
    {
        this->Dimension = 0;
        Elements = nullptr;
    }
    
    grid3D(uint32 Dimension)
    {
        this->Dimension = Dimension;
        Elements = std::shared_ptr<real32>(new real32[Dimension * Dimension * Dimension]);
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
    
    // NOTE: X == Column
    //       Y == Plane
    //       Z == Row
    real32& GetXYZ(int32 X, int32 Y, int32 Z)
    {
        return GetPRC(Y, Z, X);
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