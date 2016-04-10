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
    uint32 Resolution;
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

#define HASH_UNINITIALIZED -1
#define HASH_ZERO_BLOCK -2
#define HASH_DELETED -3

struct block_hash
{
    world_block_pos Key;
    int32 Index;
};

struct block_neighbours
{
    world_block_pos Pos[27];
};

#define TERRAIN_BLOCK_SIZE GRID_DIMENSION
#define RENDER_BLOCK_VERTEX_COUNT 7000

#define RENDER_SPACE_UNIT 1.0f

struct terrain_render_block
{
    v3 Pos;
    uint32 Resolution;
    uint32 VertexCount;
    world_block_pos NeighbourPositions[27];
    vertex Vertices[RENDER_BLOCK_VERTEX_COUNT];
};//280'016 B

#define DENSITY_BLOCK_COUNT 6000
#define RENDER_BLOCK_COUNT 1500
#define BLOCK_HASH_SIZE 8192
#define ZERO_HASH_SIZE 32768

struct world_density
{
    real32 BlockSize;
    uint32 DeletedDensityBlockCount;
    uint32 DeletedRenderBlockCount;
    
    uint32 DensityBlockCount;
    terrain_density_block DensityBlocks[DENSITY_BLOCK_COUNT];
    uint32 PoligonisedBlockCount;
    terrain_render_block PoligonisedBlocks[RENDER_BLOCK_COUNT];
    
    // NOTE: This must be a power of two for now!
    block_hash DensityHash[BLOCK_HASH_SIZE];
    block_hash ResolutionMapping[BLOCK_HASH_SIZE];
    block_hash RenderHash[BLOCK_HASH_SIZE];
    uint32 ZeroBlockCount;
    block_hash ZeroHash[ZERO_HASH_SIZE];
};


#define TEREPGEN_TERRAIN_H
#endif