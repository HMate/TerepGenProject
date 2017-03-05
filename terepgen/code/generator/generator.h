#if !defined(TEREPGEN_GENERATOR_MODULE_H)
/*
    Terep generátor by Hidvégi Máté @2017

*/

#include "..\terepgen_types.h"
#include "..\terepgen_grid.h"
#include "..\terepgen_random.h"
#include "..\platform.h"
#include "..\utils.h"

#include "terrain.h"

bool32 WorldPosEquals(world_block_pos *A, world_block_pos *B);
inline world_block_pos WorldPosFromV3(v3 Pos, int32 Resolution);
inline v3 V3FromWorldPos(world_block_pos Pos);
block_node GetActualBlockNode(world_block_pos *Original, int32 X, int32 Y, int32 Z);
block_node ConvertRenderPosToBlockNode(v3 RenderPos, int32 Resolution);
v3 ConvertBlockNodeToRenderPos(block_node *Node);

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

#define CubeVertexCount 36
struct cube
{
    vertex Vertices[CubeVertexCount];
};

struct generator_position
{
    world_block_pos Centers[RESOLUTION_COUNT];
};

#include "..\renderer\renderer.h"
generator_position CalculateTerrainGeneratorPositon(terrain *, v3);
void InitializeTerrain(terrain *);
void SaveTerrain(memory_arena *, terrain *, session_description *);
void ClearFarawayBlocks(memory_arena *, terrain *, session_description *, generator_position *);
void GenerateTerrainBlocks(memory_arena *, terrain *, game_input *Input,
                           session_description *, generator_position *,
                           v3 WorldMousePos, v3 CameraOrigo, cube *, v3 CameraP, v3 CamDir,
                           dx_resource *DXResources);
void SaveCompressedBlockArrayToFile(memory_arena *, session_description *,  
                                    compressed_block *BlockArray, uint32 ArraySize);

compressed_block *CompressBlock(memory_arena *Arena, terrain_density_block *Block);
int32 GetBlockMappedResolution(terrain *World, world_block_pos *BlockP);

#define TEREPGEN_GENERATOR_MODULE_H
#endif

