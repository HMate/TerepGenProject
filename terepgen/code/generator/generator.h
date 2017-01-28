#if !defined(TEREPGEN_GENERATOR_MODULE_H)
/*
    Terep generátor by Hidvégi Máté @2017

*/

#include "..\terepgen_types.h"
#include "..\terepgen_math.h"
#include "..\terepgen_grid.h"
#include "..\terepgen_vector.h"
#include "..\terepgen_random.h"
#include "..\platform.h"
#include "..\utils.h"

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

vertex Vertex(v3 Pos, v3 Norm, v4 Color);

#include "terrain.h"

bool32 WorldPosEquals(world_block_pos *A, world_block_pos *B);
inline world_block_pos WorldPosFromV3(v3 Pos, int32 Resolution);
inline v3 V3FromWorldPos(world_block_pos Pos);
block_node GetActualBlockNode(world_block_pos *Original, int32 X, int32 Y, int32 Z);
block_node ConvertRenderPosToBlockNode(v3 RenderPos, int32 Resolution);
v3 ConvertBlockNodeToRenderPos(block_node *Node);

struct world_density
{
    uint32 Seed;
    perlin_noise_array PerlinArray;
    
    int32 FixedResolution[RESOLUTION_COUNT];
    uint32 StoreResolutionCount;
    uint32 MaxResolutionToRender;
    
    real32 BlockSize;
    uint32 DeletedDensityBlockCount;
    uint32 DeletedDynamicBlockCount;
    uint32 DeletedRenderBlockCount;
    
    uint32 BlockMappedCount;
    uint32 DensityBlockCount;
    terrain_density_block DensityBlocks[DENSITY_BLOCK_COUNT];
    uint32 DynamicBlockCount;
    terrain_density_block DynamicBlocks[DENSITY_BLOCK_COUNT];
    uint32 PoligonisedBlockCount;
    terrain_render_block PoligonisedBlocks[RENDER_BLOCK_COUNT];
    
    // NOTE: This must be a power of two for now!
    block_hash DensityHash[BLOCK_HASH_SIZE];
    block_hash DynamicHash[BLOCK_HASH_SIZE];
    block_hash RenderHash[BLOCK_HASH_SIZE];
    // NOTE: Index in ResolutionMapping means the Resolution that the block should be rendered
    block_hash ResolutionMapping[BLOCK_HASH_SIZE];
    uint32 ZeroBlockCount;
    block_hash ZeroHash[ZERO_HASH_SIZE];
    
    block_pos_array RenderPositionStore[RESOLUTION_COUNT];
    density_block_pos_array DensityPositionStore[RESOLUTION_COUNT];
    
    uint32 RenderBlockCount;
    terrain_render_block *RenderBlocks[RENDER_BLOCK_COUNT];
    avarage_time AvgPoligoniseTime;
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

#define CubeVertexCount 36
struct cube
{
    vertex Vertices[CubeVertexCount];
};

struct generator_position
{
    world_block_pos Centers[RESOLUTION_COUNT];
};

generator_position CalculateTerrainGeneratorPositon(world_density*, v3);
void InitializeTerrain(world_density*);
void ClearFarawayBlocks(memory_arena*, world_density*, 
                        char* DynamicStoreName, uint32 SessionId, 
                        generator_position *);
void GenerateTerrainBlocks(memory_arena*, world_density*, game_input *Input,
                           char* DynamicStoreName, uint32 SessionId, generator_position *,
                           v3 WorldMousePos, v3 CameraOrigo, cube *, v3 CameraP, v3 CamDir);
void SaveCompressedBlockArrayToFile(memory_arena *Arena, char *FileName, uint32 SessionId,  
                                    compressed_block *BlockArray, uint32 ArraySize);

compressed_block *CompressBlock(memory_arena *Arena, terrain_density_block *Block);
int32 GetBlockMappedResolution(world_density *World, world_block_pos *BlockP);

#define TEREPGEN_GENERATOR_MODULE_H
#endif

