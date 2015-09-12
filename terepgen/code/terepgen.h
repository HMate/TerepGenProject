#if !defined(TEREPGEN_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_types.h"
#include "terepgen_terrain.h"

#include "terepgen_random.cpp"
#include "terepgen_terrain.cpp"
#include "terepgen_renderer.cpp"


struct world_block_pos
{
    int32 BlockX;
    int32 BlockY;
    int32 BlockZ;
};

struct block_hash
{
    world_block_pos Key;
    int32 BlockIndex;
};

// NOTE: (4/3)n^3 + 2n^2 + (8/3)n + 1
#define POS_GRID_SIZE(n) (((4.0*(n)*(n)*(n)) + (n)*8.0 )/3.0 ) + (2*(n)*(n)) + 1
#define BLOCK_POS_COUNT (uint32)POS_GRID_SIZE(10)
#define RENDER_BLOCK_COUNT 1000

struct game_state 
{
    bool32 Initialized;
    uint32 Seed;
    v3 CameraPos;
    v3 CameraDir;
    perlin_noise_generator Rng;
    real32 BlockSize;
    uint32 BlockResolution;
    
    uint32 BlockPosCount;
    world_block_pos BlockPositions[BLOCK_POS_COUNT];
    
    uint32 DeletedBlockCount;
    uint32 StoredRenderBlockCount;
    terrain_render_block StoredRenderBlocks[RENDER_BLOCK_COUNT];
    uint32 RenderBlockCount;
    terrain_render_block *RenderBlocks[RENDER_BLOCK_COUNT];
    
    // NOTE: This must be a power of two for now!
    block_hash BlockHash[2048];
    uint32 ZeroBlockCount;
    block_hash ZeroHash[4096];
};

internal void UpdateGameState(game_state);
internal void RenderGame(terrain_renderer *TRenderer, game_state *GameState);

#define TEREPGEN_H
#endif