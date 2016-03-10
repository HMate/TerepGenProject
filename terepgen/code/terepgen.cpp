/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_random.cpp"
#include "terepgen_terrain.cpp"
#include "terepgen_dx_renderer.cpp"
    
// TODO: Should be based on resolution!
internal void 
CalculateBlockPositions(block_pos_array *PosArray, world_block_pos *CentralBlockPos, int32 Radius)
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
                    PosArray->Pos[PosArray->Count].BlockX = CentralBlockPos->BlockX + XIndex;
                    PosArray->Pos[PosArray->Count].BlockY = CentralBlockPos->BlockY + YIndex;
                    PosArray->Pos[PosArray->Count].BlockZ = CentralBlockPos->BlockZ + ZIndex;
                    PosArray->Pos[PosArray->Count].Resolution = CentralBlockPos->Resolution;
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
AddToRenderBlocks(game_state *GameState, terrain_render_block *Block)
{
    const uint32 Resolution = Block->Resolution;
    const v3 CameraP = GameState->CameraPos;
    const v3 CamDir = GameState->CameraDir;
    const v3 DiffX = V3FromWorldPos(world_block_pos{1,0,0,Resolution});
    const v3 DiffY = V3FromWorldPos(world_block_pos{0,1,0,Resolution});
    const v3 DiffZ = V3FromWorldPos(world_block_pos{0,0,1,Resolution});
    
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
AddCube(game_state *GameState, v3 WorldMousePos)
{
    GameState->Cube.Pos = WorldMousePos;

    const v3 Corner0 ={-0.5f, -0.5f, -0.5f};
    const v3 Corner1 ={-0.5f, 0.5f, -0.5f};
    const v3 Corner2 ={0.5f, -0.5f, -0.5f};
    const v3 Corner3 ={0.5f, 0.5f, -0.5f};
    const v3 Corner4 ={-0.5f, -0.5f, 0.5f};
    const v3 Corner5 ={-0.5f, 0.5f, 0.5f};
    const v3 Corner6 ={0.5f, -0.5f, 0.5f};
    const v3 Corner7 ={0.5f, 0.5f, 0.5f};
    
    const v3 Normal0 = v3{0.0f, 0.0f, -1.0f}; //front
    const v3 Normal1 = v3{1.0f, 0.0f, 0.0f};  //right
    const v3 Normal2 = v3{0.0f, 0.0f, 1.0f};  //back
    const v3 Normal3 = v3{-1.0f, 0.0f, 0.0f}; //left
    const v3 Normal4 = v3{0.0f, -1.0f, 0.0f}; // down
    const v3 Normal5 = v3{0.0f, 1.0f, 0.0f};  //up
    
    const v4 ColorR = {1.0f, 0.0f, 0.0f, 1.0f};
    const v4 ColorG = {0.0f, 1.0f, 0.0f, 1.0f};
    const v4 ColorB = {0.0f, 0.0f, 1.0f, 1.0f};
    
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
DeleteRenderBlock(world_density *World, int32 StoreIndex)
{
    terrain_render_block *Block = World->PoligonisedBlocks + StoreIndex;
    terrain_render_block *Last = World->PoligonisedBlocks + (--World->PoligonisedBlockCount);
    world_block_pos BlockP = WorldPosFromV3(Block->Pos, Block->Resolution);
    world_block_pos LastP = WorldPosFromV3(Last->Pos, Last->Resolution);
    
    // NOTE: This is from stored blocks, so it cant be a zero block!
    block_hash *RemovedHash = GetHash((block_hash*)&World->RenderHash, BlockP);
    Assert(RemovedHash->Index != HASH_UNINITIALIZED && RemovedHash->Index != HASH_DELETED);
    block_hash *LastHash = GetHash((block_hash*)&World->RenderHash, LastP);
    Assert(LastHash->Index != HASH_UNINITIALIZED && LastHash->Index != HASH_DELETED);
    
    LastHash->Index = StoreIndex;
    RemovedHash->Index = HASH_DELETED;
    World->DeletedRenderBlockCount++;
    
    // NOTE: If we are deleting the last block
    if(StoreIndex != (int32)World->PoligonisedBlockCount)
    {
        *Block = *Last;
    }
}

internal bool32
DoRectangleContains(world_block_pos *Center, int32 Radius, world_block_pos *P)
{
    bool32 Result = (Center->BlockX + Radius < P->BlockX) ||
                    (Center->BlockX - Radius > P->BlockX) ||
                    (Center->BlockY + Radius < P->BlockY) ||
                    (Center->BlockY - Radius > P->BlockY) ||
                    (Center->BlockZ + Radius < P->BlockZ) ||
                    (Center->BlockZ - Radius > P->BlockZ);
    return Result;
}

internal bool32
DidDensityBlocksLoaded(world_density *World, world_block_pos *Positions, uint32 Count)
{
    bool32 Result = true;
    for(uint32 PosIndex = 0;
        PosIndex < Count;
        ++PosIndex)
    {
        block_hash *BlockHash = GetHash(World->BlockHash, Positions[PosIndex]);
        if(HashIsEmpty(BlockHash))
        {
            Result = false;
        }
    }
    
    return Result;
}

internal bool32
DidRenderBlocksLoaded(world_density *World, world_block_pos *Positions, uint32 Count)
{
    bool32 Result = true;
    for(uint32 PosIndex = 0;
        PosIndex < Count;
        ++PosIndex)
    {
        block_hash *RenderHash = GetHash(World->RenderHash, Positions[PosIndex]);
        block_hash *ZeroHash = GetZeroHash(World, Positions[PosIndex]);
        if(HashIsEmpty(RenderHash) && HashIsEmpty(ZeroHash))
        {
            Result = false;
        }
    }
    
    return Result;
}

internal void
UpdateGameState(game_state *GameState, v3 WorldMousePos)
{
    const uint32 ResolutionCount = 2;
    const uint32 FixedResolution = 4;
    const uint32 FixedResolution2 = FixedResolution / 2;

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
    
    v3 CameraP = GameState->CameraPos;
    world_block_pos WorldCameraP[ResolutionCount] = {
        WorldPosFromV3(CameraP, FixedResolution),
        WorldPosFromV3(CameraP, FixedResolution2)
    };
    block_pos_array BlockPositionStore[ResolutionCount];
    CalculateBlockPositions(BlockPositionStore, WorldCameraP, RENDERED_BLOCK_RADIUS);
    CalculateBlockPositions(BlockPositionStore + 1, WorldCameraP + 1, RENDERED_BLOCK_RADIUS);
    
    win32_clock Clock;
    //
    // NOTE: Delete blocks that are too far from the camera
    //
    // TODO: Maybe we need to reinitialize the block hash, if there are too many deleted blocks?
    // TODO: Delete from other resolutions too!
    if(World->DensityBlockCount > (ArrayCount(World->DensityBlocks) * 7 / 8))
    {
        int32 LoadSpaceRadius = RENDERED_BLOCK_RADIUS;
        for(uint32 StoreIndex = 0; 
            StoreIndex < World->DensityBlockCount; 
            ++StoreIndex)
        {
            terrain_density_block *Block = World->DensityBlocks + StoreIndex;
            world_block_pos BlockP = Block->Pos;
            if(DoRectangleContains(WorldCameraP, LoadSpaceRadius, &BlockP))
            {
                terrain_density_block *Last = World->DensityBlocks + (--World->DensityBlockCount);
                world_block_pos LastP = Last->Pos;
                
                // NOTE: This is from stored blocks, so it cant be a zero block!
                block_hash *RemovedHash = GetHash(World->BlockHash, BlockP);
                Assert(RemovedHash->Index != HASH_UNINITIALIZED && RemovedHash->Index != HASH_DELETED);
                block_hash *LastHash = GetHash(World->BlockHash, LastP);
                Assert(LastHash->Index != HASH_UNINITIALIZED && LastHash->Index != HASH_DELETED);
                
                LastHash->Index = StoreIndex;
                RemovedHash->Index = HASH_DELETED;
                World->DeletedDensityBlockCount++;
                
                if(StoreIndex != (int32)World->DensityBlockCount)
                {
                    *Block = *Last;
                }
            }
        }
    }
    Clock.Reset();
    
    if(World->PoligonisedBlockCount > (ArrayCount(World->PoligonisedBlocks) * 7/8))
    {
        int32 LoadSpaceRadius = RENDERED_BLOCK_RADIUS;
        for(uint32 StoreIndex = 0; 
            StoreIndex < World->PoligonisedBlockCount; 
            ++StoreIndex)
        {
            terrain_render_block *Block = World->PoligonisedBlocks + StoreIndex;
            world_block_pos BlockP = WorldPosFromV3(Block->Pos, FixedResolution);
            if(DoRectangleContains(WorldCameraP, LoadSpaceRadius, &BlockP))
            {
                DeleteRenderBlock(World, StoreIndex);
            }
        }
    }
    Clock.Reset();
    
    int32 ZeroGridTotalSize = POS_GRID_SIZE(ZERO_BLOCK_RADIUS);
    Assert(ZeroGridTotalSize < ZERO_HASH_SIZE);
    if(World->ZeroBlockCount > (ArrayCount(World->ZeroHash)*7/8))
    {
        int32 ZeroSpaceRadius = ZERO_BLOCK_RADIUS;
        block_hash NewZeroHash[ZERO_HASH_SIZE];
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
               DoRectangleContains(WorldCameraP, ZeroSpaceRadius, &ZeroP))
            {
                block_hash *ZeroHash = WriteZeroHash(World, ZeroP);
                World->ZeroBlockCount++;
            }
        }
    }
    //real64 DeleteZeroTime = Clock.GetSecondsElapsed();
    Clock.Reset();
    
    // NOTE: Generate density blocks
    uint32 MaxBlocksToGenerateInFrame = 3;
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < ResolutionCount;
        ResolutionIndex++)
    {
        block_pos_array *BlockPositions = BlockPositionStore + ResolutionIndex;
        for(size_t PosIndex = 0; 
            (PosIndex < BlockPositions->Count) && (MaxBlocksToGenerateInFrame > 0) ;
            ++PosIndex)
        {
            world_block_pos BlockP = BlockPositions->Pos[PosIndex];
            block_hash *BlockHash = GetHash(World->BlockHash, BlockP);
            // NOTE: This can give back a deleted hash, if it had the same key as this block,
            // and it was already deleted once, and wasn't overwritten since.
            if(HashIsEmpty(BlockHash))
            {
                MaxBlocksToGenerateInFrame--;
                // NOTE: Initialize block
                uint32 BlockIndex = World->DensityBlockCount;
                terrain_density_block *DensityBlock = World->DensityBlocks + BlockIndex;
                
                GenerateDensityGrid(DensityBlock, &GameState->PerlinArray, BlockP);
                    
                BlockHash = WriteHash(World->BlockHash, BlockP, World->DensityBlockCount++);
                Assert(World->DensityBlockCount < ArrayCount(World->DensityBlocks));
                
                // NOTE: Delete neighbouring render blocks
                const uint32 NeighbourCount = 27;
                world_block_pos NeighbourBlockPositions[NeighbourCount];
                GetNeighbourBlockPositions(NeighbourBlockPositions, BlockP);
                
                for(uint32 NeighbourIndex = 0;
                    NeighbourIndex < NeighbourCount;
                    NeighbourIndex++)
                {
                    world_block_pos NeighbourBP = NeighbourBlockPositions[NeighbourIndex];
                    block_hash *NRenderHash = GetHash(World->RenderHash, NeighbourBP);
                    if(!HashIsEmpty(NRenderHash))
                    {
                        DeleteRenderBlock(World, NRenderHash->Index);
                    }
                    block_hash *NZeroHash = GetZeroHash(World, NeighbourBP);
                    if(!HashIsEmpty(NZeroHash))
                    {
                        NZeroHash->Index = HASH_DELETED;
                        World->ZeroBlockCount--;
                    }
                }
            }
        }
    }
    
    real64 TimeGenerateDensity = Clock.GetSecondsElapsed();
    Clock.Reset();
    
    uint32 MaxRenderBlocksToGenerateInFrame = 6;
