/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_random.cpp"
#include "terepgen_terrain.cpp"
#include "terepgen_dx_renderer.cpp"

#define HASH_UNINITIALIZED -1
#define HASH_ZERO_BLOCK -2
#define HASH_DELETED -3
    
internal block_hash *
GetBlockHash(game_state *GameState, world_block_pos P)
{
    block_hash *Result = 0;

    uint32 HashValue = 2557*P.Resolution + 151*P.BlockX + 37*P.BlockY + 5*P.BlockZ;
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
            (P.BlockZ == Hash->Key.BlockZ) && 
            (P.Resolution == Hash->Key.Resolution)))
        {
            Result = Hash;
            break;
        }
    }
    Assert(Result);
    
    return Result;
}
    
internal block_hash *
WriteBlockHash(game_state *GameState, world_block_pos P, int32 NewBlockIndex)
{
    block_hash *Result = 0;

    uint32 HashValue =  2557*P.Resolution + 151*P.BlockX + 37*P.BlockY + 5*P.BlockZ;
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
            (P.BlockZ == Hash->Key.BlockZ) && 
            (P.Resolution == Hash->Key.Resolution)))
        {
            Result = Hash;
            break;
        }
    }
    Assert(Result);
    Assert(Result->BlockIndex == HASH_UNINITIALIZED || Result->BlockIndex == HASH_DELETED);
        
    Result->Key = P;
    Result->BlockIndex = NewBlockIndex;
    
    return Result;
}

internal block_hash *
GetZeroHash(game_state *GameState, world_block_pos P)
{
    block_hash *Result = 0;

    uint32 HashValue = 2579*P.Resolution + 757*P.BlockX + 89*P.BlockY + 5*P.BlockZ;
    uint32 HashMask = (ArrayCount(GameState->ZeroHash) - 1);
    
    for(uint32 Offset = 0;
        Offset < ArrayCount(GameState->ZeroHash);
        ++Offset)
    {
        uint32 HashIndex = (HashValue + Offset) & HashMask;
        Assert(HashIndex < ArrayCount(GameState->ZeroHash));
        block_hash *Hash = GameState->ZeroHash + HashIndex;
        
        // NOTE: return hash, if its uninited, or it has the position we are looking for
        if(Hash->BlockIndex == HASH_UNINITIALIZED || 
           ((P.BlockX == Hash->Key.BlockX) && 
            (P.BlockY == Hash->Key.BlockY) && 
            (P.BlockZ == Hash->Key.BlockZ) && 
            (P.Resolution == Hash->Key.Resolution)))
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
                    GameState->BlockPositions[GameState->BlockPosCount].Resolution = 4;
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
GetWorldPosFromV3(game_state *GameState, v3 Pos, int32 Resolution)
{
    world_block_pos Result = {};
    
    v3 CentralBlockPos = Pos / (GameState->BlockSize*Resolution);
    Result.BlockX = FloorInt32(CentralBlockPos.X); 
    Result.BlockY = FloorInt32(CentralBlockPos.Y);
    Result.BlockZ = FloorInt32(CentralBlockPos.Z);
    Result.Resolution = Resolution;
     
    return Result;
}

inline v3
GetV3FromWorldPos(game_state *GameState, world_block_pos Pos)
{
    v3 Result = {};
    Result.X = (real32)Pos.BlockX * GameState->BlockSize * Pos.Resolution;
    Result.Y = (real32)Pos.BlockY * GameState->BlockSize * Pos.Resolution;
    Result.Z = (real32)Pos.BlockZ * GameState->BlockSize * Pos.Resolution;
    
    return Result;
}

inline void
InitBlockHash(game_state *GameState)
{
    // TODO: Does zeroing out stored block count belong here?
    GameState->PoligonisedBlock1Count = 0;
    GameState->PoligonisedBlock2Count = 0;
    GameState->PoligonisedBlock4Count = 0;
    GameState->DeletedBlockCount = 0;
    
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
PrintDebug(world_block_pos BlockP, terrain_density_block DensityBlock)
{
#if TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG_GAME] Block Pos X: %d, Y: %d, Z: %d\n",
        BlockP.BlockX, BlockP.BlockY, BlockP.BlockZ);
    OutputDebugStringA(DebugBuffer);
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG_GAME] World Pos X: %f, Y: %f, Z: %f\n",
        DensityBlock.Pos.X, DensityBlock.Pos.Y, DensityBlock.Pos.Z);
    OutputDebugStringA(DebugBuffer);
#endif
}

