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
AddToRenderBlocks(game_state *GameState, terrain_render_block *Block, v3 CameraP, v3 CamDir)
{
    const uint32 Resolution = Block->Resolution;
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
    
    block_hash *RemovedHash = GetHash(World->RenderHash, &BlockP);
    Assert(StoreIndex == RemovedHash->Index);
    Assert(!HashIsEmpty(RemovedHash));
    block_hash *LastHash = GetHash(World->RenderHash, &LastP);
    Assert(!HashIsEmpty(LastHash));
    
    LastHash->Index = StoreIndex;
    RemovedHash->Index = HASH_DELETED;
    World->DeletedRenderBlockCount++;
    
    // NOTE: If we are not deleting the last block
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
        block_hash *DensityHash = GetHash(World->DensityHash, Positions + PosIndex);
        Result = Result && !HashIsEmpty(DensityHash);
    }
    
    return Result;
}

internal bool32
DidBiggerMappedDensitiesLoad(world_density *World, world_block_pos *Positions, uint32 Count)
{
    bool32 Result = true;
    for(uint32 PosIndex = 0;
        PosIndex < Count;
        ++PosIndex)
    {
        world_block_pos *Pos = Positions + PosIndex;
        world_block_pos MappedPos = GetMappedPosition(World, Pos);
        block_hash *DensityHash = GetHash(World->DensityHash, &MappedPos);
        Result = Result && !HashIsEmpty(DensityHash);
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
        world_block_pos *Pos = Positions + PosIndex;
        block_hash *RenderHash = GetHash(World->RenderHash, Pos);
        block_hash *ZeroHash = GetZeroHash(World, Pos);
        if(HashIsEmpty(RenderHash) && HashIsEmpty(ZeroHash))
        {
            Result = false;
        }
    }
    
    return Result;
}

internal bool32
AreNeighboursAreMappedSameOrBigger(world_density *World, world_block_pos *Positions, uint32 Count, int32 MappedIndex)
{
    bool32 Result = true;
    for(uint32 PosIndex = 0;
        PosIndex < Count;
        ++PosIndex)
    {
        world_block_pos *Pos = Positions + PosIndex;
        block_hash *MappedHash = GetHash(World->ResolutionMapping, Pos);
        Assert(!HashIsEmpty(MappedHash));
        Result = Result && (MappedHash->Index >= MappedIndex);
    }
    
    return Result;
}

internal uint32
GetResolutionIndex(uint32 Resolution)
{
    uint32 Result;
    if(Resolution==4)
    {
        Result = 0;
    }
    else
    {
        Result = 1;
    }
    return Result;
}

inline bool32 
BlockWasRendered(world_density *World, world_block_pos *BlockP)
{
    bool32 Result = false;
    
    block_hash *RenderHash = GetHash(World->RenderHash, BlockP);
    block_hash *ZeroHash = GetZeroHash(World, BlockP);
    Result = !(HashIsEmpty(RenderHash) && HashIsEmpty(ZeroHash));
    
    return Result;
}

internal void
DeleteRenderedBlock(world_density *World, world_block_pos *BlockP)
{
    block_hash *RenderHash = GetHash(World->RenderHash, BlockP);
    block_hash *ZeroHash = GetZeroHash(World, BlockP);
    if(!HashIsEmpty(RenderHash))
    {
        DeleteRenderBlock(World, RenderHash->Index);
    }
    if(!HashIsEmpty(ZeroHash))
    {
        ZeroHash->Index = HASH_DELETED;
        World->ZeroBlockCount--;
    }
}