#if 0
    // TODO: Refactor mouse buttons to input!
    bool32 MouseRightIsDown = GetKeyState(VK_RBUTTON) & (1 << 15);
    if(MouseRightIsDown)
    {
        v3 RayDirection = Normalize(WorldMousePos - GameState->CameraOrigo);
        for(real32 RayLength = 0.5f; 
            RayLength < 2000.0f; 
            RayLength += 0.5f)
        {
            v3 CheckPos = GameState->CameraOrigo + (RayLength*RayDirection);
            real32 PosValue = GetWorldGridValueFromV3(World, CheckPos, FixedResolution);
            if(PosValue < DENSITY_ISO_LEVEL)
            {
                real32 SphereRadius = 30.0f;
                v3 StartBlockRP = CheckPos - v3{SphereRadius, SphereRadius, SphereRadius};
                v3 EndBlockRP = CheckPos + v3{SphereRadius, SphereRadius, SphereRadius};
                block_node StartNode = ConvertRenderPosToBlockNode(StartBlockRP, FixedResolution);
                block_node EndNode = ConvertRenderPosToBlockNode(EndBlockRP, FixedResolution);
                
                uint32 BlocksTouched = 0;
                block_node Node = StartNode;
                for(uint32 XIndex = 0;
                    (Node.BlockP.BlockX != EndNode.BlockP.BlockX) || (Node.X != EndNode.X);
                    XIndex++)
                {
                    Node = GetActualBlockNode(&StartNode.BlockP, 
                                StartNode.X+XIndex, StartNode.Y, StartNode.Z);
                    for(uint32 YIndex = 0; 
                        (Node.BlockP.BlockY != EndNode.BlockP.BlockY) || (Node.Y != EndNode.Y);
                        YIndex++)
                    {
                        Node = GetActualBlockNode(&StartNode.BlockP, 
                                    StartNode.X+XIndex, StartNode.Y+YIndex, StartNode.Z);
                        for(uint32 ZIndex = 0; 
                            (Node.BlockP.BlockZ != EndNode.BlockP.BlockZ) || (Node.Z != EndNode.Z);
                            ZIndex++)
                        {
                            Node = GetActualBlockNode(&StartNode.BlockP, 
                                        StartNode.X+XIndex, StartNode.Y+YIndex, StartNode.Z+ZIndex);
                            // NOTE: Change node density and invalidate render block
                            v3 NodeRenderP = ConvertBlockNodeToRenderPos(Node);
                            v3 Diff = NodeRenderP - CheckPos;
                            real32 Len = Length(Diff);
                            if(Len < 25.0f)
                            {
                                block_hash *BlockHash = GetHash(World->BlockHash, Node.BlockP);
                                if(!HashIsEmpty(BlockHash))
                                {
                                    terrain_density_block *ActDensityBlock = World->DensityBlocks + BlockHash->Index;
                                    real32 GridVal = GetGrid(&ActDensityBlock->Grid, Node.X, Node.Y, Node.Z);
                                    SetGrid(&ActDensityBlock->Grid, Node.X, Node.Y, Node.Z, GridVal + 1.0f);
                                    
                                    // NOTE: Delete this and neighbouring render blocks
                                    const uint32 NeighbourCount = 27;
                                    world_block_pos NeighbourBlockPositions[NeighbourCount];
                                    GetNeighbourBlockPositions(NeighbourBlockPositions, Node.BlockP);
                                    
                                    for(uint32 NeighbourIndex = 0;
                                        NeighbourIndex < NeighbourCount;
                                        NeighbourIndex++)
                                    {
                                        world_block_pos BlockP = NeighbourBlockPositions[NeighbourIndex];
                                        block_hash *NodeRenderHash = GetHash(World->RenderHash, BlockP);
                                        if(!HashIsEmpty(NodeRenderHash))
                                        {
                                            DeleteRenderBlock(World, NodeRenderHash->Index);
                                            BlocksTouched++;
                                        }
                                        block_hash *NodeZeroHash = GetZeroHash(World, BlockP);
                                        if(!HashIsEmpty(NodeZeroHash))
                                        {
                                            NodeZeroHash->Index = HASH_DELETED;
                                            World->ZeroBlockCount--;
                                            BlocksTouched++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                MaxRenderBlocksToGenerateInFrame += BlocksTouched;
                AddCube(GameState, CheckPos);
                break;
            }
        }
    }
#endif
    real64 TimeRightClick = Clock.GetSecondsElapsed();
    Clock.Reset();
    
    // NOTE: Render blocks in full resolution and mixed resolution passes?
    // for mixed pass, we have to get the blocks that have been rendered, but got a lower res neighbour since then?
    
    // If all the lower blocks of a block are generated, then the lower blocks can be mix rendered
    // If a mix rendered blocks neigbours are done generating their lower blocks, then they can be generated normally
    
    // When generated a density, just tell it to the neighbours, to rerender themselves
    
    win32_clock AvgClock;
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < ResolutionCount;
        ResolutionIndex++)
    {
        block_pos_array *BlockPositions = BlockPositionStore + ResolutionIndex;
        for(size_t PosIndex = 0; 
            (PosIndex < BlockPositions->Count) && (MaxRenderBlocksToGenerateInFrame > 0) ;
            ++PosIndex)
        {
            world_block_pos BlockP = BlockPositions->Pos[PosIndex];
            block_hash *ZeroHash = GetZeroHash(World, BlockP);
            if(HashIsEmpty(ZeroHash))
            {
                block_hash *RenderHash = GetHash(World->RenderHash, BlockP);
                // NOTE: This can give back a deleted hash, if it had the same key as this block,
                // and it was already deleted once, and wasn't overwritten since.
                if(HashIsEmpty(RenderHash))
                {                    
                    // NOTE: Check if lower resolution blocks are available
                    world_block_pos LowerBlockPositions[8];
                    GetLowerResBlockPositions(LowerBlockPositions, &BlockP);
                    
                    bool32 LowerBlocksLoaded = DidRenderBlocksLoaded(World, LowerBlockPositions, 8);
                    if(!LowerBlocksLoaded)
                    {
                        // NOTE: Neighbours include this block too
                        const uint32 NeighbourCount = 27;
                        world_block_pos NeighbourBlockPositions[NeighbourCount];
                        GetNeighbourBlockPositions(NeighbourBlockPositions, BlockP);
                        bool32 NeighboursGenerated = DidDensityBlocksLoaded(World, NeighbourBlockPositions, 27);
                        
                        if(NeighboursGenerated)
                        {
                            // NOTE: Initialize block
                            MaxRenderBlocksToGenerateInFrame--;
                            // NOTE: DensityHash has been checked among neighbours to be valid
                            block_hash *DensityHash = GetHash(World->BlockHash, BlockP);
                            terrain_density_block *DensityBlock = World->DensityBlocks + DensityHash->Index;
                            
                            AvgClock.Reset();
                            PoligoniseBlock(World, World->PoligonisedBlocks + World->PoligonisedBlockCount, 
                                DensityBlock);
                            
                            if(World->PoligonisedBlocks[World->PoligonisedBlockCount].VertexCount != 0)
                            {
                                real64 CurrentPoligoniseTime = AvgClock.GetSecondsElapsed();
                                //win32_printer::Print("poligonise: %f", CurrentPoligoniseTime * 1000.0);
                                real64 LastMeasure = GameState->PoligoniseTimeMeasured;
                                GameState->PoligoniseTimeMeasured += 1.0f;
                                GameState->AvgPoligoniseTime = 
                                    (LastMeasure/GameState->PoligoniseTimeMeasured)*GameState->AvgPoligoniseTime + 
                                    (CurrentPoligoniseTime / GameState->PoligoniseTimeMeasured);
                                
                                RenderHash = WriteHash(World->RenderHash, BlockP, World->PoligonisedBlockCount++);
                                Assert(World->PoligonisedBlockCount < ArrayCount(World->PoligonisedBlocks));
                            }
                            else
                            {
                                ZeroHash = WriteZeroHash(World, BlockP);
                                World->ZeroBlockCount++;
                            }
                        }
                        else if(BlockP.Resolution < FixedResolution)
                        {   // NOTE: Neighbours not generated, but maybe the block can be mixed rendered, if all
                            // its neighbours inside the its bigger block are generated
                            // If yes, then its bigger block can be deleted, and dont generate it again!
                            
                            world_block_pos BiggerBP = GetBiggerResBlockPosition(&BlockP);
                            
                            // NOTE: Check if same resolution blocks are available
                            world_block_pos SameResBlockPositions[8];
                            GetLowerResBlockPositions(SameResBlockPositions, &BiggerBP);
                            
                            bool32 SameResBlocksLoaded = DidDensityBlocksLoaded(World, SameResBlockPositions, 8);
                            
                            if(SameResBlocksLoaded)
                            {         
                                MaxRenderBlocksToGenerateInFrame--;
                                // NOTE: DensityHash has been checked among neighbours to be valid
                                block_hash *DensityHash = GetHash(World->BlockHash, BlockP);
                                terrain_density_block *DensityBlock = World->DensityBlocks + DensityHash->Index;
                                // NOTE: Mixed render these blocks
                                MixedPoligoniseBlock(World, World->PoligonisedBlocks + World->PoligonisedBlockCount, 
                                    DensityBlock);
                                
                                if(World->PoligonisedBlocks[World->PoligonisedBlockCount].VertexCount != 0)
                                {
                                    RenderHash = WriteHash(World->RenderHash, BlockP, World->PoligonisedBlockCount++);
                                    Assert(World->PoligonisedBlockCount < ArrayCount(World->PoligonisedBlocks));
                                }
                                else
                                {
                                    ZeroHash = WriteZeroHash(World, BlockP);
                                    World->ZeroBlockCount++;
                                }
                                
                                SameResBlocksLoaded = DidRenderBlocksLoaded(World, SameResBlockPositions, 8);                                
                                if(SameResBlocksLoaded)
                                {
                                    // NOTE: Delete bigger block from renderblocks, because lower blocks have loaded
                                    block_hash *BiggerRenderHash = GetHash(World->RenderHash, BiggerBP);
                                    if(!HashIsEmpty(BiggerRenderHash))
                                    {
                                        DeleteRenderBlock(World, BiggerRenderHash->Index);
                                    }
                                    block_hash *BiggerZeroHash = GetZeroHash(World, BiggerBP);
                                    if(!HashIsEmpty(BiggerZeroHash))
                                    {
                                        BiggerZeroHash->Index = HASH_DELETED;
                                        World->ZeroBlockCount--;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    real64 TimeGenerateRender = Clock.GetSecondsElapsed();
    Clock.Reset();
    
#if TEREPGEN_DEBUG
    for(uint32 DebugIndex = 0;
        DebugIndex < World->PoligonisedBlockCount;
        DebugIndex++)
    {
        uint32 DebCounter = 0;
        for(uint32 DebugIndex2 = 0;
            DebugIndex2 < World->PoligonisedBlockCount;
            DebugIndex2++)
        {
            terrain_render_block *Block1 = World->PoligonisedBlocks + DebugIndex;
            terrain_render_block *Block2 = World->PoligonisedBlocks + DebugIndex2;
            bool32 Equ = (Block1->Pos == Block2->Pos) && (Block1->Resolution == Block2->Resolution);
            if(Equ)
                DebCounter++;
        }
        Assert(DebCounter < 2);
    }
#endif
    
    GameState->RenderBlockCount = 0;
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < ResolutionCount;
        ResolutionIndex++)
    {
        block_pos_array *BlockPositions = BlockPositionStore + ResolutionIndex;
        for(size_t PosIndex = 0; 
            (PosIndex < BlockPositions->Count);
            ++PosIndex)
        {
            world_block_pos BlockP = BlockPositions->Pos[PosIndex];
            block_hash *ZeroHash = GetZeroHash(World, BlockP);
            if(HashIsEmpty(ZeroHash))
            {
                block_hash *Hash = GetHash(World->RenderHash, BlockP);
                
                if(!HashIsEmpty(Hash))
                {
                    AddToRenderBlocks(GameState, World->PoligonisedBlocks + Hash->Index);
                }
            }
        }
    }
    real64 TimeAddToRender = Clock.GetSecondsElapsed();
    
    // win32_printer::Print("Avg poligonise: %f", GameState->AvgPoligoniseTime * 1000.0);
    
    win32_printer::Print("Generate density time: %f", TimeGenerateDensity * 1000.0);
    win32_printer::Print("Right click time: %f", TimeRightClick * 1000.0);
    win32_printer::Print("Generate render: %f", TimeGenerateRender * 1000.0);
    win32_printer::Print("Add to render time: %f", TimeAddToRender * 1000.0);
    
}

internal void
RenderGame(game_state *GameState, camera *Camera)
{
    win32_clock RenderClock;
    dx_resource *DXResources = GameState->DXResources;
    DXResources->LoadResource(Camera->SceneConstantBuffer,
                  &Camera->SceneConstants, sizeof(Camera->SceneConstants));
    
    DXResources->ClearViews();
    
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
    }
    DXResources->SetTransformations(v3{});
    
    // Draw cube
    DXResources->DeviceContext->PSSetShader(DXResources->LinePS, 0, 0);
    
    DXResources->SetTransformations(GameState->Cube.Pos);
    DXResources->DrawTriangles(GameState->Cube.Vertices, 36);
    DXResources->SetTransformations(v3{});
    
    DXResources->SwapChain->Present(0, 0);
    RenderClock.PrintMiliSeconds("Render time:");
}
