/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen.h"

#define HASH_UNINITIALIZED -1
#define HASH_ZERO_BLOCK -2
#define HASH_DELETED -3
    
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

    uint32 HashValue = 151*P.BlockX + 37*P.BlockY + 3*P.BlockZ;
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
CalculateBlockPositions(game_state *GameState, world_block_pos CentralBlockPos, int32 Radius)
{    
    GameState->BlockPosCount = 0;
    for(int32 Dist = 0;
        Dist <= Radius;
        Dist++)
    {
        int32 YSign = 1, YDiff = 1, YIndex = 0;
        while(Abs(YIndex) <= Dist)
        {
            int32 XSign = 1, XDiff = 1, XIndex = 0;
            while(Abs(XIndex) <= Dist-Abs(YIndex))
            {
                int32 ZIndex = Dist-Abs(XIndex)-Abs(YIndex);
                int32 ZSign = -1;
                int32 ZDiff = (ZIndex) ? 2*ZIndex : 1;
                while(Abs(ZIndex) <= Dist-Abs(XIndex)-Abs(YIndex))
                {
                    GameState->BlockPositions[GameState->BlockPosCount].BlockX = CentralBlockPos.BlockX + XIndex;
                    GameState->BlockPositions[GameState->BlockPosCount].BlockY = CentralBlockPos.BlockY + YIndex;
                    GameState->BlockPositions[GameState->BlockPosCount].BlockZ = CentralBlockPos.BlockZ + ZIndex;
                    GameState->BlockPosCount++;
                    
                    ZIndex += ZSign * (ZDiff++);
                    ZSign *= -1;
                }
                
                XIndex += XSign * (XDiff++);
                XSign *= -1;
            }
            YIndex += YSign * (YDiff++);
            YSign *= -1;
        }
    }
    
    Assert(GameState->BlockPosCount <= ArrayCount(GameState->BlockPositions));
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
InitBlockHash(game_state *GameState)
{
    // TODO: Does zeroing out stored block count belong here?
    GameState->StoredRenderBlockCount = 0;
    
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(GameState->BlockHash);
        ++HashIndex)
    {
        block_hash *Hash = GameState->BlockHash + HashIndex;
        Hash->BlockIndex = HASH_UNINITIALIZED;
    }
}

inline void
InitZeroHash(game_state *GameState)
{
    GameState->ZeroBlockCount = 0;
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
        GameState->Rng.SetSeed(GameState->Seed);
        InitBlockHash(GameState);
        InitZeroHash(GameState);
        GameState->Initialized = true;
    }
    
    
    world_block_pos CentralBlockPos = GetWorldPosFromV3(GameState, GameState->CameraPos);
    CalculateBlockPositions(GameState, CentralBlockPos, 10);
    
    //
    // NOTE: Delete blocks that are too far from the camera
    //
    // TODO: Maybe we need to reinitialize the block hash, if there are too many deleted blocks?
    
    int32 LoadSpaceRadius = 13;
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
            
            // NOTE: This is from stored blocks, so it cant be a zero block!
            block_hash *RemovedHash = GetBlockHashForRead(GameState, BlockP);
            Assert(RemovedHash->BlockIndex != HASH_UNINITIALIZED && RemovedHash->BlockIndex != HASH_DELETED);
            block_hash *LastHash = GetBlockHashForRead(GameState, LastP);
            Assert(LastHash->BlockIndex != HASH_UNINITIALIZED && LastHash->BlockIndex != HASH_DELETED);
            
            RemovedHash->BlockIndex = HASH_DELETED;
            LastHash->BlockIndex = StoreIndex;
            
            *Block = *Last;
        }
    }
    
    if(GameState->ZeroBlockCount > (4096*3/4))
    {
        int32 ZeroSpaceRadius = LoadSpaceRadius + 3;
        block_hash NewZeroHash[4096];
        uint32 NewZeroHashEntryCount = GameState->ZeroBlockCount;
        for(uint32 ZeroIndex = 0; 
            ZeroIndex < ArrayCount(GameState->ZeroHash); 
            ++ZeroIndex)
        {
            NewZeroHash[ZeroIndex] = GameState->ZeroHash[ZeroIndex];
        }
        
        InitZeroHash(GameState);
        
        for(uint32 ZeroIndex = 0; 
            ZeroIndex < ArrayCount(GameState->ZeroHash); 
            ++ZeroIndex)
        {
            block_hash *Entry = NewZeroHash + ZeroIndex;
            world_block_pos ZeroP = Entry->Key;
            if((Entry->BlockIndex == HASH_ZERO_BLOCK) &&
              ((CentralBlockPos.BlockX + ZeroSpaceRadius > ZeroP.BlockX) &&
               (CentralBlockPos.BlockX - ZeroSpaceRadius < ZeroP.BlockX) &&
               (CentralBlockPos.BlockY + ZeroSpaceRadius > ZeroP.BlockY) &&
               (CentralBlockPos.BlockY - ZeroSpaceRadius < ZeroP.BlockY) &&
               (CentralBlockPos.BlockZ + ZeroSpaceRadius > ZeroP.BlockZ) &&
               (CentralBlockPos.BlockZ - ZeroSpaceRadius < ZeroP.BlockZ)))
            {
                block_hash *ZeroHash = GetZeroHash(GameState, ZeroP);
                ZeroHash->Key = ZeroP;
                ZeroHash->BlockIndex = HASH_ZERO_BLOCK;
                GameState->ZeroBlockCount++;
            }
        }
    }
    
    //
    // NOTE: Collect the render block we want to draw to the screen
    //
    
    terrain_density_block DensityBlock;
    uint32 MaxBlocksToGenerateInFrame = 2;
    GameState->RenderBlockCount = 0;
    for(size_t PosIndex = 0; 
        (PosIndex < GameState->BlockPosCount);
        ++PosIndex)
    {
        block_hash *ZeroHash = GetZeroHash(GameState, GameState->BlockPositions[PosIndex]);
        if(ZeroHash->BlockIndex == HASH_UNINITIALIZED)
        {
            block_hash *BlockHash = GetBlockHashForRead(GameState, GameState->BlockPositions[PosIndex]);
            // NOTE: This can give back a deleted hash, if it had the same key as this block,
            // and it was already deleted once, and wasn't overwritten since.
            if((BlockHash->BlockIndex == HASH_UNINITIALIZED) ||
               (BlockHash->BlockIndex == HASH_DELETED))
            {
                if(MaxBlocksToGenerateInFrame)
                {
                    MaxBlocksToGenerateInFrame--;
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
                        GameState->ZeroBlockCount++;
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
