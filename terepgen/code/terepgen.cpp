/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen.h"

#define UNINITIALIZED_HASH UINT32_MAX
    
internal block_hash *
GetBlockHash(game_state *GameState, world_block_pos P)
{
    block_hash *Result = 0;

    uint32 HashValue = 31*P.BlockX + 13*P.BlockY + 5*P.BlockZ;
    uint32 HashMask = (ArrayCount(GameState->BlockHash) - 1);
    
    for(uint32 Offset = 0;
        Offset < ArrayCount(GameState->BlockHash);
        ++Offset)
    {
        uint32 HashIndex = (HashValue + Offset) & HashMask;
        Assert(HashIndex < ArrayCount(GameState->BlockHash));
        block_hash *Hash = GameState->BlockHash + HashIndex;
        
        if(Hash->BlockIndex == UNINITIALIZED_HASH || 
           ((P.BlockX == Hash->Key.BlockX) && 
            (P.BlockY == Hash->Key.BlockY) && 
            (P.BlockZ == Hash->Key.BlockZ)))
        {
            Result = Hash;
            break;
        }
    }
    
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
GetWorldPosFromV3(v3 Pos, real32 BlockSize)
{
    // TODO: Needs block resolution too?
    world_block_pos Result = {};
    
    v3 CentralBlockPos = Pos / BlockSize;
    Result.BlockX = FloorInt32(CentralBlockPos.X); 
    Result.BlockY = FloorInt32(CentralBlockPos.Y);
    Result.BlockZ = FloorInt32(CentralBlockPos.Z);
     
    return Result;
}

inline v3
GetV3FromWorldPos(world_block_pos Pos, real32 BlockSize)
{
    // TODO: Needs block resolution too?
    v3 Result = {};
    Result.X = (real32)Pos.BlockX * BlockSize;
    Result.Y = (real32)Pos.BlockY * BlockSize;
    Result.Z = (real32)Pos.BlockZ * BlockSize;
    
    return Result;
}

internal void
GenerateTerrain(game_state *GameState)
{
    real32 BlockSize = real32(TERRAIN_BLOCK_SIZE);
    uint32 BlockResolution = 8;
    
    world_block_pos CentralBlockPos = GetWorldPosFromV3(GameState->CameraPos, BlockSize);
    CalculateBlockPositions(GameState, CentralBlockPos);
                         
    terrain_density_block DensityBlock;
    RandomGenerator Rng(GameState->Seed);
    Rng.SetSeed(1000);
    
    uint32 RenderBlockIndex = 0;
    for(size_t BlockIndex = 0; 
        BlockIndex < ArrayCount(GameState->BlockPositions);
        ++BlockIndex)
    {
        DensityBlock.Pos = GetV3FromWorldPos(GameState->BlockPositions[BlockIndex], BlockSize) * (real32)BlockResolution;
        GenerateDensityGrid(&DensityBlock, &Rng, BlockResolution);
        CreateRenderVertices(&(GameState->RenderBlocks[GameState->RenderBlockCount]), &DensityBlock, BlockResolution);
        if(GameState->RenderBlocks[GameState->RenderBlockCount].VertexCount != 0)
        {
            block_hash *Hash = GetBlockHash(GameState, GameState->BlockPositions[BlockIndex]);
            Assert(Hash && Hash->BlockIndex == UNINITIALIZED_HASH);
            
            Hash->Key = GameState->BlockPositions[BlockIndex];
            Hash->BlockIndex = GameState->RenderBlockCount++;
            Assert(GameState->RenderBlockCount < ArrayCount(GameState->RenderBlocks));
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
        Hash->BlockIndex = UNINITIALIZED_HASH;
    }
}

internal void
UpdateGameState(game_state *GameState)
{
    if(GameState->Initialized == false)
    {
        GameState->RenderBlockCount = 0;
        InitHashTable(GameState);
        GenerateTerrain(GameState);
        GameState->Initialized = true;
    }

}

internal void
RenderGame(terrain_renderer *Renderer, game_state *GameState)
{
    for(size_t RenderBlockIndex = 0; 
        RenderBlockIndex < GameState->RenderBlockCount; 
        RenderBlockIndex++)
    {
        Renderer->SetTransformations(GameState->RenderBlocks[RenderBlockIndex].Pos);
        Renderer->DrawTriangles(
            GameState->RenderBlocks[RenderBlockIndex].Vertices,
            GameState->RenderBlocks[RenderBlockIndex].VertexCount);
        Renderer->SetTransformations(v3{});
    }
}
