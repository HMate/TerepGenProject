#if !defined(TEREPGEN_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_types.h"
#include "terepgen_terrain.cpp"
#include "terepgen_renderer.cpp"


#define RENDER_BLOCK_COUNT 125

struct game_state 
{
    bool32 Initialized;
    terrain_render_block RenderBlocks[RENDER_BLOCK_COUNT];
    v3 BlockPositions[RENDER_BLOCK_COUNT];
    v3 CameraPos;
    uint32 Seed;
};

internal void UpdateGameState(game_state);
internal void RenderGame(terrain_renderer *TRenderer, game_state *GameState);

#define TEREPGEN_H
#endif