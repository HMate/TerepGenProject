#if !defined(TEREPGEN_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_types.h"
#include "terepgen_grid.h"
#include "terepgen_vector.h"
#include "terepgen_random.h"
#include "terepgen_dx_renderer.h"
#include "terepgen_terrain.h"



struct world_block_pos
{
    int32 BlockX;
    int32 BlockY;
    int32 BlockZ;
    
    int32 Resolution;
};

struct block_hash
{
    world_block_pos Key;
    int32 BlockIndex;
};

// NOTE: (4/3)n^3 + 2n^2 + (8/3)n + 1
#define POS_GRID_SIZE(n) ((uint32)(((4.0*(n)*(n)*(n)) + (n)*8.0 )/3.0 ) + (2*(n)*(n)) + 1)
#define RENDERED_BLOCK_RADIUS 7
#define ZERO_BLOCK_RADIUS 23
#define BLOCK_POS_COUNT POS_GRID_SIZE(RENDERED_BLOCK_RADIUS)
#define RENDER_BLOCK_COUNT 1500

#define BLOCK_HASH_SIZE 4096
#define ZERO_HASH_SIZE 32768

// TODO: how to implement different blockpos array sizes? do i need it?
struct block_pos_array
{
    uint32 Count;
    world_block_pos Pos[BLOCK_POS_COUNT];
};

struct game_state 
{
    bool32 Initialized;
    dx_resource *DXResources;
    uint32 Seed;
    v3 CameraPos;
    v3 CameraDir;
    perlin_noise_array PerlinArray;
    real32 BlockSize;
    uint32 BlockResolution;
    uint32 RenderMode;
    
    uint32 DeletedBlockCount;
    uint32 PoligonisedBlock1Count;
    uint32 PoligonisedBlock2Count;
    uint32 PoligonisedBlock4Count;
    terrain_render_block PoligonisedBlocks1[RENDER_BLOCK_COUNT];
    terrain_render_block PoligonisedBlocks2[RENDER_BLOCK_COUNT];
    terrain_render_block PoligonisedBlocks4[RENDER_BLOCK_COUNT];
    uint32 RenderBlockCount;
    terrain_render_block *RenderBlocks[RENDER_BLOCK_COUNT];
    
    // NOTE: This must be a power of two for now!
    block_hash BlockHash[BLOCK_HASH_SIZE];
    uint32 ZeroBlockCount;
    block_hash ZeroHash[ZERO_HASH_SIZE];
};

internal void UpdateGameState(game_state);
internal void RenderGame(game_state *GameState, camera *Camera);

#define TEREPGEN_H
#endif