#if !defined(TEREPGEN_TERRAIN_H)
/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_marching_cubes.cpp"

struct terrain_density_block
{
    v3 Pos;
    static_grid3D Grid;
};

#define TERRAIN_BLOCK_SIZE GRID_DIMENSION-5
#define RENDER_BLOCK_VERTEX_COUNT 7000

struct terrain_render_block
{
    v3 Pos;
    uint32 VertexCount;
    vertex Vertices[RENDER_BLOCK_VERTEX_COUNT];
};


#define TEREPGEN_TERRAIN_H
#endif