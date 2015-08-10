#if !defined(TEREPGEN_TERRAIN_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include <random>
#include <atomic>

#include "terepgen_dxresources.h"
#include "terepgen_types.h"
#include "terepgen_grid.h"
#include "terepgen_vector.h"

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

struct RandomGenerator
{
    uint32 Seed;
    std::ranlux48 Rng;
    std::uniform_real_distribution<> UniformRng;
    
    RandomGenerator(uint32 Seed)
    {
        UniformRng = std::uniform_real_distribution<>(-1.0f, 1.0f);
        this->Seed = Seed;
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
    
    // TODO: This is really expensive now
    real32 RandomFloat(real32 Plane, real32 Row, real32 Column)
    {
        
        int32 PlaneWhole = FloorInt32(Plane);
        real32 PlaneRemainder = Plane - (real32)PlaneWhole;
        int32 RowWhole = FloorInt32(Row);
        real32 RowRemainder = Row - (real32)RowWhole;
        int32 ColumnWhole = FloorInt32(Column);
        real32 ColumnRemainder = Column - (real32)ColumnWhole;
        
        real32 R000 = RandomFloat(PlaneWhole  , RowWhole  , ColumnWhole  );
        real32 R001 = RandomFloat(PlaneWhole  , RowWhole  , ColumnWhole+1);
        real32 R010 = RandomFloat(PlaneWhole  , RowWhole+1, ColumnWhole  );
        real32 R011 = RandomFloat(PlaneWhole  , RowWhole+1, ColumnWhole+1);
        real32 R100 = RandomFloat(PlaneWhole+1, RowWhole  , ColumnWhole  );
        real32 R101 = RandomFloat(PlaneWhole+1, RowWhole  , ColumnWhole+1);
        real32 R110 = RandomFloat(PlaneWhole+1, RowWhole+1, ColumnWhole  );
        real32 R111 = RandomFloat(PlaneWhole+1, RowWhole+1, ColumnWhole+1);
        
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

struct terrainRenderer
{
    dx_resource *DXResource;
    object_constants ObjectConstants;
    ID3D11Buffer *ObjectConstantBuffer;
    ID3D11Buffer *VertexBuffer;  
    ID3D11RasterizerState *RSWireFrame = nullptr; 
    ID3D11RasterizerState *RSDefault = nullptr;
    uint32 MaxVertexCount;
    bool32 DXReleased;
    
    terrainRenderer();
    terrainRenderer(const terrainRenderer&) = delete;
    
    HRESULT Initialize(dx_resource *DXResources, uint32 MaxVertexCount);
    ~terrainRenderer();
    void Release();
    
    void SetTransformations(v3 Translation);
    void DrawWireframe(vertex *Vertices, uint32 VertexCount);
    void DrawTriangles(vertex *Vertices, uint32 VertexCount);
    void DrawDebugTriangle();
    void DrawAxis(real32 Size = 1.0f);
};

enum class terrain_render_mode
{
    Triangles = 0,
    Wireframe = 1,
    Points = 2
};

struct terrain3D
{
    uint32 TerrainDimension;
    grid3D TerrainGrid;
    v3 GridPos;
    v3 RenderPos;
    
    uint32 MaxVertexCount;  
    uint32 CurrentVertexCount;
    vertex *Vertices;
    std::atomic<bool32> Loaded = false;
    
    uint32 LastSeed;
    real32 LastPersistence;
    terrain_render_mode LastRenderMode;
    
    //terrain3D(const terrain3D&) = delete;
    terrain3D& operator=(terrain3D& Other)
    {
        this->TerrainDimension   = Other.TerrainDimension;
        this->TerrainGrid        = Other.TerrainGrid;
        this->GridPos            = Other.GridPos;
        this->RenderPos          = Other.RenderPos;
        this->MaxVertexCount     = Other.MaxVertexCount;  
        this->CurrentVertexCount = Other.CurrentVertexCount;
        this->Vertices           = Other.Vertices;
        Other.Vertices = nullptr;
        bool32 Temp;
        this->Loaded             = Temp = Other.Loaded;
        this->LastSeed           = Other.LastSeed;
        this->LastPersistence    = Other.LastPersistence;
        this->LastRenderMode     = Other.LastRenderMode;
        return *this;
    }
    ~terrain3D();
    
    void Initialize(uint32 BlockDimension, uint32 Seed, real32 Persistence, v3 WorldPos, uint32 CubeSize);
    virtual void GenerateTerrain(uint32 Seed, real32 Persistence);
    vertex* CreateRenderVertices(uint32 CubeSize);
    vertex* CreateVerticesForPointRendering();
    
    void Update(uint32 Seed, real32 Persistence, terrain_render_mode RenderMode, uint32 CubeSize);
    void Draw(terrainRenderer *Renderer);
};

struct terrain_render_vertices
{
    v3 RenderPos;
    vertex Vertices[GRID_DIMENSION * GRID_DIMENSION * GRID_DIMENSION * 4];
};

#define TEREPGEN_TERRAIN_H
#endif