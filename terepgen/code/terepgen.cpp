/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen.h"

#define HASH_UNINITIALIZED UINT32_MAX
#define HASH_ZERO_BLOCK UINT32_MAX-1
    
internal block_hash *
GetBlockHash(game_state *GameState, world_block_pos P)
{
    block_hash *Result = 0;

    uint32 HashValue = 151*P.BlockX + 37*P.BlockY + 5*P.BlockZ;
    uint32 HashMask = (ArrayCount(GameState->BlockHash) - 1);
    
    for(uint32 Offset = 0;
        Offset < ArrayCount(GameState->BlockHash);
        ++Offset)
    {
        uint32 HashIndex = (HashValue + Offset) & HashMask;
        Assert(HashIndex < ArrayCount(GameState->BlockHash));
        block_hash *Hash = GameState->BlockHash + HashIndex;
        
        if(Hash->BlockIndex == HASH_UNINITIALIZED || 
           ((P.BlockX == Hash->Key.BlockX) && 
            (P.BlockY == Hash->Key.BlockY) && 
            (P.BlockZ == Hash->Key.BlockZ)))
        {
            Result = Hash;
            break;
        }
    }
    Assert(Result);
    
    return Result;
}
    
    
internal void 
CalculateBlockPositions(game_state *GameState, world_block_pos CentralBlockPos)
{    
    int32 CubeRoot = Cbrt(ArrayCount(GameState->BlockPositions));
    int32 IndexDelta = CubeRoot/2;
    int32 Start = -IndexDelta;
    int32 End = CubeRoot - IndexDelta;
    
    uint32 PosIndex = 0;
    for(int32 XIndex = Start; XIndex < End; ++XIndex)
    {
        for(int32 YIndex = Start; YIndex < End; ++YIndex)
        {
            for(int32 ZIndex = Start; ZIndex < End; ++ZIndex)
            {
                GameState->BlockPositions[PosIndex].BlockX = CentralBlockPos.BlockX + XIndex;
                GameState->BlockPositions[PosIndex].BlockY = CentralBlockPos.BlockY + YIndex;
                GameState->BlockPositions[PosIndex].BlockZ = CentralBlockPos.BlockZ + ZIndex;
                PosIndex++;
            }
        }
    }
    Assert(PosIndex == ArrayCount(GameState->BlockPositions));
}

inline world_block_pos
GetWorldPosFromV3(v3 Pos, real32 BlockSize, uint32 BlockResolution)
{
    // TODO: Needs block resolution too?
    world_block_pos Result = {};
    
    v3 CentralBlockPos = Pos / (BlockSize*BlockResolution);
    Result.BlockX = FloorInt32(CentralBlockPos.X); 
    Result.BlockY = FloorInt32(CentralBlockPos.Y);
    Result.BlockZ = FloorInt32(CentralBlockPos.Z);
     
    return Result;
}

inline v3
GetV3FromWorldPos(world_block_pos Pos, real32 BlockSize, uint32 BlockResolution)
{
    // TODO: Needs block resolution too?
    v3 Result = {};
    Result.X = (real32)Pos.BlockX * BlockSize * BlockResolution;
    Result.Y = (real32)Pos.BlockY * BlockSize * BlockResolution;
    Result.Z = (real32)Pos.BlockZ * BlockSize * BlockResolution;
    
    return Result;
}

