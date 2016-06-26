#if !defined(TEREPGEN_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_types.h"
#include "terepgen_math.cpp"
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
#include "terepgen_terrain.h"

#define CubeVertexCount 36
struct cube
{
    vertex Vertices[CubeVertexCount];
};
#define CubeFrameVertexCount 24
struct cube_frame
{
    vertex Vertices[CubeFrameVertexCount];
};

struct avarage_time
{
    real64 AvgTime = 0.0f;
    real64 MeasureCount = 0.0f;
};

typedef HANDLE FileHandle;

struct session_description
{
    uint32 ID;
    // NOTE: MAX_PATH is used for ASCII file names in windows
    char DynamicStore[MAX_PATH];
};

enum deformer_tpye
{
    DeformerTypeSphere = 1,
    DeformerTypeGradualSphere = 2,
    DeformerTypeCube = 3
};

struct block_deformer
{
    deformer_tpye Type;
    // NOTE: positive makes air, negative makes ground
    real32 Sign;
    real32 Radius;
    real32 Strength;
    v3 Center;
};

struct render_state
{
    bool32 Initialized;
    dx_resource DXResources;
    
    v3 CameraDir;
    // NOTE: Camera origo is from where the origin of the camera ray casts that hit the screen
    v3 CameraOrigo;
    camera Camera;
};

struct game_state 
{
    bool32 Initialized;
    session_description Session;
    uint32 Seed;
    
    real64 dtForFrame;
    
    perlin_noise_array PerlinArray;
    uint32 RenderMode;
    
    memory_arena WorldArena;
    world_density WorldDensity;
    
    render_state RenderState;
    
    uint32 RenderBlockCount;
    terrain_render_block *RenderBlocks[RENDER_BLOCK_COUNT];
    
    cube Cube;
    v3 CubePos;
    
    cube_frame DebugBlockFrames[DENSITY_BLOCK_COUNT];
    int32 DebugBlockFrameCount;
    
    avarage_time AvgPoligoniseTime;
    avarage_time FrameAvg;
};

struct transient_state
{
    bool32 Initialized;
    memory_arena TranArena;
};

internal void TerminateGame();
internal void UpdateAndRenderGame(game_memory*, game_input*, screen_info, bool32);
internal void SaveGameState(game_memory*);

#define TEREPGEN_H
#endif