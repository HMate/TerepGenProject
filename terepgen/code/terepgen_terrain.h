#if !defined(TEREPGEN_TERRAIN_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include <random>

#include "terepgen_types.h"
#include "terepgen_dxresources.h"
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
    float RandomFloat(uint32 Row, uint32 Column)
    {
        uint32 ColumnSeed = _rotr(((Seed + Column) * 6529), Row);
        uint32 RowSeed = _rotr(((Seed + Row) * 2311), Column);
        Rng.seed(RowSeed + ColumnSeed);
        real32 Result = UniformRng(Rng);
        return Result;
    }
    
    // Gives a random float in the (-1, 1) range based on 3d coordinates
    float RandomFloat(int32 Plane, int32 Row, int32 Column)
    {
        uint32 ColumnSeed = _rotr(((Seed + Column) * 6529), Row);
        uint32 RowSeed = _rotr(((Seed + Row) * 2311), Plane);
        uint32 TableSeed = _rotr(((Seed + Plane) * 14281), Column);
        Rng.seed(RowSeed + ColumnSeed + TableSeed);
        real32 Result = UniformRng(Rng);
        return Result;
    }
};

struct terrainRenderer
{
    dx_resource DXResource;
    ID3D11Buffer *ObjectConstantBuffer;
    object_constants ObjectConstants;
    ID3D11Buffer *VertexBuffer; 
    uint32 MaxVertexCount;
    bool32 DXReleased;
    
    void Initialize(dx_resource &DXResources, uint32 MaxVertexCount);
    ~terrainRenderer();
    void Release();
    
    void SetTransformations(v3 Translation);
    void DrawWireframe(std::shared_ptr<vertex> Vertices, uint32 VertexCount);
    void DrawTriangles(std::shared_ptr<vertex> Vertices, uint32 VertexCount);
    void DrawDebugTriangle();
    void DrawAxis(real32 Size = 1.0f);
};

enum class terrain_render_mode
{
    Triangles = 0,
    Wireframe = 1,
    Points = 2
};

struct terrain
{
    uint32 TerrainDimension;
    grid2D TerrainGrid;
    color Color;
    uint32 LastSeed;
    real32 LastPersistence;
    uint32 MaxVertexCount;   
    std::shared_ptr<vertex> Vertices; 
    
    void Initialize(uint32 Seed, real32 Persistence);
    virtual void GenerateTerrain(uint32 Seed, real32 Persistence);
    std::shared_ptr<vertex> CreateRenderVertices();
    void Update(uint32 Seed, real32 Persistence);
};

struct terrain3D
{
    uint32 TerrainDimension;
    grid3D TerrainGrid;
    v3 GridPos;
    v3 RenderPos;
    color Color;
    
    uint32 MaxVertexCount;  
    uint32 CurrentVertexCount;
    // std::shared_ptr<v3> VertexLocations;
    // uint32 VertexLocationCount;
    std::shared_ptr<vertex> Vertices;
    
    uint32 LastSeed;
    real32 LastPersistence;
    terrain_render_mode LastRenderMode;
    
    void Initialize(uint32 Seed, real32 Persistence, v3 WorldPos);
    virtual void GenerateTerrain(uint32 Seed, real32 Persistence);
    std::shared_ptr<vertex> CreateRenderVertices();
    std::shared_ptr<vertex> CreateVerticesForPointRendering();
    std::shared_ptr<vertex> CreateVerticesForWireframeRendering();
    
    void Update(uint32 Seed, real32 Persistence, terrain_render_mode RenderMode);
    void Draw(terrainRenderer &Renderer);
};

struct RandomGeneratorVariables
{
    uint32 Seed;
    real32 Persistence;
    uint32 OctaveCount;
    uint32 OctaveIndex;
    real32 Weight;
    uint32 WaveLength;
    grid2D *PerlinGrid;
    grid2D *TerrainGrid;
    uint32 Row;
    uint32 Column;
};

struct InterpolatingFunctionVariables : RandomGeneratorVariables
{
    grid2D * StrechedGrid;
};

struct SmoothingFunctionVariables : InterpolatingFunctionVariables
{
    grid2D * SmoothGrid;
};

struct functional_terrain : terrain
{
    std::function<float(RandomGeneratorVariables&)> RandomGeneratorFunction;
    std::function<float(InterpolatingFunctionVariables&)> InterpolatingFunction;
    std::function<float(SmoothingFunctionVariables&)> SmoothingFunction;
    
    functional_terrain();
    
    void GenerateTerrain(uint32 Seed, real32 Persistence) override;
};

#define TEREPGEN_TERRAIN_H
#endif