internal void
GenerateTerrain(game_state *GameState)
{
    real32 BlockSize = real32(TERRAIN_BLOCK_SIZE);
    uint32 BlockResolution = 8;
    
    world_block_pos CentralBlockPos = GetWorldPosFromV3(GameState->CameraPos, BlockSize, BlockResolution);
    CalculateBlockPositions(GameState, CentralBlockPos);
    
    terrain_density_block DensityBlock;
    
    for(size_t PosIndex = 0; 
        PosIndex < ArrayCount(GameState->BlockPositions);
        ++PosIndex)
    {
        DensityBlock.Pos = GetV3FromWorldPos(GameState->BlockPositions[PosIndex], BlockSize, BlockResolution);
        GenerateDensityGrid(&DensityBlock, &GameState->Rng, BlockResolution);
        CreateRenderVertices(&(GameState->StoredRenderBlocks[GameState->StoredRenderBlockCount]), &DensityBlock, BlockResolution);
        
        block_hash *Hash = GetBlockHash(GameState, GameState->BlockPositions[PosIndex]);
        Assert(Hash && Hash->BlockIndex == HASH_UNINITIALIZED);
        
        Hash->Key = GameState->BlockPositions[PosIndex];
        if(GameState->StoredRenderBlocks[GameState->StoredRenderBlockCount].VertexCount != 0)
        {
            Hash->BlockIndex = GameState->StoredRenderBlockCount++;
            Assert(GameState->StoredRenderBlockCount < ArrayCount(GameState->StoredRenderBlocks));
        }
        else
        {
            Hash->BlockIndex = HASH_ZERO_BLOCK;
        }
    }
}

inline void
InitHashTable(game_state *GameState)
{
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(GameState->BlockHash);
        ++HashIndex)
    {
        block_hash *Hash = GameState->BlockHash + HashIndex;
        Hash->BlockIndex = HASH_UNINITIALIZED;
    }
}

internal void
UpdateGameState(game_state *GameState)
{
    if(GameState->Initialized == false)
    {
        GameState->StoredRenderBlockCount = 0;
        GameState->Rng.SetSeed(GameState->Seed);
        InitHashTable(GameState);
        GenerateTerrain(GameState);
        GameState->Initialized = true;
    }
    real32 BlockSize = real32(TERRAIN_BLOCK_SIZE);
    uint32 BlockResolution = 8;
    
    world_block_pos CentralBlockPos = GetWorldPosFromV3(GameState->CameraPos, BlockSize, BlockResolution);
    CalculateBlockPositions(GameState, CentralBlockPos);
    
    GameState->RenderBlockCount = 0;
    for(size_t PosIndex = 0; 
        PosIndex < ArrayCount(GameState->BlockPositions);
        ++PosIndex)
    {
        block_hash *Hash = GetBlockHash(GameState, GameState->BlockPositions[PosIndex]);
        if(Hash->BlockIndex == HASH_UNINITIALIZED)
        {
            // NOTE: Initialize block
            terrain_density_block DensityBlock;
            DensityBlock.Pos = GetV3FromWorldPos(GameState->BlockPositions[PosIndex], BlockSize, BlockResolution);
            GenerateDensityGrid(&DensityBlock, &GameState->Rng, BlockResolution);
            CreateRenderVertices(&(GameState->StoredRenderBlocks[GameState->StoredRenderBlockCount]), &DensityBlock, BlockResolution);
            if(GameState->StoredRenderBlocks[GameState->StoredRenderBlockCount].VertexCount != 0)
            {
                Hash->BlockIndex = GameState->StoredRenderBlockCount++;
                Assert(GameState->StoredRenderBlockCount < ArrayCount(GameState->StoredRenderBlocks));
            }
            else
            {
                Hash->BlockIndex = HASH_ZERO_BLOCK;
            }
            Hash->Key = GameState->BlockPositions[PosIndex];
        }
        if(Hash->BlockIndex != HASH_ZERO_BLOCK)
        {
            GameState->RenderBlocks[GameState->RenderBlockCount++] = GameState->StoredRenderBlocks + Hash->BlockIndex;
        }
    }
}

internal void
RenderGame(terrain_renderer *Renderer, game_state *GameState)
{
    for(size_t RenderBlockIndex = 0; 
        RenderBlockIndex < GameState->RenderBlockCount; 
        RenderBlockIndex++)
    {
        Renderer->SetTransformations(GameState->RenderBlocks[RenderBlockIndex]->Pos);
        Renderer->DrawTriangles(
            GameState->RenderBlocks[RenderBlockIndex]->Vertices,
            GameState->RenderBlocks[RenderBlockIndex]->VertexCount);
        Renderer->SetTransformations(v3{});
    }
}