internal void
UpdateAndRenderGame(game_state *GameState, game_input *Input, camera *Camera, screen_info ScreenInfo)
{
    const uint32 ResolutionCount = 2;
    const uint32 FixedResolution[ResolutionCount] = {4, 2};

    world_density *World = &GameState->WorldDensity;
    if(GameState->Initialized == false)
    {
        GameState->Seed = 1000;
        GameState->WorldDensity.BlockSize = real32(TERRAIN_BLOCK_SIZE);
        SetSeed(&GameState->PerlinArray.Noise[0], GameState->Seed);
        SetSeed(&GameState->PerlinArray.Noise[1], GameState->Seed+1);
        SetSeed(&GameState->PerlinArray.Noise[2], GameState->Seed+2);
        InitBlockHash(World);
        InitZeroHash(World);
        
        GameState->Initialized = true;
    }
    GameState->RenderMode = Input->RenderMode;
    
    
    Camera->Update(Input, GameState->dtForFrame);
    
    v3 CameraP = Camera->GetPos();
    v3 CamDir = Camera->GetLookDirection();
    
    v2 MouseInPixel = v2{(real32)Input->MouseX, (real32)Input->MouseY};
    real32 WorldScreenSizeY = 2.0f*Tan(Camera->Fov/2.0f);
    real32 WorldScreenSizeX = WorldScreenSizeY*(real32)ScreenInfo.Width/ScreenInfo.Height;
    v3 UpDir = Camera->GetUpDirection();
    v3 RightDir = Normalize(Cross(UpDir, CamDir));
    v2 NormalizedMouse = MouseInPixel - v2{(real32)ScreenInfo.Width/2, (real32)ScreenInfo.Height/2};
    NormalizedMouse.X = NormalizedMouse.X / ScreenInfo.Width;
    NormalizedMouse.Y = -NormalizedMouse.Y / ScreenInfo.Height;
    
    v3 WorldMousePos = CameraP + (UpDir*NormalizedMouse.Y*WorldScreenSizeY) 
        + (RightDir*NormalizedMouse.X*WorldScreenSizeX);
        
    GameState->CameraOrigo = CameraP + Normalize(Cross(UpDir, RightDir));
    
    
    world_block_pos WorldCameraP[ResolutionCount] = {
        WorldPosFromV3(CameraP, FixedResolution[0]),
        WorldPosFromV3(CameraP, FixedResolution[1])
    };
    block_pos_array BlockPositionStore[ResolutionCount];
    CalculateBlockPositions(BlockPositionStore, WorldCameraP, RENDERED_BLOCK_RADIUS);
    CalculateBlockPositions(BlockPositionStore + 1, WorldCameraP + 1, RENDERED_BLOCK_RADIUS);
    
    win32_clock Clock;
    //
    // NOTE: Delete blocks that are too far from the camera
    //
    // TODO: Maybe we need to reinitialize the block hash, if there are too many deleted blocks?
    if(World->DensityBlockCount > (ArrayCount(World->DensityBlocks) * 7 / 8))
    {
        int32 LoadSpaceRadius = RENDERED_BLOCK_RADIUS;
        for(uint32 StoreIndex = 0; 
            StoreIndex < World->DensityBlockCount; 
            ++StoreIndex)
        {
            terrain_density_block *Block = World->DensityBlocks + StoreIndex;
            world_block_pos *BlockP = &Block->Pos;
            uint32 ResIndex = GetResolutionIndex(BlockP->Resolution);
            if(DoRectangleContains(WorldCameraP + ResIndex, LoadSpaceRadius, BlockP))
            {
                terrain_density_block *Last = World->DensityBlocks + (--World->DensityBlockCount);
                world_block_pos *LastP = &Last->Pos;
                
                block_hash *RemovedHash = GetHash(World->DensityHash, BlockP);
                Assert(!HashIsEmpty(RemovedHash));
                block_hash *LastHash = GetHash(World->DensityHash, LastP);
                Assert(!HashIsEmpty(LastHash));
                
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
            world_block_pos BlockP = WorldPosFromV3(Block->Pos, FixedResolution[0]);
            uint32 ResIndex = GetResolutionIndex(BlockP.Resolution);
            if(DoRectangleContains(WorldCameraP + ResIndex, LoadSpaceRadius, &BlockP))
            {
                DeleteRenderBlock(World, StoreIndex);
            }
        }
    }
    Clock.Reset();
    
    // TODO: Clear Resolution mapping too!
    
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
            world_block_pos *ZeroP = &Entry->Key;
            uint32 ResIndex = GetResolutionIndex(ZeroP->Resolution);
            if((Entry->Index == HASH_ZERO_BLOCK) && 
               DoRectangleContains(WorldCameraP + ResIndex, ZeroSpaceRadius, ZeroP))
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
            world_block_pos *BlockP = BlockPositions->Pos + PosIndex;
            block_hash *DensityHash = GetHash(World->DensityHash, BlockP);
            // NOTE: This can give back a deleted hash, if it had the same key as this block,
            // and it was already deleted once, and wasn't overwritten since.
            if(HashIsEmpty(DensityHash))
            {
                MaxBlocksToGenerateInFrame--;
                // NOTE: Initialize block
                uint32 BlockIndex = World->DensityBlockCount;
                terrain_density_block *DensityBlock = World->DensityBlocks + BlockIndex;
                
                GenerateDensityGrid(DensityBlock, &GameState->PerlinArray, BlockP);
                    
                DensityHash = WriteHash(World->DensityHash, BlockP, World->DensityBlockCount++);
                Assert(World->DensityBlockCount < ArrayCount(World->DensityBlocks));
                
                // NOTE: delete old render blocks, and mark them for rerender.
                /*const uint32 NeighbourCount = 27;
                world_block_pos NeighbourBlockPositions[NeighbourCount];
                GetNeighbourBlockPositions(NeighbourBlockPositions, BlockP);
                
                for(uint32 NeighbourIndex = 0;
                    NeighbourIndex < NeighbourCount;
                    NeighbourIndex++)
                {
                    world_block_pos *NeighbourBP = NeighbourBlockPositions + NeighbourIndex;
                    block_hash *NRenderHash = GetHash(World->RenderHash, NeighbourBP);
                    block_hash *NZeroHash = GetZeroHash(World, NeighbourBP);
                    if(!HashIsEmpty(NRenderHash))
                    {
                        DeleteRenderBlock(World, NRenderHash->Index);
                        BlocksToRerender[RerenderCount++] = NeighbourBP;
                    }
                    if(!HashIsEmpty(NZeroHash))
                    {
                        NZeroHash->Index = HASH_DELETED;
                        World->ZeroBlockCount--;
                        BlocksToRerender[RerenderCount++] = NeighbourBP;
                    }
                }*/
            }
        }
    }
    real64 TimeGenerateDensity = Clock.GetSecondsElapsed();
    Clock.Reset();
    
    uint32 MaxRenderBlocksToGenerateInFrame = 6;
