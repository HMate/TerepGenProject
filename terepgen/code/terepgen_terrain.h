#if !defined(TEREPGEN_TERRAIN_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_types.h"
#include "terepgen_grid.h"
#include "terepgen_vector.h"
#include "terepgen_dxresources.h"
#include "terepgen_marching_cubes.cpp"

struct vertex
{
    // NOTE: position is in left handed coordinate system
    // +X points right initially, -X points left
    // +Y is vertical axis and points up 
    // -Z points through screen to user initially, +Z points toward screen 
    real32 X, Y, Z, W;
    real32 NX, NY, NZ, NW;
    color Color;
};

#define RANDOM_TEX_SIZE 33

struct RandomGenerator
{
    uint32 Seed;
    dynamic_grid3D RandomTex;
    std::ranlux48 Rng;
    std::uniform_real_distribution<real32> UniformRng;
    
    RandomGenerator(uint32 Seed) : Seed(Seed),RandomTex(RANDOM_TEX_SIZE)
    {
        UniformRng = std::uniform_real_distribution<real32>(-1.0f, 1.0f);
        //this->Seed = Seed;
    }
    
    void SetSeed(uint32 NewSeed)
    {
        this->Seed = NewSeed;
        Rng.seed(NewSeed);
        uint32 TexSize = RandomTex.Dimension * RandomTex.Dimension * RandomTex.Dimension;
        for(uint32 Index = 0; Index < TexSize; Index++)
        {
            RandomTex.Elements[Index] = UniformRng(Rng);
        }
    }
    
    // Gives a random float in the (-1, 1) range based on 2d coordinates
    real32 RandomFloat(uint32 Row, uint32 Column)
    {
        uint32 ColumnSeed = _rotr(((Seed + Column) * 6529), Row);
        uint32 RowSeed = _rotr(((Seed + Row) * 2311), Column);
        Rng.seed(RowSeed + ColumnSeed);
        real32 Result = UniformRng(Rng);
        return Result;
    }
    
    // Gives a random float in the (-1, 1) range based on 3d coordinates
    real32 RandomFloat(uint32 Plane, uint32 Row, uint32 Column)
    {
        uint32 ColumnSeed = _rotr(((Seed * Column) * 10145111), Row);
        uint32 RowSeed = _rotr(((Seed * Row) * 4588277), Plane);
        uint32 TableSeed = _rotr(((Seed * Plane) * 14281), Column);
        uint32 PRCSeed = RowSeed + ColumnSeed + TableSeed;  
        Rng.seed(PRCSeed);
        real32 Result = UniformRng(Rng);
        return Result;
    }
    
    // Gives a random float in the (-1, 1) range based on 3d coordinates
    real32 RandomFloat(int32 Plane, int32 Row, int32 Column)
    {
        uint32 ColumnSeed = _rotr(((Seed * Column) * 10145111), Row);
        uint32 RowSeed = _rotr(((Seed * Row) * 4588277), Plane);
        uint32 TableSeed = _rotr(((Seed * Plane) * 14281), Column);
        uint32 PRCSeed = RowSeed + ColumnSeed + TableSeed;  
        Rng.seed(PRCSeed);
        real32 Result = UniformRng(Rng);
        return Result;
    }
        
    real32 RandomFloat(real32 Plane, real32 Row, real32 Column)
    {
        real32 PlaneMod = ModReal32(Plane, (real32)RANDOM_TEX_SIZE);
        uint32 PlaneWhole = FloorInt32(PlaneMod);
        uint32 PlaneWhole2 = (PlaneWhole+1) % RANDOM_TEX_SIZE;
        real32 PlaneRemainder = PlaneMod - (real32)PlaneWhole;
        
        real32 RowMod = ModReal32(Row, (real32)RANDOM_TEX_SIZE);
        uint32 RowWhole = FloorInt32(RowMod);
        uint32 RowWhole2 = (RowWhole+1) % RANDOM_TEX_SIZE;
        real32 RowRemainder = RowMod - (real32)RowWhole;
        
        real32 ColumnMod = ModReal32(Column, (real32)RANDOM_TEX_SIZE);
        uint32 ColumnWhole = FloorInt32(ColumnMod);
        uint32 ColumnWhole2 = (ColumnWhole+1) % RANDOM_TEX_SIZE;
        real32 ColumnRemainder = ColumnMod - (real32)ColumnWhole;
        
        real32 R000 = RandomTex.GetPRC(PlaneWhole , RowWhole , ColumnWhole );
        real32 R001 = RandomTex.GetPRC(PlaneWhole , RowWhole , ColumnWhole2);
        real32 R010 = RandomTex.GetPRC(PlaneWhole , RowWhole2, ColumnWhole );
        real32 R011 = RandomTex.GetPRC(PlaneWhole , RowWhole2, ColumnWhole2);
        real32 R100 = RandomTex.GetPRC(PlaneWhole2, RowWhole , ColumnWhole );
        real32 R101 = RandomTex.GetPRC(PlaneWhole2, RowWhole , ColumnWhole2);
        real32 R110 = RandomTex.GetPRC(PlaneWhole2, RowWhole2, ColumnWhole );
        real32 R111 = RandomTex.GetPRC(PlaneWhole2, RowWhole2, ColumnWhole2);
        
        real32 I00 = R000 + ColumnRemainder * (R001-R000);
        real32 I01 = R010 + ColumnRemainder * (R011-R010);
        real32 I10 = R100 + ColumnRemainder * (R101-R100);
        real32 I11 = R110 + ColumnRemainder * (R111-R110);
        
        real32 I0 = I00 + RowRemainder * (I01-I00);
        real32 I1 = I10 + RowRemainder * (I11-I10);
        
        real32 Result = I0 + PlaneRemainder * (I1-I0);
        return Result;
    }
    
    real32 RandomFloat(v3 WorldPos)
    {
        real32 Result = RandomFloat(WorldPos.X, WorldPos.Y, WorldPos.Z);
        return Result;
    }
};

struct terrain_density_block
{
    v3 Pos;
    static_grid3D Grid;
};

#define TERRAIN_BLOCK_SIZE GRID_DIMENSION-5
#define RENDER_BLOCK_VERTEX_COUNT 5000

struct terrain_render_block
{
    v3 Pos;
    uint32 VertexCount;
    vertex Vertices[RENDER_BLOCK_VERTEX_COUNT];
};


#define TEREPGEN_TERRAIN_H
#endif