internal world_block_pos
ConvertToResolution(world_block_pos P, int32 NewRes)
{
    world_block_pos Result;
    Result.Resolution = NewRes;
    
    real32 Ratio = (real32)P.Resolution / NewRes;
    Result.BlockX = (int32)(P.BlockX * Ratio);
    Result.BlockY = (int32)(P.BlockY * Ratio);
    Result.BlockZ = (int32)(P.BlockZ * Ratio);
    
    return Result;
}

internal void
GetLowerResBlockPositions(world_block_pos *LowerBlockPositions, world_block_pos *BlockP)
{
    LowerBlockPositions[0].Resolution = BlockP->Resolution/2;
    LowerBlockPositions[0].BlockX = BlockP->BlockX * 2;
    LowerBlockPositions[0].BlockY = BlockP->BlockY * 2;
    LowerBlockPositions[0].BlockZ = BlockP->BlockZ * 2;
    
    LowerBlockPositions[1].Resolution = BlockP->Resolution/2;
    LowerBlockPositions[1].BlockX = BlockP->BlockX * 2 + 1;
    LowerBlockPositions[1].BlockY = BlockP->BlockY * 2;
    LowerBlockPositions[1].BlockZ = BlockP->BlockZ * 2;
    
    LowerBlockPositions[2].Resolution = BlockP->Resolution/2;
    LowerBlockPositions[2].BlockX = BlockP->BlockX * 2;
    LowerBlockPositions[2].BlockY = BlockP->BlockY * 2 + 1;
    LowerBlockPositions[2].BlockZ = BlockP->BlockZ * 2;
    
    LowerBlockPositions[3].Resolution = BlockP->Resolution/2;
    LowerBlockPositions[3].BlockX = BlockP->BlockX * 2 + 1;
    LowerBlockPositions[3].BlockY = BlockP->BlockY * 2 + 1;
    LowerBlockPositions[3].BlockZ = BlockP->BlockZ * 2;
    
    LowerBlockPositions[4].Resolution = BlockP->Resolution/2;
    LowerBlockPositions[4].BlockX = BlockP->BlockX * 2;
    LowerBlockPositions[4].BlockY = BlockP->BlockY * 2;
    LowerBlockPositions[4].BlockZ = BlockP->BlockZ * 2 + 1;
    
    LowerBlockPositions[5].Resolution = BlockP->Resolution/2;
    LowerBlockPositions[5].BlockX = BlockP->BlockX * 2 + 1;
    LowerBlockPositions[5].BlockY = BlockP->BlockY * 2;
    LowerBlockPositions[5].BlockZ = BlockP->BlockZ * 2 + 1;
    
    LowerBlockPositions[6].Resolution = BlockP->Resolution/2;
    LowerBlockPositions[6].BlockX = BlockP->BlockX * 2;
    LowerBlockPositions[6].BlockY = BlockP->BlockY * 2 + 1;
    LowerBlockPositions[6].BlockZ = BlockP->BlockZ * 2 + 1;
    
    LowerBlockPositions[7].Resolution = BlockP->Resolution/2;
    LowerBlockPositions[7].BlockX = BlockP->BlockX * 2 + 1;
    LowerBlockPositions[7].BlockY = BlockP->BlockY * 2 + 1;
    LowerBlockPositions[7].BlockZ = BlockP->BlockZ * 2 + 1;
}

