#if !defined(TEREPGEN_TERRAIN_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

struct world_block_pos
{
    int32 BlockX;
    int32 BlockY;
    int32 BlockZ;
        
    // TODO: Maybe resolution shouldnt be stored here
    int32 Resolution;
};

struct block_node
{
    world_block_pos BlockP;
    uint32 X;
    uint32 Y;
    uint32 Z;
};

struct terrain_density_block
{
    world_block_pos Pos;
    static_grid3D Grid;
};//2'064 B

struct compressed_node
{
    uint16 Count;
    real32 Value;
};

#pragma warning(disable : 4200)
struct compressed_block
{
    world_block_pos Pos;
    // NOTE compressed block is created in a way, that NodeCount number of 
    // compressed_node comes after it in memory.
    uint32 NodeCount;
    compressed_node Nodes[0];
};

#define HASH_UNINITIALIZED -1
#define HASH_ZERO_BLOCK -2
#define HASH_DELETED -3

struct block_hash
{
    world_block_pos Key;
    int32 Index;
};

const int32 NeighbourSameResCount = 27;
struct block_same_res_neighbours
{
    world_block_pos Pos[NeighbourSameResCount];
};

const int32 NeighbourLowerCount = 64;
struct block_lower_neighbours
{
    world_block_pos Pos[NeighbourLowerCount];
};

const int32 LowerBlockCount = 8;
struct lower_blocks
{
    world_block_pos Pos[LowerBlockCount];
};

#define TERRAIN_BLOCK_SIZE GRID_DIMENSION
#define RENDER_BLOCK_VERTEX_COUNT 7000

// NOTE: This is the size of a resolution 1 block in render space
#define RENDER_SPACE_UNIT 1.0f

struct terrain_render_block
{
    v3 Pos;
    world_block_pos WPos;
    uint32 VertexCount;
    vertex Vertices[RENDER_BLOCK_VERTEX_COUNT];
};//280'016 B

// NOTE: (4/3)n^3 + 2n^2 + (8/3)n + 1
#define POS_GRID_SIZE(n) ((uint32)(((4.0*(n)*(n)*(n)) + (n)*8.0 )/3.0 ) + (2*(n)*(n)) + 1)
#define RENDERED_BLOCK_RADIUS 8
#define DENSITY_BLOCK_RADIUS RENDERED_BLOCK_RADIUS+6
#define ZERO_BLOCK_RADIUS 23
#define BLOCK_POS_COUNT POS_GRID_SIZE(RENDERED_BLOCK_RADIUS)
#define DENSITY_POS_COUNT POS_GRID_SIZE(DENSITY_BLOCK_RADIUS)

// TODO: how to implement different blockpos array sizes? do i need it?
struct block_pos_array
{
    uint32 Count;
    world_block_pos Pos[BLOCK_POS_COUNT];
};
struct density_block_pos_array
{
    uint32 Count;
    world_block_pos Pos[DENSITY_POS_COUNT];
};

#define DENSITY_BLOCK_COUNT 8000
#define RENDER_BLOCK_COUNT 1500
#define BLOCK_HASH_SIZE 32768
#define ZERO_HASH_SIZE 65536

constexpr int32 RESOLUTION_COUNT = 3;

struct terrain
{
    uint32 Seed;
    perlin_noise_array PerlinArray;
    
    int32 FixedResolution[RESOLUTION_COUNT];
    uint32 StoreResolutionCount;
    uint32 MaxResolutionToRender;
    
    real32 BlockSize;
    uint32 DeletedDensityBlockCount;
    uint32 DeletedDynamicBlockCount;
    uint32 DeletedRenderBlockCount;
    
    uint32 BlockMappedCount;
    uint32 DensityBlockCount;
    terrain_density_block DensityBlocks[DENSITY_BLOCK_COUNT];
    uint32 DynamicBlockCount;
    terrain_density_block DynamicBlocks[DENSITY_BLOCK_COUNT];
    uint32 PoligonisedBlockCount;
    terrain_render_block PoligonisedBlocks[RENDER_BLOCK_COUNT];
    
    // NOTE: This must be a power of two for now!
    block_hash DensityHash[BLOCK_HASH_SIZE];
    block_hash DynamicHash[BLOCK_HASH_SIZE];
    block_hash RenderHash[BLOCK_HASH_SIZE];
    // NOTE: Index in ResolutionMapping means the Resolution that the block should be rendered
    block_hash ResolutionMapping[BLOCK_HASH_SIZE];
    uint32 ZeroBlockCount;
    block_hash ZeroHash[ZERO_HASH_SIZE];
    
    block_pos_array RenderPositionStore[RESOLUTION_COUNT];
    density_block_pos_array DensityPositionStore[RESOLUTION_COUNT];
    
    uint32 RenderBlockCount;
    terrain_render_block *RenderBlocks[RENDER_BLOCK_COUNT];
    avarage_time AvgPoligoniseTime;
};



#define TEREPGEN_TERRAIN_H
#endif