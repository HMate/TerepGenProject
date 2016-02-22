/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_random.cpp"
#include "terepgen_terrain.cpp"
#include "terepgen_dx_renderer.cpp"
    
// TODO: Should be based on resolution!
internal void 
CalculateBlockPositions(block_pos_array *PosArray, world_block_pos CentralBlockPos, int32 Radius)
{    
    PosArray->Count = 0;
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
                    PosArray->Pos[PosArray->Count].BlockX = CentralBlockPos.BlockX + XIndex;
                    PosArray->Pos[PosArray->Count].BlockY = CentralBlockPos.BlockY + YIndex;
                    PosArray->Pos[PosArray->Count].BlockZ = CentralBlockPos.BlockZ + ZIndex;
                    PosArray->Pos[PosArray->Count].Resolution = CentralBlockPos.Resolution;
                    PosArray->Count++;
                    
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
    
    Assert(PosArray->Count <= ArrayCount(PosArray->Pos));
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
    // TODO: Be more concise
    // TODO: Give Resolution in param
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
AddToRenderBlocks(game_state *GameState, terrain_render_block *Block, int32 Resolution)
{
    const v3 CameraP = GameState->CameraPos;
    const v3 CamDir = GameState->CameraDir;
    const v3 DiffX = V3FromWorldPos(&GameState->WorldDensity, world_block_pos{1,0,0,Resolution});
    const v3 DiffY = V3FromWorldPos(&GameState->WorldDensity, world_block_pos{0,1,0,Resolution});
    const v3 DiffZ = V3FromWorldPos(&GameState->WorldDensity, world_block_pos{0,0,1,Resolution});
    
    v3 P = Block->Pos - CameraP;
    
    v3 P0 = P;
    v3 P1 = P + DiffX;
    v3 P2 = P + DiffY;
    v3 P3 = P + DiffZ;
    v3 P4 = P + DiffX + DiffY;
    v3 P5 = P + DiffY + DiffZ;
    v3 P6 = P + DiffX + DiffZ;
    v3 P7 = P + DiffX + DiffY + DiffZ;
    if((DotProduct(P0, CamDir) > 0.0f) || (DotProduct(P1, CamDir) > 0.0f) ||
       (DotProduct(P2, CamDir) > 0.0f) || (DotProduct(P3, CamDir) > 0.0f) ||
       (DotProduct(P4, CamDir) > 0.0f) || (DotProduct(P5, CamDir) > 0.0f) ||
       (DotProduct(P6, CamDir) > 0.0f) || (DotProduct(P7, CamDir) > 0.0f))
    {
        GameState->RenderBlocks[GameState->RenderBlockCount++] = Block;
        Assert(GameState->RenderBlockCount < ArrayCount(GameState->RenderBlocks));
    }  
}

internal void
AddCube(game_state *GameState, v3 WorldMouse)
{
    GameState->Cube.Pos = WorldMouse;

    const v3 Corner0 ={-0.5f, -0.5f, -0.5f};
    const v3 Corner1 ={-0.5f, 0.5f, -0.5f};
    const v3 Corner2 ={0.5f, -0.5f, -0.5f};
    const v3 Corner3 ={0.5f, 0.5f, -0.5f};
    const v3 Corner4 ={-0.5f, -0.5f, 0.5f};
    const v3 Corner5 ={-0.5f, 0.5f, 0.5f};
    const v3 Corner6 ={0.5f, -0.5f, 0.5f};
    const v3 Corner7 ={0.5f, 0.5f, 0.5f};
    
    v3 Normal0 = v3{0.0f, 0.0f, -1.0f}; //front
    v3 Normal1 = v3{1.0f, 0.0f, 0.0f};  //right
    v3 Normal2 = v3{0.0f, 0.0f, 1.0f};  //back
    v3 Normal3 = v3{-1.0f, 0.0f, 0.0f}; //left
    v3 Normal4 = v3{0.0f, -1.0f, 0.0f}; // down
    v3 Normal5 = v3{0.0f, 1.0f, 0.0f};  //up
    
    v4 ColorR = {1.0f, 0.0f, 0.0f, 1.0f};
    v4 ColorG = {0.0f, 1.0f, 0.0f, 1.0f};
    v4 ColorB = {0.0f, 0.0f, 1.0f, 1.0f};
    
    GameState->Cube.Vertices[0] = Vertex(Corner0, Normal0, ColorB);
    GameState->Cube.Vertices[1] = Vertex(Corner1, Normal0, ColorB);
    GameState->Cube.Vertices[2] = Vertex(Corner2, Normal0, ColorB);
    GameState->Cube.Vertices[3] = Vertex(Corner2, Normal0, ColorB);
    GameState->Cube.Vertices[4] = Vertex(Corner1, Normal0, ColorB);
    GameState->Cube.Vertices[5] = Vertex(Corner3, Normal0, ColorB);
    
    GameState->Cube.Vertices[6] = Vertex(Corner2, Normal1, ColorR);
    GameState->Cube.Vertices[7] = Vertex(Corner3, Normal1, ColorR);
    GameState->Cube.Vertices[8] = Vertex(Corner6, Normal1, ColorR);
    GameState->Cube.Vertices[9] = Vertex(Corner6, Normal1, ColorR);
    GameState->Cube.Vertices[10] = Vertex(Corner3, Normal1, ColorR);
    GameState->Cube.Vertices[11] = Vertex(Corner7, Normal1, ColorR);
    
    GameState->Cube.Vertices[12] = Vertex(Corner6, Normal2, ColorB);
    GameState->Cube.Vertices[13] = Vertex(Corner7, Normal2, ColorB);
    GameState->Cube.Vertices[14] = Vertex(Corner4, Normal2, ColorB);
    GameState->Cube.Vertices[15] = Vertex(Corner4, Normal2, ColorB);
    GameState->Cube.Vertices[16] = Vertex(Corner7, Normal2, ColorB);
    GameState->Cube.Vertices[17] = Vertex(Corner5, Normal2, ColorB);
    
    GameState->Cube.Vertices[18] = Vertex(Corner4, Normal3, ColorR);
    GameState->Cube.Vertices[19] = Vertex(Corner5, Normal3, ColorR);
    GameState->Cube.Vertices[20] = Vertex(Corner0, Normal3, ColorR);
    GameState->Cube.Vertices[21] = Vertex(Corner0, Normal3, ColorR);
    GameState->Cube.Vertices[22] = Vertex(Corner5, Normal3, ColorR);
    GameState->Cube.Vertices[23] = Vertex(Corner1, Normal3, ColorR);
    
    GameState->Cube.Vertices[24] = Vertex(Corner4, Normal4, ColorG);
    GameState->Cube.Vertices[25] = Vertex(Corner0, Normal4, ColorG);
    GameState->Cube.Vertices[26] = Vertex(Corner6, Normal4, ColorG);
    GameState->Cube.Vertices[27] = Vertex(Corner6, Normal4, ColorG);
    GameState->Cube.Vertices[28] = Vertex(Corner0, Normal4, ColorG);
    GameState->Cube.Vertices[29] = Vertex(Corner2, Normal4, ColorG);
    
    GameState->Cube.Vertices[30] = Vertex(Corner1, Normal5, ColorG);
    GameState->Cube.Vertices[31] = Vertex(Corner5, Normal5, ColorG);
    GameState->Cube.Vertices[32] = Vertex(Corner3, Normal5, ColorG);
    GameState->Cube.Vertices[33] = Vertex(Corner3, Normal5, ColorG);
    GameState->Cube.Vertices[34] = Vertex(Corner5, Normal5, ColorG);
    GameState->Cube.Vertices[35] = Vertex(Corner7, Normal5, ColorG);
}

internal void
UpdateGameState(game_state *GameState, v3 WorldMouse)
{
    world_density *World = &GameState->WorldDensity;
    if(GameState->Initialized == false)
    {
        GameState->WorldDensity.BlockSize = real32(TERRAIN_BLOCK_SIZE);
        SetSeed(&GameState->PerlinArray.Noise[0], GameState->Seed);
        SetSeed(&GameState->PerlinArray.Noise[1], GameState->Seed+1);
        SetSeed(&GameState->PerlinArray.Noise[2], GameState->Seed+2);
        InitBlockHash(World);
        InitZeroHash(World);
        
        GameState->Initialized = true;
    }
    
    // TODO: Refactor to input!
    bool32 MouseRightIsDown = GetKeyState(VK_RBUTTON) & (1 << 15);
    if(MouseRightIsDown)
    {
        AddCube(GameState, WorldMouse);
    }
    
    v3 CameraP = GameState->CameraPos;
    world_block_pos WorldCameraP = WorldPosFromV3(World, CameraP, 4);
    block_pos_array BlockPositions;
    CalculateBlockPositions(&BlockPositions, WorldCameraP, RENDERED_BLOCK_RADIUS);
    
    //
    // NOTE: Delete blocks that are too far from the camera
    //
    // TODO: Maybe we need to reinitialize the block hash, if there are too many deleted blocks?
    // TODO: Delete from other resolutions too!
    /*int32 LoadSpaceRadius = RENDERED_BLOCK_RADIUS;
    for(uint32 StoreIndex = 0; 
        StoreIndex < World->DensityBlockCount; 
        ++StoreIndex)
    {
        terrain_density_block *Block = World->DensityBlocks + StoreIndex;
        world_block_pos BlockP = WorldPosFromV3(World, Block->Pos, 4);
        if((WorldCameraP.BlockX + LoadSpaceRadius < BlockP.BlockX) ||
           (WorldCameraP.BlockX - LoadSpaceRadius > BlockP.BlockX) ||
           (WorldCameraP.BlockY + LoadSpaceRadius < BlockP.BlockY) ||
           (WorldCameraP.BlockY - LoadSpaceRadius > BlockP.BlockY) ||
           (WorldCameraP.BlockZ + LoadSpaceRadius < BlockP.BlockZ) ||
           (WorldCameraP.BlockZ - LoadSpaceRadius > BlockP.BlockZ))
        {
            terrain_density_block *Last = World->DensityBlocks + (--World->DensityBlockCount);
            world_block_pos LastP = WorldPosFromV3(World, Last->Pos, 4);
            
            // NOTE: This is from stored blocks, so it cant be a zero block!
            block_hash *RemovedHash = GetHash((block_hash*)&World->BlockHash, BlockP);
            Assert(RemovedHash->Index != HASH_UNINITIALIZED && RemovedHash->Index != HASH_DELETED);
            block_hash *LastHash = GetHash((block_hash*)&World->BlockHash, LastP);
            Assert(LastHash->Index != HASH_UNINITIALIZED && LastHash->Index != HASH_DELETED);
            
            RemovedHash->Index = HASH_DELETED;
            LastHash->Index = StoreIndex;
            World->DeletedBlockCount++;
            
            *Block = *Last;
        }
    }*/
    
    int32 ZeroGridTotalSize = POS_GRID_SIZE(ZERO_BLOCK_RADIUS);
    Assert(ZeroGridTotalSize < ZERO_HASH_SIZE);
    if(World->ZeroBlockCount > (ArrayCount(World->ZeroHash)*7/8))
    {
        int32 ZeroSpaceRadius = ZERO_BLOCK_RADIUS;
        block_hash NewZeroHash[ZERO_HASH_SIZE];
        uint32 NewZeroHashEntryCount = World->ZeroBlockCount;
        for(uint32 ZeroIndex = 0; 
            ZeroIndex < ArrayCount(World->ZeroHash); 
            ++ZeroIndex)
        {
            NewZeroHash[ZeroIndex] = World->ZeroHash[ZeroIndex];
        }
        
        InitZeroHash(World);
        
        for(uint32 ZeroIndex = 0; 
            ZeroIndex < ArrayCount(World->ZeroHash); 
            ++ZeroIndex)
        {
            block_hash *Entry = NewZeroHash + ZeroIndex;
            world_block_pos ZeroP = Entry->Key;
            if((Entry->Index == HASH_ZERO_BLOCK) &&
              ((WorldCameraP.BlockX + ZeroSpaceRadius > ZeroP.BlockX) &&
               (WorldCameraP.BlockX - ZeroSpaceRadius < ZeroP.BlockX) &&
               (WorldCameraP.BlockY + ZeroSpaceRadius > ZeroP.BlockY) &&
               (WorldCameraP.BlockY - ZeroSpaceRadius < ZeroP.BlockY) &&
               (WorldCameraP.BlockZ + ZeroSpaceRadius > ZeroP.BlockZ) &&
               (WorldCameraP.BlockZ - ZeroSpaceRadius < ZeroP.BlockZ)))
            {
                block_hash *ZeroHash = GetZeroHash(World, ZeroP);
                ZeroHash->Key = ZeroP;
                ZeroHash->Index = HASH_ZERO_BLOCK;
                World->ZeroBlockCount++;
            }
        }
    }
    
    //
    // NOTE: Collect the render block we want to draw to the screen
    //
    
    uint32 MaxBlocksToGenerateInFrame = 3;
    for(size_t PosIndex = 0; 
        (PosIndex < BlockPositions.Count) && (MaxBlocksToGenerateInFrame > 0) ;
        ++PosIndex)
    {
        world_block_pos BlockP = ConvertToResolution(BlockPositions.Pos[PosIndex], 4);
        block_hash *ZeroHash = GetZeroHash(World, BlockP);
        if(ZeroHash->Index == HASH_UNINITIALIZED)
        {
            block_hash *BlockHash = GetHash((block_hash*)&World->BlockHash, BlockP);
            // NOTE: This can give back a deleted hash, if it had the same key as this block,
            // and it was already deleted once, and wasn't overwritten since.
            if(HashIsEmpty(BlockHash))
            {
                if(MaxBlocksToGenerateInFrame > 0)
                {
                    MaxBlocksToGenerateInFrame--;
                    // NOTE: Initialize block
                    uint32 BlockIndex = World->DensityBlockCount;
                    terrain_density_block *DensityBlock = World->DensityBlocks + BlockIndex;
                    
                    DensityBlock->Pos = V3FromWorldPos(World, BlockP);
                    GenerateDensityGrid(DensityBlock, &GameState->PerlinArray, 4);
                    // TODO: What should i do with neighbouring blocks for normal calculation?
                    PoligoniseBlock(World, &World->PoligonisedBlocks[World->PoligonisedBlockCount], 
                        DensityBlock);
                        
                    BlockHash = WriteHash((block_hash*)&World->BlockHash, BlockP, World->DensityBlockCount++);
                    Assert(World->DensityBlockCount < ArrayCount(World->DensityBlocks));
                    
                    if(World->PoligonisedBlocks[World->PoligonisedBlockCount].VertexCount != 0)
                    {
                        BlockHash = WriteHash((block_hash*)&World->RenderHash, BlockP, World->PoligonisedBlockCount++);
                        Assert(World->PoligonisedBlockCount < ArrayCount(World->PoligonisedBlocks));
                    }
                    else
                    {
                        ZeroHash->Key = BlockP;
                        ZeroHash->Index = HASH_ZERO_BLOCK;
                        World->ZeroBlockCount++;
                    }
                }
            }
        }
    }
    
    win32_clock PoligoniseClock;
    GameState->RenderBlockCount = 0;
    for(size_t PosIndex = 0; 
        (PosIndex < BlockPositions.Count);
        ++PosIndex)
    {
        world_block_pos BlockP = ConvertToResolution(BlockPositions.Pos[PosIndex], 4);
        block_hash *ZeroHash = GetZeroHash(World, BlockP);
        if(ZeroHash->Index == HASH_UNINITIALIZED)
        {
            block_hash *Hash = GetHash((block_hash*)&World->RenderHash, BlockP);
            
            if(!HashIsEmpty(Hash))
            {
                // TODO: dont forget to empty the hashes too.
                AddToRenderBlocks(GameState, World->PoligonisedBlocks + Hash->Index, 4);
            }
        }
    }
    PoligoniseClock.PrintMiliSeconds("Poligonise time:");
    
    /*
    terrain_density_block DensityBlock;
    uint32 MaxBlocksToGenerateInFrame = 3;
    for(size_t PosIndex = 0; 
        (PosIndex < BlockPositions.Count) && (MaxBlocksToGenerateInFrame > 0) ;
        ++PosIndex)
    {
        world_block_pos BlockP = ConvertToResolution(BlockPositions.Pos[PosIndex], 4);
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
                    GenerateDensityGrid(&DensityBlock, &GameState->PerlinArray, 4);
                    PoligoniseBlock(&(GameState->PoligonisedBlocks4[GameState->PoligonisedBlock4Count]), 
                        &DensityBlock, 4);
                    if(GameState->PoligonisedBlocks4[GameState->PoligonisedBlock4Count].VertexCount != 0)
                    {
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
    
    world_block_pos WorldCameraP2 = ConvertToResolution(WorldCameraP, 2);
    block_pos_array Block2Positions;
    CalculateBlockPositions(&Block2Positions, WorldCameraP2, RENDERED_BLOCK_RADIUS);
    
    for(size_t PosIndex = 0; 
        (PosIndex < Block2Positions.Count) && (MaxBlocksToGenerateInFrame > 0) ;
        ++PosIndex)
    {
        world_block_pos BlockP = ConvertToResolution(Block2Positions.Pos[PosIndex], 2);
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
    
    GameState->RenderBlockCount = 0;
    for(size_t PosIndex = 0; 
        (PosIndex < BlockPositions.Count);
        ++PosIndex)
    {
        world_block_pos BlockP = ConvertToResolution(BlockPositions.Pos[PosIndex], 4);
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
                        AddToRenderBlocks(GameState, GameState->PoligonisedBlocks2, LowBlockHash->BlockIndex, 2);
                    }
                }
            }
            else if((BlockHash->BlockIndex != HASH_UNINITIALIZED) &&
                    (BlockHash->BlockIndex != HASH_DELETED))
            {
                // TODO: Add polgonising here
                // And dont forget to empty the hashes too.
                AddToRenderBlocks(GameState, GameState->PoligonisedBlocks4, BlockHash->BlockIndex, 4);
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
    
    // Draw cube
    DXResources->DeviceContext->PSSetShader(DXResources->LinePS, 0, 0);
    
    DXResources->SetTransformations(GameState->Cube.Pos);
    DXResources->DrawTriangles(GameState->Cube.Vertices, 36);
    DXResources->SetTransformations(v3{});
    
    DXResources->SwapChain->Present(0, 0);
    //RenderClock.PrintMiliSeconds("Render time:");
}