internal void
UpdateGameState(game_state *GameState)
{
    if(GameState->Initialized == false)
    {
        GameState->BlockSize = real32(TERRAIN_BLOCK_SIZE);
        GameState->BlockResolution = 4;
        SetSeed(&GameState->PerlinArray.Noise[0], GameState->Seed);
        SetSeed(&GameState->PerlinArray.Noise[1], GameState->Seed+1);
        SetSeed(&GameState->PerlinArray.Noise[2], GameState->Seed+2);
        InitBlockHash(GameState);
        InitZeroHash(GameState);
        
        GameState->Initialized = true;
    }
    
    v3 CameraP = GameState->CameraPos;
    world_block_pos WorldCameraP = GetWorldPosFromV3(GameState, CameraP, 4);
    CalculateBlockPositions(GameState, WorldCameraP, RENDERED_BLOCK_RADIUS);
    
    //
    // NOTE: Delete blocks that are too far from the camera
    //
    // TODO: Maybe we need to reinitialize the block hash, if there are too many deleted blocks?
    // TODO: Delete from other resolutions too!
    int32 LoadSpaceRadius = RENDERED_BLOCK_RADIUS;
    for(uint32 StoreIndex = 0; 
        StoreIndex < GameState->PoligonisedBlock4Count; 
        ++StoreIndex)
    {
        terrain_render_block *Block = GameState->PoligonisedBlocks4 + StoreIndex;
        world_block_pos BlockP = GetWorldPosFromV3(GameState, Block->Pos, 4);
        if((WorldCameraP.BlockX + LoadSpaceRadius < BlockP.BlockX) ||
           (WorldCameraP.BlockX - LoadSpaceRadius > BlockP.BlockX) ||
           (WorldCameraP.BlockY + LoadSpaceRadius < BlockP.BlockY) ||
           (WorldCameraP.BlockY - LoadSpaceRadius > BlockP.BlockY) ||
           (WorldCameraP.BlockZ + LoadSpaceRadius < BlockP.BlockZ) ||
           (WorldCameraP.BlockZ - LoadSpaceRadius > BlockP.BlockZ))
        {
            terrain_render_block *Last = GameState->PoligonisedBlocks4 + (--GameState->PoligonisedBlock4Count);
            world_block_pos LastP = GetWorldPosFromV3(GameState, Last->Pos, 4);
            
            // NOTE: This is from stored blocks, so it cant be a zero block!
            block_hash *RemovedHash = GetBlockHash(GameState, BlockP);
            Assert(RemovedHash->BlockIndex != HASH_UNINITIALIZED && RemovedHash->BlockIndex != HASH_DELETED);
            block_hash *LastHash = GetBlockHash(GameState, LastP);
            Assert(LastHash->BlockIndex != HASH_UNINITIALIZED && LastHash->BlockIndex != HASH_DELETED);
            
            RemovedHash->BlockIndex = HASH_DELETED;
            LastHash->BlockIndex = StoreIndex;
            GameState->DeletedBlockCount++;
            
            *Block = *Last;
        }
    }
    
    int32 ZeroGridTotalSize = POS_GRID_SIZE(ZERO_BLOCK_RADIUS);
    Assert(ZeroGridTotalSize < ZERO_HASH_SIZE);
    if(GameState->ZeroBlockCount > (ArrayCount(GameState->ZeroHash)*7/8))
    {
        int32 ZeroSpaceRadius = ZERO_BLOCK_RADIUS;
        block_hash NewZeroHash[ZERO_HASH_SIZE];
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
              ((WorldCameraP.BlockX + ZeroSpaceRadius > ZeroP.BlockX) &&
               (WorldCameraP.BlockX - ZeroSpaceRadius < ZeroP.BlockX) &&
               (WorldCameraP.BlockY + ZeroSpaceRadius > ZeroP.BlockY) &&
               (WorldCameraP.BlockY - ZeroSpaceRadius < ZeroP.BlockY) &&
               (WorldCameraP.BlockZ + ZeroSpaceRadius > ZeroP.BlockZ) &&
               (WorldCameraP.BlockZ - ZeroSpaceRadius < ZeroP.BlockZ)))
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
    uint32 MaxBlocksToGenerateInFrame = 3;
    for(size_t PosIndex = 0; 
        (PosIndex < GameState->BlockPosCount) && (MaxBlocksToGenerateInFrame > 0) ;
        ++PosIndex)
    {
        world_block_pos BlockP = ConvertToResolution(GameState->BlockPositions[PosIndex], 4);
        block_hash *ZeroHash = GetZeroHash(GameState, BlockP);
        if(ZeroHash->BlockIndex == HASH_UNINITIALIZED)
        {
            block_hash *BlockHash = GetBlockHash(GameState, BlockP);
            // NOTE: This can give back a deleted hash, if it had the same key as this block,
            // and it was already deleted once, and wasn't overwritten since.
            if((BlockHash->BlockIndex == HASH_UNINITIALIZED) ||
               (BlockHash->BlockIndex == HASH_DELETED))
            {
                if(MaxBlocksToGenerateInFrame > 0)
                {
                    MaxBlocksToGenerateInFrame--;
                    // NOTE: Initialize block
                    
                    DensityBlock.Pos = GetV3FromWorldPos(GameState, BlockP);
                    //win32_clock Clock;
                    GenerateDensityGrid(&DensityBlock, &GameState->PerlinArray, /*GameState->BlockResolution*/4);
                    //Clock.PrintMiliSeconds("Density time:");
                    //Clock.Reset();
                    PoligoniseBlock(&(GameState->PoligonisedBlocks4[GameState->PoligonisedBlock4Count]), 
                        &DensityBlock, /*GameState->BlockResolution*/4);
                    if(GameState->PoligonisedBlocks4[GameState->PoligonisedBlock4Count].VertexCount != 0)
                    {
                        //Clock.PrintMiliSeconds("Poligonise time:");
                        BlockHash = WriteBlockHash(GameState, BlockP, GameState->PoligonisedBlock4Count++);
                        Assert(GameState->PoligonisedBlock4Count < ArrayCount(GameState->PoligonisedBlocks4));
                    }
                    else
                    {
                        ZeroHash->Key = BlockP;
                        ZeroHash->BlockIndex = HASH_ZERO_BLOCK;
                        GameState->ZeroBlockCount++;
                    }
                }
            }
        }
    }
    
    for(size_t PosIndex = 0; 
        (PosIndex < GameState->BlockPosCount) && (MaxBlocksToGenerateInFrame > 0) ;
        ++PosIndex)
    {
        world_block_pos BlockP = ConvertToResolution(GameState->BlockPositions[PosIndex], 2);
        block_hash *ZeroHash = GetZeroHash(GameState, BlockP);
        if(ZeroHash->BlockIndex == HASH_UNINITIALIZED)
        {
            block_hash *BlockHash = GetBlockHash(GameState, BlockP);
            // NOTE: This can give back a deleted hash, if it had the same key as this block,
            // and it was already deleted once, and wasn't overwritten since.
            if((BlockHash->BlockIndex == HASH_UNINITIALIZED) ||
               (BlockHash->BlockIndex == HASH_DELETED))
            {
                if(MaxBlocksToGenerateInFrame > 0)
                {
                    MaxBlocksToGenerateInFrame--;
                    // NOTE: Initialize block
                    
                    DensityBlock.Pos = GetV3FromWorldPos(GameState, BlockP);
                    GenerateDensityGrid(&DensityBlock, &GameState->PerlinArray, 2);
                    PoligoniseBlock(&(GameState->PoligonisedBlocks2[GameState->PoligonisedBlock2Count]), 
                        &DensityBlock, 2);
                    if(GameState->PoligonisedBlocks2[GameState->PoligonisedBlock2Count].VertexCount != 0)
                    {
                        BlockHash = WriteBlockHash(GameState, BlockP, GameState->PoligonisedBlock2Count++);
                        Assert(GameState->PoligonisedBlock2Count < ArrayCount(GameState->PoligonisedBlocks2));
                    }
                    else
                    {
                        ZeroHash->Key = BlockP;
                        ZeroHash->BlockIndex = HASH_ZERO_BLOCK;
                        GameState->ZeroBlockCount++;
                    }
                }
            }
        }
    }
    
    const v3 CamDir = GameState->CameraDir;
    const v3 Diff4X = GetV3FromWorldPos(GameState, world_block_pos{1,0,0,4});
    const v3 Diff4Y = GetV3FromWorldPos(GameState, world_block_pos{0,1,0,4});
    const v3 Diff4Z = GetV3FromWorldPos(GameState, world_block_pos{0,0,1,4});
    const v3 Diff2X = GetV3FromWorldPos(GameState, world_block_pos{1,0,0,2});
    const v3 Diff2Y = GetV3FromWorldPos(GameState, world_block_pos{0,1,0,2});
    const v3 Diff2Z = GetV3FromWorldPos(GameState, world_block_pos{0,0,1,2});
    
    GameState->RenderBlockCount = 0;
    for(size_t PosIndex = 0; 
        (PosIndex < GameState->BlockPosCount);
        ++PosIndex)
    {
        world_block_pos BlockP = ConvertToResolution(GameState->BlockPositions[PosIndex], 4);
        block_hash *ZeroHash = GetZeroHash(GameState, BlockP);
        if(ZeroHash->BlockIndex == HASH_UNINITIALIZED)
        {
            block_hash *BlockHash = GetBlockHash(GameState, BlockP);
            
            // NOTE: Check if lower resolution blocks are available
            bool32 LowersBlocksLoaded = true;
            world_block_pos LowerBlockPositions[8];
            GetLowerResBlockPositions(LowerBlockPositions, &BlockP);
            
            for(int32 LowerPosIndex=0;
                LowerPosIndex < 8;
                ++LowerPosIndex)
            {
                block_hash *LowZeroHash = GetZeroHash(GameState, LowerBlockPositions[LowerPosIndex]);
                block_hash *LowBlockHash = GetBlockHash(GameState, LowerBlockPositions[LowerPosIndex]);
                if((LowZeroHash->BlockIndex != HASH_ZERO_BLOCK) &&
                    ((LowBlockHash->BlockIndex == HASH_UNINITIALIZED) ||
                     (LowBlockHash->BlockIndex == HASH_DELETED)))
                {
                    LowersBlocksLoaded = false;
                }
            }
            
            if(LowersBlocksLoaded)
            {
                for(int32 LowerPosIndex=0;
                    LowerPosIndex < 8;
                    ++LowerPosIndex)
                {
                    block_hash *LowBlockHash = GetBlockHash(GameState, LowerBlockPositions[LowerPosIndex]);
                    if((LowBlockHash->BlockIndex != HASH_UNINITIALIZED) &&
                        (LowBlockHash->BlockIndex != HASH_DELETED))
                    {
                        terrain_render_block *Block = GameState->PoligonisedBlocks2 + LowBlockHash->BlockIndex;
                        v3 P = Block->Pos - CameraP;
                        
                        v3 P0 = P;
                        v3 P1 = P + Diff2X;
                        v3 P2 = P + Diff2Y;
                        v3 P3 = P + Diff2Z;
                        v3 P4 = P + Diff2X + Diff2Y;
                        v3 P5 = P + Diff2Y + Diff2Z;
                        v3 P6 = P + Diff2X + Diff2Z;
                        v3 P7 = P + Diff2X + Diff2Y + Diff2Z;
                        if((DotProduct(P0, CamDir) > 0.0f) || (DotProduct(P1, CamDir) > 0.0f) ||
                           (DotProduct(P2, CamDir) > 0.0f) || (DotProduct(P3, CamDir) > 0.0f) ||
                           (DotProduct(P4, CamDir) > 0.0f) || (DotProduct(P5, CamDir) > 0.0f) ||
                           (DotProduct(P6, CamDir) > 0.0f) || (DotProduct(P7, CamDir) > 0.0f))
                        {
                            GameState->RenderBlocks[GameState->RenderBlockCount++] = Block;
                            Assert(GameState->RenderBlockCount < ArrayCount(GameState->RenderBlocks));
                        }
                    }
                }
            }
            else if((BlockHash->BlockIndex != HASH_UNINITIALIZED) &&
                    (BlockHash->BlockIndex != HASH_DELETED))
            {
                terrain_render_block *Block = GameState->PoligonisedBlocks4 + BlockHash->BlockIndex;
                v3 P = Block->Pos - CameraP;
                
                v3 P0 = P;
                v3 P1 = P + Diff4X;
                v3 P2 = P + Diff4Y;
                v3 P3 = P + Diff4Z;
                v3 P4 = P + Diff4X + Diff4Y;
                v3 P5 = P + Diff4Y + Diff4Z;
                v3 P6 = P + Diff4X + Diff4Z;
                v3 P7 = P + Diff4X + Diff4Y + Diff4Z;
                if((DotProduct(P0, CamDir) > 0.0f) || (DotProduct(P1, CamDir) > 0.0f) ||
                   (DotProduct(P2, CamDir) > 0.0f) || (DotProduct(P3, CamDir) > 0.0f) ||
                   (DotProduct(P4, CamDir) > 0.0f) || (DotProduct(P5, CamDir) > 0.0f) ||
                   (DotProduct(P6, CamDir) > 0.0f) || (DotProduct(P7, CamDir) > 0.0f))
                {
                    GameState->RenderBlocks[GameState->RenderBlockCount++] = Block;
                    Assert(GameState->RenderBlockCount < ArrayCount(GameState->RenderBlocks));
                }   
            }
        }
    }
    /*
    for(size_t PosIndex = 0; 
        (PosIndex < GameState->BlockPosCount);
        ++PosIndex)
    {
        world_block_pos BlockP = GameState->BlockPositions[PosIndex];
        BlockP.Resolution = 2;
        block_hash *ZeroHash = GetZeroHash(GameState, BlockP);
        if(ZeroHash->BlockIndex == HASH_UNINITIALIZED)
        {
            block_hash *BlockHash = GetBlockHash(GameState, BlockP);
            if((BlockHash->BlockIndex != HASH_UNINITIALIZED) &&
               (BlockHash->BlockIndex != HASH_DELETED))
            {
                terrain_render_block *Block = GameState->PoligonisedBlocks2 + BlockHash->BlockIndex;
                v3 P = Block->Pos - CameraP;
                
                v3 P0 = P;
                v3 P1 = P + Diff2X;
                v3 P2 = P + Diff2Y;
                v3 P3 = P + Diff2Z;
                v3 P4 = P + Diff2X + Diff2Y;
                v3 P5 = P + Diff2Y + Diff2Z;
                v3 P6 = P + Diff2X + Diff2Z;
                v3 P7 = P + Diff2X + Diff2Y + Diff2Z;
                if((DotProduct(P0, CamDir) > 0.0f) || (DotProduct(P1, CamDir) > 0.0f) ||
                   (DotProduct(P2, CamDir) > 0.0f) || (DotProduct(P3, CamDir) > 0.0f) ||
                   (DotProduct(P4, CamDir) > 0.0f) || (DotProduct(P5, CamDir) > 0.0f) ||
                   (DotProduct(P6, CamDir) > 0.0f) || (DotProduct(P7, CamDir) > 0.0f))
                {
                    GameState->RenderBlocks[GameState->RenderBlockCount++] = Block;
                    Assert(GameState->RenderBlockCount < ArrayCount(GameState->RenderBlocks));
                }   
            }
        }
    }*/
}

