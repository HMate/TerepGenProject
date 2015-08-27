/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen.h"

#define HASH_UNINITIALIZED UINT32_MAX
#define HASH_ZERO_BLOCK UINT32_MAX-1
#define HASH_DELETED UINT32_MAX-2
    
internal block_hash *
GetBlockHashForRead(game_state *GameState, world_block_pos P)
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
    
internal block_hash *
GetBlockHashForWrite(game_state *GameState, world_block_pos P)
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
        
        if(Hash->BlockIndex == HASH_UNINITIALIZED || Hash->BlockIndex == HASH_DELETED ||
           ((P.BlockX == Hash->Key.BlockX) && 
            (P.BlockY == Hash->Key.BlockY) && 
            (P.BlockZ == Hash->Key.BlockZ)))
        {
            Result = Hash;
            break;
        }
    }
    Assert(Result);
    Assert(Result->BlockIndex == HASH_UNINITIALIZED || Result->BlockIndex == HASH_DELETED);
    
    return Result;
}

internal block_hash *
GetZeroHash(game_state *GameState, world_block_pos P)
{
    block_hash *Result = 0;

    uint32 HashValue = 151*P.BlockX + 37*P.BlockY + 5*P.BlockZ;
    uint32 HashMask = (ArrayCount(GameState->ZeroHash) - 1);
    
    for(uint32 Offset = 0;
        Offset < ArrayCount(GameState->ZeroHash);
        ++Offset)
    {
        uint32 HashIndex = (HashValue + Offset) & HashMask;
        Assert(HashIndex < ArrayCount(GameState->ZeroHash));
        block_hash *Hash = GameState->ZeroHash + HashIndex;
        
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
GetWorldPosFromV3(game_state *GameState, v3 Pos)
{
    world_block_pos Result = {};
    
    v3 CentralBlockPos = Pos / (GameState->BlockSize*GameState->BlockResolution);
    Result.BlockX = FloorInt32(CentralBlockPos.X); 
    Result.BlockY = FloorInt32(CentralBlockPos.Y);
    Result.BlockZ = FloorInt32(CentralBlockPos.Z);
     
    return Result;
}

inline v3
GetV3FromWorldPos(game_state *GameState, world_block_pos Pos)
{
    v3 Result = {};
    Result.X = (real32)Pos.BlockX * GameState->BlockSize * GameState->BlockResolution;
    Result.Y = (real32)Pos.BlockY * GameState->BlockSize * GameState->BlockResolution;
    Result.Z = (real32)Pos.BlockZ * GameState->BlockSize * GameState->BlockResolution;
    
    return Result;
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
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(GameState->ZeroHash);
        ++HashIndex)
    {
        block_hash *Hash = GameState->ZeroHash + HashIndex;
        Hash->BlockIndex = HASH_UNINITIALIZED;
    }
}

internal void
UpdateGameState(game_state *GameState)
{
    if(GameState->Initialized == false)
    {
        GameState->BlockSize = real32(TERRAIN_BLOCK_SIZE);
        GameState->BlockResolution = 16;
        GameState->StoredRenderBlockCount = 0;
        GameState->Rng.SetSeed(GameState->Seed);
        InitHashTable(GameState);
        GameState->Initialized = true;
    }
    
    
    world_block_pos CentralBlockPos = GetWorldPosFromV3(GameState, GameState->CameraPos);
    CalculateBlockPositions(GameState, CentralBlockPos);
    
    //
    // NOTE: Delete blocks that are too far from the camera
    //
    // TODO: Same with zero blocks!
    
    int32 LoadSpaceRadius = 7;
    for(uint32 StoreIndex = 0; 
        StoreIndex < GameState->StoredRenderBlockCount; 
        ++StoreIndex)
    {
        terrain_render_block *Block = GameState->StoredRenderBlocks + StoreIndex;
        world_block_pos BlockP = GetWorldPosFromV3(GameState, Block->Pos);
        if((CentralBlockPos.BlockX + LoadSpaceRadius < BlockP.BlockX) ||
           (CentralBlockPos.BlockX - LoadSpaceRadius > BlockP.BlockX) ||
           (CentralBlockPos.BlockY + LoadSpaceRadius < BlockP.BlockY) ||
           (CentralBlockPos.BlockY - LoadSpaceRadius > BlockP.BlockY) ||
           (CentralBlockPos.BlockZ + LoadSpaceRadius < BlockP.BlockZ) ||
           (CentralBlockPos.BlockZ - LoadSpaceRadius > BlockP.BlockZ))
        {
            terrain_render_block *Last = GameState->StoredRenderBlocks + (--GameState->StoredRenderBlockCount);
            world_block_pos LastP = GetWorldPosFromV3(GameState, Last->Pos);
            
            block_hash *RemovedHash = GetBlockHashForRead(GameState, BlockP);
            // NOTE: This is from stored blocks, so it cant be a zero block!
            Assert(RemovedHash->BlockIndex != HASH_UNINITIALIZED && RemovedHash->BlockIndex != HASH_DELETED);
            block_hash *LastHash = GetBlockHashForRead(GameState, LastP);
            Assert(LastHash->BlockIndex != HASH_UNINITIALIZED && LastHash->BlockIndex != HASH_DELETED);
            
            RemovedHash->BlockIndex = HASH_DELETED;
            LastHash->BlockIndex = StoreIndex;
            
            *Block = *Last;
        }
    }
    
    //
    // NOTE: Collect the render block we want to draw to the screen
    //
    
    terrain_density_block DensityBlock;
    uint32 MaxBlocksToRenderInFrame = 3;
    GameState->RenderBlockCount = 0;
    for(size_t PosIndex = 0; 
        PosIndex < ArrayCount(GameState->BlockPositions);
        ++PosIndex)
    {
        block_hash *ZeroHash = GetZeroHash(GameState, GameState->BlockPositions[PosIndex]);
        if(ZeroHash->BlockIndex == HASH_UNINITIALIZED)
        {
            block_hash *BlockHash = GetBlockHashForRead(GameState, GameState->BlockPositions[PosIndex]);
            // NOTE: This can give back a deleted hash, if it had the same key as this block,
            // and it was already deleted once, and wasn't overwritten since.
            if(BlockHash->BlockIndex == HASH_UNINITIALIZED ||
               BlockHash->BlockIndex == HASH_DELETED)
            {
                if(MaxBlocksToRenderInFrame)
                {
                    MaxBlocksToRenderInFrame--;
                    // NOTE: Initialize block
                    
                    DensityBlock.Pos = GetV3FromWorldPos(GameState, GameState->BlockPositions[PosIndex]);
                    GenerateDensityGrid(&DensityBlock, &GameState->Rng, GameState->BlockResolution);
                    CreateRenderVertices(&(GameState->StoredRenderBlocks[GameState->StoredRenderBlockCount]), 
                        &DensityBlock, GameState->BlockResolution);
                    if(GameState->StoredRenderBlocks[GameState->StoredRenderBlockCount].VertexCount != 0)
                    {
                        BlockHash = GetBlockHashForWrite(GameState, GameState->BlockPositions[PosIndex]);
                        BlockHash->Key = GameState->BlockPositions[PosIndex];
                        BlockHash->BlockIndex = GameState->StoredRenderBlockCount++;
                        Assert(GameState->StoredRenderBlockCount < ArrayCount(GameState->StoredRenderBlocks));
                        
                        GameState->RenderBlocks[GameState->RenderBlockCount++] = 
                            GameState->StoredRenderBlocks + BlockHash->BlockIndex;
                    }
                    else
                    {
                        ZeroHash->Key = GameState->BlockPositions[PosIndex];
                        ZeroHash->BlockIndex = HASH_ZERO_BLOCK;
                    }
                }
            }
            else
            {
                GameState->RenderBlocks[GameState->RenderBlockCount++] = 
                    GameState->StoredRenderBlocks + BlockHash->BlockIndex;
            }
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