#if 0
    if(Input->MouseRightButton)
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
                                block_hash *DensityHash = GetHash(World->DensityHash, Node.BlockP);
                                if(!HashIsEmpty(DensityHash))
                                {
                                    terrain_density_block *ActDensityBlock = World->DensityBlocks + DensityHash->Index;
                                    real32 GridVal = GetGrid(&ActDensityBlock->Grid, Node.X, Node.Y, Node.Z);
                                    SetGrid(&ActDensityBlock->Grid, Node.X, Node.Y, Node.Z, GridVal + 1.0f);
                                    
                                    // NOTE: Delete this and neighbouring render blocks                                    
                                    block_neighbours NPositions;
                                    GetNeighbourBlockPositions(NPositions, Node.BlockP);
                                    
                                    for(uint32 NeighbourIndex = 0;
                                        NeighbourIndex < ArrayCount(NPositions.Pos);
                                        NeighbourIndex++)
                                    {
                                        world_block_pos *BlockP = NPositions.Pos + NeighbourIndex;
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
    
    const uint32 BlockRenderMaxCount = 1000;
    world_block_pos BlocksToRender[BlockRenderMaxCount];
    uint32 RenderCount = 0;
    
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < ResolutionCount;
        ResolutionIndex++)
    {
        block_pos_array *BlockPositions = BlockPositionStore + ResolutionIndex;
        for(size_t BlockPosIndex = 0; 
            (BlockPosIndex < BlockPositions->Count) && (MaxRenderBlocksToGenerateInFrame > 0) ;
            ++BlockPosIndex)
        {
            world_block_pos *BlockP = BlockPositions->Pos + BlockPosIndex;
            block_hash *DensityHash = GetHash(World->DensityHash, BlockP);
            if(!HashIsEmpty(DensityHash))
            {                
                block_neighbours NPositions;
                GetNeighbourBlockPositions(&NPositions, BlockP);
                block_hash *ResHash = GetHash(World->ResolutionMapping, BlockP);
                
                if(HashIsEmpty(ResHash))
                {
                    // NOTE: This block wasnt even counted before
                    
                    // NOTE: check if bigger resolution is generated
                    // TODO: Check this recursivly for all resolutions?
                    world_block_pos BiggerP = GetBiggerResBlockPosition(BlockP);
                    block_hash *BiggerPHash = GetHash(World->ResolutionMapping, &BiggerP);
                    if(!HashIsEmpty(BiggerPHash))
                    {
                        WriteHash(World->ResolutionMapping, BlockP, (int32)BiggerPHash->Index);
                    }
                    else
                    {
                        Assert(BlockP->Resolution == FixedResolution[0]);
                        WriteHash(World->ResolutionMapping, BlockP, (int32)BlockP->Resolution);
                        // NOTE: Generate this block, if all its neighbours are generated
                        bool32 NeighboursGenerated = DidDensityBlocksLoaded(World, 
                                                                            NPositions.Pos, ArrayCount(NPositions.Pos));
                        
                        // Neighbours resoltuion should be consistent with this block!
                        if(NeighboursGenerated)
                        {
                            for(uint32 NIndex = 0;
                                NIndex < ArrayCount(NPositions.Pos);
                                NIndex++)
                            {
                                world_block_pos *NPos = NPositions.Pos + NIndex;
                                block_hash *NPosHash = GetHash(World->ResolutionMapping, NPos);
                                if(HashIsEmpty(NPosHash))
                                {
                                    // NOTE: Neighbours density is loaded, so just save its resolution
                                    Assert(NPos->Resolution == BlockP->Resolution);
                                    WriteHash(World->ResolutionMapping, NPos, NPos->Resolution);
                                }
                                else
                                {
                                    // TODO: Neighbour is already on different resolution?
                                    Assert(NPosHash->Index == (int32)NPos->Resolution);
                                }
                            }
                            
                            // NOTE: Add to render blocks here
                            MaxRenderBlocksToGenerateInFrame--;
                            BlocksToRender[RenderCount++] = *BlockP;
                        }
                    }
                }
                else
                {
                    // NOTE: Was already in ResolutionMapping,
                    // maybe it wasnt rendered though, because it didt had its neighbours at the time
                    // decide if we should upgrade it to the next resolution, together with its neighbours
                    
                    // NOTE: upgrade, if we want to generate more block in frame, and we find a block
                    // which was indexed on a bigger resolution ->
                    // this means that it is time to upgrade one to smaller resolution.
                    // This should apply to all the smaller res blocks in the parent block
                        
                    world_block_pos BiggerP = GetBiggerResBlockPosition(BlockP);
                    const uint32 SiblingCount = 8;
                    world_block_pos Siblings[SiblingCount];
                    GetLowerResBlockPositions(Siblings, &BiggerP);
                    
                    bool32 SiblingDensitiesLoaded = DidDensityBlocksLoaded(World, Siblings, SiblingCount);
                    if(ResHash->Index > (int32)BlockP->Resolution && SiblingDensitiesLoaded)
                    {
                        // NOTE: Rerender this block + neighbour blocks, to remain consistent
                        
                        int32 OldRes = ResHash->Index;
                        uint32 NewResIndex = GetResolutionIndex((uint32)OldRes) + 1;
                        Assert(NewResIndex < ResolutionCount);
                        int32 NewResolution = FixedResolution[NewResIndex];
                        
                        for(uint32 SiblingIndex = 0;
                            SiblingIndex < SiblingCount;
                            SiblingIndex++)
                        {
                            // NOTE: Ensure that every sibling is mapped right
                            world_block_pos *SiblingP = Siblings + SiblingIndex;
                            auto SiblingHash = GetHash(World->ResolutionMapping, SiblingP);
                            if(HashIsEmpty(SiblingHash))
                            {
                                WriteHash(World->ResolutionMapping, SiblingP, NewResolution);
                            }
                            else
                            {
                                Assert(SiblingHash->Index == OldRes);
                                SiblingHash->Index = NewResolution;
                            }
                        }
                        
                        for(uint32 SiblingIndex = 0;
                            SiblingIndex < SiblingCount;
                            SiblingIndex++)
                        {
                            world_block_pos *SiblingP = Siblings + SiblingIndex;
                            block_neighbours SiblingNeighbours;
                            GetNeighbourBlockPositions(&SiblingNeighbours, SiblingP);
                            for(uint32 NIndex = 0;
                                NIndex < ArrayCount(SiblingNeighbours.Pos);
                                NIndex++)
                            {
                                world_block_pos *NPos = SiblingNeighbours.Pos + NIndex;
                                block_hash *NHash = GetHash(World->ResolutionMapping, NPos);
                                if(HashIsEmpty(NHash))
                                {
                                    world_block_pos NBiggerP = GetBiggerResBlockPosition(NPos);
                                    NHash = GetHash(World->ResolutionMapping, &NBiggerP);
                                    // NOTE: We can presume that Smaller resolutions are 
                                    // not on the edge of the block position store
                                    Assert(!HashIsEmpty(NHash)); 
                                    NHash = WriteHash(World->ResolutionMapping, NPos, NHash->Index);
                                }
                                
                                if(BlockWasRendered(World, NPos) && (NHash->Index == ResHash->Index))
                                {
                                    MaxRenderBlocksToGenerateInFrame--;
                                    BlocksToRender[RenderCount++] = *NPos;
                                }
                            }
                            if(DidBiggerMappedDensitiesLoad(World, SiblingNeighbours.Pos, ArrayCount(SiblingNeighbours.Pos)))
                            {
                                MaxRenderBlocksToGenerateInFrame--;
                                BlocksToRender[RenderCount++] = *SiblingP;
                            }
                        }
                        
                        block_hash *BiggerHash = GetHash(World->ResolutionMapping, &BiggerP);
                        Assert(!HashIsEmpty(BiggerHash));
                        BiggerHash->Index = NewResolution;
                        DeleteRenderedBlock(World, &BiggerP);
                    }
                    else
                    {
                        if(!BlockWasRendered(World, BlockP) && (ResHash->Index == (int32)BlockP->Resolution))
                        {
                            // NOTE: Block was never rendered. should we render it now?
                            
                            // NOTE: Ensure that all neghbours are mapped
                            for(uint32 NIndex = 0;
                                NIndex < ArrayCount(NPositions.Pos);
                                NIndex++)
                            {
                                world_block_pos *NPos = NPositions.Pos + NIndex;
                                block_hash *NPosHash = GetHash(World->ResolutionMapping, NPos);
                                if(HashIsEmpty(NPosHash))
                                {
                                    auto NBiggerP = GetBiggerResBlockPosition(NPos);
                                    auto BigHash = GetHash(World->ResolutionMapping, &NBiggerP);
                                    if(!HashIsEmpty(BigHash))
                                    {
                                        NPosHash = WriteHash(World->ResolutionMapping, NPos, BigHash->Index);
                                    }
                                    else
                                    {
                                        // NOTE: Presume that this can only happen at the biggest resolution
                                        Assert(NPos->Resolution == FixedResolution[0]);
                                        NPosHash = WriteHash(World->ResolutionMapping, NPos, NPos->Resolution);
                                    }
                                }
                            }
                            
                            // TODO: Check if every neighbours are mapped same or bigger
                            world_block_pos DebugLowers[8];
                            GetLowerResBlockPositions(DebugLowers, BlockP);
                            if(AreNeighboursAreMappedSameOrBigger(World, 
                                    NPositions.Pos, ArrayCount(NPositions.Pos), ResHash->Index))
                            {
                                bool32 NeighboursGenerated = DidBiggerMappedDensitiesLoad(World, 
                                                                NPositions.Pos, ArrayCount(NPositions.Pos));
                                if(NeighboursGenerated)
                                {
                                    // NOTE: Add to render blocks here
                                    MaxRenderBlocksToGenerateInFrame--;
                                    BlocksToRender[RenderCount++] = *BlockP;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    win32_clock AvgClock;
    /*
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < ResolutionCount;
        ResolutionIndex++)
    {
        block_pos_array *BlockPositions = BlockPositionStore + ResolutionIndex;
        for(size_t BlockPosIndex = 0; 
            (BlockPosIndex < BlockPositions->Count) && (MaxRenderBlocksToGenerateInFrame > 0) ;
            ++BlockPosIndex)
        {
            world_block_pos *BlockP = BlockPositions->Pos + BlockPosIndex;
            block_hash *ZeroHash = GetZeroHash(World, BlockP);
            block_hash *RenderHash = GetHash(World->RenderHash, BlockP);
            // NOTE: This can give back a deleted hash, if it had the same key as this block,
            // and it was already deleted once, and wasn't overwritten since.
            if(HashIsEmpty(ZeroHash) && HashIsEmpty(RenderHash))
            {
                // NOTE: Check if lower resolution blocks are available
                const uint32 SameResBlockCount = 8;
                world_block_pos LowerBlockPositions[SameResBlockCount];
                GetLowerResBlockPositions(LowerBlockPositions, BlockP);
                
                bool32 LowerBlocksLoaded = DidDensityBlocksLoaded(World, LowerBlockPositions, SameResBlockCount);
                bool32 LowerBlocksAreInRange = true;
                if(LowerBlocksLoaded)
                {
                    for(uint32 PosIndex = 0;
                        (PosIndex < SameResBlockCount) && LowerBlocksAreInRange;
                        ++PosIndex)
                    {
                        Assert(ResolutionIndex < ResolutionCount-1);
                        block_pos_array *LowerBlockPosArray = BlockPositionStore + ResolutionIndex + 1;
                        world_block_pos *P = LowerBlockPositions + PosIndex;
                        bool32 IsInRange = false;
                        for(uint32 LowerBlockIndex = 0;
                            LowerBlockIndex < LowerBlockPosArray->Count;
                            LowerBlockIndex++)
                        {
                            world_block_pos *LP = LowerBlockPosArray->Pos + LowerBlockIndex;
                            if(WorldPosEquals(P, LP))
                            {
                                IsInRange = true;
                            }
                        }
                        if(!IsInRange)
                        {
                            LowerBlocksAreInRange = false;
                        }
                    }
                }
                if(!(LowerBlocksLoaded && LowerBlocksAreInRange))
                {
                    // NOTE: Neighbours include this block too
                    const uint32 NeighbourCount = 27;
                    world_block_pos NeighbourBlockPositions[NeighbourCount];
                    GetNeighbourBlockPositions(NeighbourBlockPositions, BlockP);
                    bool32 NeighboursGenerated = DidDensityBlocksLoaded(World, NeighbourBlockPositions, NeighbourCount);
                    
                    if(NeighboursGenerated)
                    {
                        // NOTE: Initialize block
                        // NOTE: DensityHash has been checked among neighbours to be valid
                        block_hash *DensityHash = GetHash(World->DensityHash, BlockP);
                        Assert(!HashIsEmpty(DensityHash));
                        terrain_density_block *DensityBlock = World->DensityBlocks + DensityHash->Index;
                        
                        AvgClock.Reset();
                        PoligoniseBlock(World, World->PoligonisedBlocks + World->PoligonisedBlockCount, 
                            DensityBlock);
                        CalculateAvarageTime(AvgClock, &GameState->AvgPoligoniseTime);
                        
                        MaxRenderBlocksToGenerateInFrame--;
                        
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
                    }
                }
            }
        }
    }*/
    
    // NOTE: Remove duplicates from BlockToRender
    for(uint32 RenderIndex = 0;
        RenderIndex < RenderCount;
        RenderIndex++)
    {
        world_block_pos *BlockP = BlocksToRender + RenderIndex;
        for(uint32 RenderInnerIndex = 0;
            RenderInnerIndex < RenderCount;
            RenderInnerIndex++)
        {
            // TODO: Start iinner loop from RenderIndex
            if(RenderIndex == RenderInnerIndex) 
            {
                continue;
            }
            world_block_pos *InnerBlockP = BlocksToRender + RenderInnerIndex;
            if(WorldPosEquals(BlockP, InnerBlockP))
            {
                world_block_pos LastP = BlocksToRender[--RenderCount];
                BlocksToRender[RenderInnerIndex] = LastP;
                --RenderInnerIndex;
            }
        }
    }
    
    // NOTE: These blocks may have been rendered once, but now they have to be generated again
    for(uint32 RenderIndex = 0;
        RenderIndex < RenderCount;
        RenderIndex++)
    {
        world_block_pos *BlockP = BlocksToRender + RenderIndex;
        /*block_hash *RenderHash = GetHash(World->RenderHash, BlockP);
        block_hash *ZeroHash = GetZeroHash(World, BlockP);
        if(!HashIsEmpty(RenderHash))
        {
            DeleteRenderBlock(World, RenderHash->Index);
        }
        if(!HashIsEmpty(ZeroHash))
        {
            ZeroHash->Index = HASH_DELETED;
            World->ZeroBlockCount--;
        }*/
        DeleteRenderedBlock(World, BlockP);
        
        Assert(HashIsEmpty(GetHash(World->RenderHash, BlockP)));
        Assert(HashIsEmpty(GetZeroHash(World, BlockP)));
        
        {
            AvgClock.Reset();
            PoligoniseBlock(World, World->PoligonisedBlocks + World->PoligonisedBlockCount, BlockP);
            CalculateAvarageTime(AvgClock, &GameState->AvgPoligoniseTime);
            
            MaxRenderBlocksToGenerateInFrame--;
            
            if(World->PoligonisedBlocks[World->PoligonisedBlockCount].VertexCount != 0)
            {
                WriteHash(World->RenderHash, BlockP, World->PoligonisedBlockCount++);
                Assert(World->PoligonisedBlockCount < ArrayCount(World->PoligonisedBlocks));
            }
            else
            {
                WriteZeroHash(World, BlockP);
                World->ZeroBlockCount++;
            }
        }
    }
    
    real64 TimeGenerateRender = Clock.GetSecondsElapsed();
    Clock.Reset();
    
    // TODO: Only render those renderblocks, that are continous with their neighbours
    // Consistnet render blocks? - how to find them?
    // it could happen, that a density have been loaded, and its neighbours too,
    // but a neghbour's neghbour havent loaded yet, so the neghbours render block is not generated,
    // but our first blocks render block is complete with its future geometry -> for now its uncompatible
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
            world_block_pos *BlockP = BlockPositions->Pos + PosIndex;
            block_hash *ZeroHash = GetZeroHash(World, BlockP);
            if(HashIsEmpty(ZeroHash))
            {
                block_hash *Hash = GetHash(World->RenderHash, BlockP);
                if(!HashIsEmpty(Hash))
                {
                    AddToRenderBlocks(GameState, World->PoligonisedBlocks + Hash->Index, CameraP, CamDir);
                }
            }
        }
    }
    real64 TimeAddToRender = Clock.GetSecondsElapsed();
    
    //
    // RENDER
    //
    
    Clock.Reset();
    
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
    DXResources->ObjectConstants.CameraDir = DirectX::XMFLOAT4(CamDir.X, CamDir.Y, CamDir.Z, 0.0f);
    DXResources->DrawBackground(BGVertices, 6);
    
    // NOTE: Render axis
    DXResources->SetDrawModeDefault();
    DXResources->DeviceContext->IASetInputLayout(DXResources->TerrainInputLayout);
    DXResources->DeviceContext->VSSetShader(DXResources->TerrainVS, 0, 0);
    DXResources->DeviceContext->PSSetShader(DXResources->LinePS, 0, 0);
    DXResources->DeviceContext->PSSetSamplers(0, 1, &DXResources->TexSamplerState);
    
    const real32 AxisSize = 256;
    const uint32 VertCount = 6;
    const v4 Red{1.0f, 0.0f, 0.0f, 1.0f};
    const v4 Green{0.0f, 1.0f, 0.0f, 1.0f}; 
    const v4 Blue{0.0f, 0.0f, 1.0f, 1.0f};
    const v3 Normal{0.0f, 1.0f, 0.0f};
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
    
    
    real64 TimeToRender = Clock.GetSecondsElapsed();
    
    
    // win32_printer::PerfPrint("Avg poligonise: %f", GameState->AvgPoligoniseTime * 1000.0);
    
    // win32_printer::PerfPrint("Generate density time: %f", TimeGenerateDensity * 1000.0);
    // win32_printer::PerfPrint("Right click time: %f", TimeRightClick * 1000.0);
    // win32_printer::PerfPrint("Generate render: %f", TimeGenerateRender * 1000.0);
    // win32_printer::PerfPrint("Add to render time: %f", TimeAddToRender * 1000.0);
    // win32_printer::PerfPrint("Render time: %f", TimeToRender * 1000.0);
    
}








