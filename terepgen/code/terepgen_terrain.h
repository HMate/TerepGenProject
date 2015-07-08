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
    real32 X, Y, Z;
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
    float RandomFloat(uint32 Table, uint32 Row, uint32 Column)
    {
        uint32 ColumnSeed = _rotr(((Seed + Column) * 6529), Row);
        uint32 RowSeed = _rotr(((Seed + Row) * 2311), Table);
        uint32 TableSeed = _rotr(((Seed + Table) * 14281), Column);
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
    uint32 FinalVertexCount;
    bool32 DXReleased;
    
    void Initialize(dx_resource &DXResources, uint32 FinalVertexCount);
    ~terrainRenderer();
    void Release();
    
    void DrawWireframe(std::shared_ptr<vertex> Vertices);
    void DrawTriangles(std::shared_ptr<vertex> Vertices);
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
    uint32 FinalVertexCount;   
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
    color Color;
    
    uint32 FinalVertexCount;  
    std::shared_ptr<v3> VertexLocations;
    uint32 VertexLocationCount;
    std::shared_ptr<vertex> Vertices;
    
    uint32 LastSeed;
    real32 LastPersistence;
    terrain_render_mode LastRenderMode;
    
    void Initialize(uint32 Seed, real32 Persistence);
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