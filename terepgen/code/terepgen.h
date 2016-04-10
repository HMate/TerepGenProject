#if !defined(TEREPGEN_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_types.h"
#include "terepgen_grid.h"
#include "terepgen_vector.h"
#include "terepgen_random.h"

struct vertex
{
    // NOTE: position is in left handed coordinate system
    // +X points right initially, -X points left
    // +Y is vertical axis and points up 
    // -Z points through screen to user initially, +Z points toward screen 
    real32 X, Y, Z;
    real32 NX, NY, NZ;
    v4 Color;
};

vertex Vertex(v3 Pos, v3 Norm, v4 Color)
{
    vertex Result;
    Result.X = Pos.X;
    Result.Y = Pos.Y;
    Result.Z = Pos.Z;
    Result.NX = Norm.X;
    Result.NY = Norm.Y;
    Result.NZ = Norm.Z;
    Result.Color = Color;
    
    return Result;
}

#include "terepgen_dx_renderer.h"
#include "terepgen_marching_cubes.cpp"
#include "terepgen_terrain.h"


struct cube
{
    v3 Pos;
    vertex Vertices[36];
};

// NOTE: (4/3)n^3 + 2n^2 + (8/3)n + 1
#define POS_GRID_SIZE(n) ((uint32)(((4.0*(n)*(n)*(n)) + (n)*8.0 )/3.0 ) + (2*(n)*(n)) + 1)
#define RENDERED_BLOCK_RADIUS 9
#define ZERO_BLOCK_RADIUS 23
#define BLOCK_POS_COUNT POS_GRID_SIZE(RENDERED_BLOCK_RADIUS)

// TODO: how to implement different blockpos array sizes? do i need it?
struct block_pos_array
{
    uint32 Count;
    world_block_pos Pos[BLOCK_POS_COUNT];
};

struct avarage_time
{
    real64 AvgTime = 0.0f;
    real64 MeasureCount = 0.0f;
};

struct game_state 
{
    bool32 Initialized;
    dx_resource *DXResources;
    uint32 Seed;
    
    real64 dtForFrame;
    
    v3 CameraDir;
    // NOTE: Camera origo is from where the origin of the camera ray casts that hit the screen
    v3 CameraOrigo;
    
    perlin_noise_array PerlinArray;
    uint32 RenderMode;
    
    world_density WorldDensity;
    
    uint32 RenderBlockCount;
    terrain_render_block *RenderBlocks[RENDER_BLOCK_COUNT];
    
    cube Cube;
    
    avarage_time AvgPoligoniseTime;
    avarage_time FrameAvg;
};

internal void UpdateAndRenderGame(game_state*, game_input*, camera*, screen_info);

#define TEREPGEN_H
#endif