internal void
RenderGame(game_state *GameState, camera *Camera)
{
    win32_clock RenderClock;
    dx_resource *DXResources = GameState->DXResources;
    DXResources->LoadResource(Camera->SceneConstantBuffer,
                  &Camera->SceneConstants, sizeof(Camera->SceneConstants));
    
    v4 BackgroundColor = {0.0f, 0.2f, 0.4f, 1.0f};
    DXResources->DeviceContext->ClearDepthStencilView(DXResources->DepthStencilView, 
        D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
    DXResources->DeviceContext->ClearRenderTargetView(DXResources->BackBuffer, BackgroundColor.C);
    
    // NOTE: Background rendering
    DXResources->SetDrawModeDefault();
    DXResources->DeviceContext->IASetInputLayout(DXResources->BackgroundInputLayout);
    DXResources->DeviceContext->VSSetShader(DXResources->BackgroundVS, 0, 0);
    DXResources->DeviceContext->PSSetShader(DXResources->BackgroundPS, 0, 0);
    DXResources->DeviceContext->PSSetSamplers(0, 1, &DXResources->CubeTexSamplerState);
    
    v3 BGVertices[6] = {{-1.0, -1.0, 0.99f},
                        {-1.0,  1.0, 0.99f},
                        { 1.0, -1.0, 0.99f},
                        { 1.0, -1.0, 0.99f},
                        {-1.0,  1.0, 0.99f},
                        { 1.0,  1.0, 0.99f}};
    DXResources->SetTransformations(v3{});
    v3 CamDir = Camera->GetLookDirection();
    DXResources->ObjectConstants.CameraDir = DirectX::XMFLOAT4(CamDir.X, CamDir.Y, CamDir.Z, 0.0f);
    // DXResources->ObjectConstants.CameraDir = DirectX::XMFLOAT4(1, 0, 0, 1.0f);
    DXResources->DrawBackground(BGVertices, 6);
    
    // NOTE: Render axis
    DXResources->SetDrawModeDefault();
    DXResources->DeviceContext->IASetInputLayout(DXResources->TerrainInputLayout);
    DXResources->DeviceContext->VSSetShader(DXResources->TerrainVS, 0, 0);
    DXResources->DeviceContext->PSSetShader(DXResources->LinePS, 0, 0);
    DXResources->DeviceContext->PSSetSamplers(0, 1, &DXResources->TexSamplerState);
    
    real32 AxisSize = 256;
    const uint32 VertCount = 6;
    v4 Red{1.0f, 0.0f, 0.0f, 1.0f},
       Green{0.0f, 1.0f, 0.0f, 1.0f}, 
       Blue{0.0f, 0.0f, 1.0f, 1.0f};
    v3 Normal{0.0f, 1.0f, 0.0f};
    vertex AxisVertices[VertCount]={Get3DVertex(v3{ 1.0f*AxisSize,  0.0f,  0.0f}, Normal, Red),
                                    Get3DVertex(v3{-1.0f*AxisSize,  0.0f,  0.0f}, Normal, Red),
                                    Get3DVertex(v3{ 0.0f,  1.0f*AxisSize,  0.0f}, Normal, Green),
                                    Get3DVertex(v3{ 0.0f, -1.0f*AxisSize,  0.0f}, Normal, Green),
                                    Get3DVertex(v3{ 0.0f,  0.0f,  1.0f*AxisSize}, Normal, Blue),
                                    Get3DVertex(v3{ 0.0f,  0.0f, -1.0f*AxisSize}, Normal, Blue)};
    DXResources->DrawLines(AxisVertices, VertCount);
    // DXResources->DrawDebugTriangle();

    if(GameState->RenderMode)
    {
        DXResources->SetDrawModeWireframe();
    }
    else
    {
        DXResources->SetDrawModeDefault();
    }
    
    DXResources->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DXResources->DeviceContext->VSSetShader(DXResources->TerrainVS, 0, 0);
    DXResources->DeviceContext->PSSetShader(DXResources->TerrainPS, 0, 0);
    
    for(size_t RenderBlockIndex = 0; 
        RenderBlockIndex < GameState->RenderBlockCount; 
        RenderBlockIndex++)
    {
        DXResources->SetTransformations(GameState->RenderBlocks[RenderBlockIndex]->Pos);
        DXResources->DrawTriangles(
            GameState->RenderBlocks[RenderBlockIndex]->Vertices,
            GameState->RenderBlocks[RenderBlockIndex]->VertexCount);
        DXResources->SetTransformations(v3{});
    }
    
    DXResources->SwapChain->Present(0, 0);
    //RenderClock.PrintMiliSeconds("Render time:");
}
