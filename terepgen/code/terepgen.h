#if !defined(TEREPGEN_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_types.h"
#include "platform.h"
#include "terepgen_math.h"
#include "terepgen_grid.h"
#include "terepgen_vector.h"
#include "terepgen_random.h"

#include "generator\generator.h"

#include "terepgen_dx_renderer.h"

#define CubeFrameVertexCount 24
struct cube_frame
{
    vertex Vertices[CubeFrameVertexCount];
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
    bool32 Running;
    bool32 Initialized;
    session_description Session;
    
    real64 dtForFrame;
    
    uint32 RenderMode;
    
    // NOTE: WorldArena is a memory_arena, where dynamic stuff can be stored, 
    // that will stay from frame to frame
    memory_arena WorldArena;
    // NOTE: Terrain size is constant now, that's why its not in WordArena
    terrain Terrain;
    
    render_state RenderState;
    
    
    cube Cube;
    v3 CubePos;
    
    cube_frame DebugBlockFrames[DENSITY_BLOCK_COUNT];
    int32 DebugBlockFrameCount;
    
    avarage_time FrameAvg;
};

struct transient_state
{
    bool32 Initialized;
    memory_arena TranArena;
};

void UpdateAndRenderGame(game_memory*, game_input*, screen_info, bool32);
void SaveGameState(game_memory*);
void TerminateGame();

#define TEREPGEN_H
#endif