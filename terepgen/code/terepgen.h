#if !defined(TEREPGEN_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_types.h"
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
    uint32 BlockIndex;
};

#define BLOCK_POS_COUNT 1000
#define RENDER_BLOCK_COUNT 500

struct game_state 
{
    bool32 Initialized;
    uint32 Seed;
    v3 CameraPos;
    RandomGenerator Rng;
    
    uint32 StoredRenderBlockCount;
    terrain_render_block StoredRenderBlocks[RENDER_BLOCK_COUNT];
    uint32 RenderBlockCount;
    terrain_render_block *RenderBlocks[RENDER_BLOCK_COUNT];
    world_block_pos BlockPositions[BLOCK_POS_COUNT];
    // NOTE: This must be a power of two for now!
    block_hash BlockHash[2048];
    block_hash ZeroHash[4096];
};

internal void UpdateGameState(game_state);
internal void RenderGame(terrain_renderer *TRenderer, game_state *GameState);

#define TEREPGEN_H
#endif