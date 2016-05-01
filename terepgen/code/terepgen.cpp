/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_random.cpp"
#include "terepgen_terrain.cpp"
#include "terepgen_dx_renderer.cpp"

internal int32 
getGridSize(const int32 n)
{
    return ((int32)(((4.0*n*n*n) + n*8.0 )/3.0 ) + (2*n*n) + 1);
}
    
// TODO: Should be based on resolution!
internal void 
CalculateBlockPositions(block_pos_array *PosArray, uint32 MaxArraySize, world_block_pos *CentralBlockPos, int32 Radius)
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
    
    Assert(PosArray->Count <= MaxArraySize);
}

internal void
AddToRenderBlocks(game_state *GameState, terrain_render_block *Block, v3 CameraP, v3 CamDir)
{
    const int32 Resolution = Block->Resolution;
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
AddCubeWireframe(cube_frame *Cube, v3 Pos, real32 Size, v4 Color)
{
    const v3 Corner0 = Pos + v3{-0.5f, -0.5f, -0.5f}*Size;
    const v3 Corner1 = Pos + v3{-0.5f, 0.5f, -0.5f}*Size;
    const v3 Corner2 = Pos + v3{0.5f, -0.5f, -0.5f}*Size;
    const v3 Corner3 = Pos + v3{0.5f, 0.5f, -0.5f}*Size;
    const v3 Corner4 = Pos + v3{-0.5f, -0.5f, 0.5f}*Size;
    const v3 Corner5 = Pos + v3{-0.5f, 0.5f, 0.5f}*Size;
    const v3 Corner6 = Pos + v3{0.5f, -0.5f, 0.5f}*Size;
    const v3 Corner7 = Pos + v3{0.5f, 0.5f, 0.5f}*Size;
    
    const v3 Normal = v3{0.0f, 0.0f, 1.0f};
    
    Cube->Vertices[0 ] = Vertex(Corner0, Normal, Color);
    Cube->Vertices[1 ] = Vertex(Corner1, Normal, Color);
    Cube->Vertices[2 ] = Vertex(Corner0, Normal, Color);
    Cube->Vertices[3 ] = Vertex(Corner2, Normal, Color);
    Cube->Vertices[4 ] = Vertex(Corner2, Normal, Color);
    Cube->Vertices[5 ] = Vertex(Corner3, Normal, Color);
    Cube->Vertices[6 ] = Vertex(Corner1, Normal, Color);
    Cube->Vertices[7 ] = Vertex(Corner3, Normal, Color);
                   
    Cube->Vertices[8 ] = Vertex(Corner4, Normal, Color);
    Cube->Vertices[9 ] = Vertex(Corner5, Normal, Color);
    Cube->Vertices[10] = Vertex(Corner4, Normal, Color);
    Cube->Vertices[11] = Vertex(Corner6, Normal, Color);
    Cube->Vertices[12] = Vertex(Corner6, Normal, Color);
    Cube->Vertices[13] = Vertex(Corner7, Normal, Color);
    Cube->Vertices[14] = Vertex(Corner5, Normal, Color);
    Cube->Vertices[15] = Vertex(Corner7, Normal, Color);
                   
    Cube->Vertices[16] = Vertex(Corner0, Normal, Color);
    Cube->Vertices[17] = Vertex(Corner4, Normal, Color);
    Cube->Vertices[18] = Vertex(Corner1, Normal, Color);
    Cube->Vertices[19] = Vertex(Corner5, Normal, Color);
    Cube->Vertices[20] = Vertex(Corner2, Normal, Color);
    Cube->Vertices[21] = Vertex(Corner6, Normal, Color);
    Cube->Vertices[22] = Vertex(Corner3, Normal, Color);
    Cube->Vertices[23] = Vertex(Corner7, Normal, Color);
}

internal void
AddCube(cube *Cube, v3 WorldMousePos, real32 Size,
        v4 ColorR, v4 ColorG, v4 ColorB)
{
    const v3 Corner0 = WorldMousePos + v3{-0.5f, -0.5f, -0.5f}*Size;
    const v3 Corner1 = WorldMousePos + v3{-0.5f, 0.5f, -0.5f}*Size;
    const v3 Corner2 = WorldMousePos + v3{0.5f, -0.5f, -0.5f}*Size;
    const v3 Corner3 = WorldMousePos + v3{0.5f, 0.5f, -0.5f}*Size;
    const v3 Corner4 = WorldMousePos + v3{-0.5f, -0.5f, 0.5f}*Size;
    const v3 Corner5 = WorldMousePos + v3{-0.5f, 0.5f, 0.5f}*Size;
    const v3 Corner6 = WorldMousePos + v3{0.5f, -0.5f, 0.5f}*Size;
    const v3 Corner7 = WorldMousePos + v3{0.5f, 0.5f, 0.5f}*Size;
    
    const v3 Normal0 = v3{0.0f, 0.0f, -1.0f}; //front
    const v3 Normal1 = v3{1.0f, 0.0f, 0.0f};  //right
    const v3 Normal2 = v3{0.0f, 0.0f, 1.0f};  //back
    const v3 Normal3 = v3{-1.0f, 0.0f, 0.0f}; //left
    const v3 Normal4 = v3{0.0f, -1.0f, 0.0f}; // down
    const v3 Normal5 = v3{0.0f, 1.0f, 0.0f};  //up
    
    Cube->Vertices[0] = Vertex(Corner0, Normal0, ColorB);
    Cube->Vertices[1] = Vertex(Corner1, Normal0, ColorB);
    Cube->Vertices[2] = Vertex(Corner2, Normal0, ColorB);
    Cube->Vertices[3] = Vertex(Corner2, Normal0, ColorB);
    Cube->Vertices[4] = Vertex(Corner1, Normal0, ColorB);
    Cube->Vertices[5] = Vertex(Corner3, Normal0, ColorB);
    
    Cube->Vertices[6] = Vertex(Corner2, Normal1, ColorR);
    Cube->Vertices[7] = Vertex(Corner3, Normal1, ColorR);
    Cube->Vertices[8] = Vertex(Corner6, Normal1, ColorR);
    Cube->Vertices[9] = Vertex(Corner6, Normal1, ColorR);
    Cube->Vertices[10] = Vertex(Corner3, Normal1, ColorR);
    Cube->Vertices[11] = Vertex(Corner7, Normal1, ColorR);
    
    Cube->Vertices[12] = Vertex(Corner6, Normal2, ColorB);
    Cube->Vertices[13] = Vertex(Corner7, Normal2, ColorB);
    Cube->Vertices[14] = Vertex(Corner4, Normal2, ColorB);
    Cube->Vertices[15] = Vertex(Corner4, Normal2, ColorB);
    Cube->Vertices[16] = Vertex(Corner7, Normal2, ColorB);
    Cube->Vertices[17] = Vertex(Corner5, Normal2, ColorB);
    
    Cube->Vertices[18] = Vertex(Corner4, Normal3, ColorR);
    Cube->Vertices[19] = Vertex(Corner5, Normal3, ColorR);
    Cube->Vertices[20] = Vertex(Corner0, Normal3, ColorR);
    Cube->Vertices[21] = Vertex(Corner0, Normal3, ColorR);
    Cube->Vertices[22] = Vertex(Corner5, Normal3, ColorR);
    Cube->Vertices[23] = Vertex(Corner1, Normal3, ColorR);
    
    Cube->Vertices[24] = Vertex(Corner4, Normal4, ColorG);
    Cube->Vertices[25] = Vertex(Corner0, Normal4, ColorG);
    Cube->Vertices[26] = Vertex(Corner6, Normal4, ColorG);
    Cube->Vertices[27] = Vertex(Corner6, Normal4, ColorG);
    Cube->Vertices[28] = Vertex(Corner0, Normal4, ColorG);
    Cube->Vertices[29] = Vertex(Corner2, Normal4, ColorG);
    
    Cube->Vertices[30] = Vertex(Corner1, Normal5, ColorG);
    Cube->Vertices[31] = Vertex(Corner5, Normal5, ColorG);
    Cube->Vertices[32] = Vertex(Corner3, Normal5, ColorG);
    Cube->Vertices[33] = Vertex(Corner3, Normal5, ColorG);
    Cube->Vertices[34] = Vertex(Corner5, Normal5, ColorG);
    Cube->Vertices[35] = Vertex(Corner7, Normal5, ColorG);
}

internal bool32
DoRectangleContains(world_block_pos *Center, int32 Radius, world_block_pos *P)
{
    bool32 Result = (Center->BlockX + Radius > P->BlockX) &&
                    (Center->BlockX - Radius < P->BlockX) &&
                    (Center->BlockY + Radius > P->BlockY) &&
                    (Center->BlockY - Radius < P->BlockY) &&
                    (Center->BlockZ + Radius > P->BlockZ) &&
                    (Center->BlockZ - Radius < P->BlockZ);
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
        block_hash *ResHash = GetHash(World->ResolutionMapping, Pos);
        if(ResHash->Index < (int32)Pos->Resolution)
        {
            // Block in position already mapped on smaller resolution
            return false;
        }
        world_block_pos MappedPos = GetBiggerMappedPosition(World, Pos);
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
    // TODO: Dont burn in values here!
    if(Resolution==8)
    {
        Result = 0;
    }
    else if(Resolution == 4)
    {
        Result = 1;
    }
    else
    {
        Result = 2;
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

internal int32
ManhattanDistance(world_block_pos *A, world_block_pos *B)
{
    int32 Result = 0;
    Result = Abs(A->BlockX - B->BlockX) + Abs(A->BlockY - B->BlockY) + Abs(A->BlockZ - B->BlockZ);
    return Result;
}

internal void 
DowngradeMapping(world_density *World, world_block_pos *BlockP, int32 MappingValue)
{
    uint32 ResIndex = GetResolutionIndex(BlockP->Resolution);
    if(ResIndex < RESOLUTION_COUNT)
    {
        lower_blocks LowerBlocks;
        GetLowerResBlockPositions(&LowerBlocks, BlockP);
        for(int32 LowerIndex = 0;
            LowerIndex < ArrayCount(LowerBlocks.Pos);
            LowerIndex++)
        {
            world_block_pos *LowerP = LowerBlocks.Pos + LowerIndex;
            DowngradeMapping(World, LowerP, MappingValue);
        }
        block_hash *ResHash = GetHash(World->ResolutionMapping, BlockP);
        if(!HashIsEmpty(ResHash))
        {
            Assert(ResHash->Index < MappingValue);
            ResHash->Index = MappingValue;
        }
        if(BlockWasRendered(World, BlockP))
        {
            DeleteRenderedBlock(World, BlockP);
        }
    }
}

internal void
UpdateAndRenderGame(game_state *GameState, game_input *Input, camera *Camera, screen_info ScreenInfo)
{
    const uint32 ResolutionCount = RESOLUTION_COUNT;
    const int32 FixedResolution[ResolutionCount] = {8, 4};
    
    int32 debugGS = getGridSize(13);
    
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
    
    CalculateBlockPositions(World->RenderPositionStore, 
                            ArrayCount(World->RenderPositionStore->Pos), 
                            WorldCameraP, RENDERED_BLOCK_RADIUS);
    CalculateBlockPositions(World->RenderPositionStore + 1, 
                            ArrayCount(World->RenderPositionStore->Pos), 
                            WorldCameraP + 1, RENDERED_BLOCK_RADIUS);
    
    CalculateBlockPositions((block_pos_array*)World->DensityPositionStore, 
                            ArrayCount(World->DensityPositionStore->Pos), 
                            WorldCameraP, DENSITY_BLOCK_RADIUS);
    CalculateBlockPositions((block_pos_array*)(World->DensityPositionStore + 1), 
                            ArrayCount(World->DensityPositionStore->Pos), 
                            WorldCameraP + 1, DENSITY_BLOCK_RADIUS);
    
    win32_clock Clock;
    //
    // NOTE: Delete blocks that are too far from the camera
    //
    // TODO: Maybe we need to reinitialize the block hash, if there are too many deleted blocks?
    if(World->DensityBlockCount > (ArrayCount(World->DensityBlocks) - 10))
    {
        int32 LoadSpaceRadius = DENSITY_BLOCK_RADIUS + 1;
        for(uint32 StoreIndex = 0; 
            StoreIndex < World->DensityBlockCount; 
            ++StoreIndex)
        {
            terrain_density_block *Block = World->DensityBlocks + StoreIndex;
            world_block_pos *BlockP = &Block->Pos;
            uint32 ResIndex = GetResolutionIndex(BlockP->Resolution);
            // TODO: Check manhattan distance, or need bigger hash and arrays
            if(ManhattanDistance(WorldCameraP + ResIndex, BlockP) > LoadSpaceRadius)
            // if(!DoRectangleContains(WorldCameraP + ResIndex, LoadSpaceRadius, BlockP))
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
            if(!DoRectangleContains(WorldCameraP + ResIndex, LoadSpaceRadius, &BlockP))
            {
                DeleteRenderBlock(World, StoreIndex);
            }
        }
    }
    Clock.Reset();
    
    if(World->BlockMappedCount > (ArrayCount(World->ResolutionMapping) * 7/8))
    {
        int32 LoadSpaceRadius = RENDERED_BLOCK_RADIUS;
        
        block_hash *NewMappingHash = new block_hash[BLOCK_HASH_SIZE];
        for(uint32 StoreIndex = 0; 
            StoreIndex < ArrayCount(World->ResolutionMapping); 
            ++StoreIndex)
        {
            NewMappingHash[StoreIndex] = World->ResolutionMapping[StoreIndex];
        }
        
        InitResolutionMapping(World);
        
        for(uint32 StoreIndex = 0; 
            StoreIndex < ArrayCount(World->ResolutionMapping); 
            ++StoreIndex)
        {
            block_hash *Hash = NewMappingHash + StoreIndex;
            world_block_pos *HashP = &Hash->Key;
            uint32 ResIndex = GetResolutionIndex(Hash->Key.Resolution);
            if(DoRectangleContains(WorldCameraP + ResIndex, LoadSpaceRadius, HashP) &&
               Hash->Index > 0)
            {
                WriteHash(World->ResolutionMapping, HashP, Hash->Index);
                World->BlockMappedCount++;
            }
        }
        delete[] NewMappingHash;
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
            world_block_pos *ZeroP = &Entry->Key;
            uint32 ResIndex = GetResolutionIndex(ZeroP->Resolution);
            if((Entry->Index == HASH_ZERO_BLOCK) && 
               !DoRectangleContains(WorldCameraP + ResIndex, ZeroSpaceRadius, ZeroP))
            {
                block_hash *ZeroHash = WriteZeroHash(World, ZeroP);
                World->ZeroBlockCount++;
            }
        }
    }
    //real64 DeleteZeroTime = Clock.GetSecondsElapsed();
    Clock.Reset();
        
    // NOTE: Generate density blocks
    int32 MaxDensityBlocksToGenerateInFrame = 3;
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < ResolutionCount;
        ResolutionIndex++)
    {
        density_block_pos_array *BlockPositions = World->DensityPositionStore + ResolutionIndex;
        for(size_t PosIndex = 0; 
            (PosIndex < BlockPositions->Count) && (MaxDensityBlocksToGenerateInFrame > 0) ;
            ++PosIndex)
        {
            world_block_pos *BlockP = BlockPositions->Pos + PosIndex;
            block_hash *DensityHash = GetHash(World->DensityHash, BlockP);
            // NOTE: This can give back a deleted hash, if it had the same key as this block,
            // and it was already deleted once, and wasn't overwritten since.
            if(HashIsEmpty(DensityHash))
            {
                MaxDensityBlocksToGenerateInFrame--;
                // NOTE: Initialize block
                uint32 BlockIndex = World->DensityBlockCount;
                terrain_density_block *DensityBlock = World->DensityBlocks + BlockIndex;
                
                GenerateDensityGrid(DensityBlock, &GameState->PerlinArray, BlockP);
                
                Assert(World->DensityBlockCount < ArrayCount(World->DensityBlocks));
                DensityHash = WriteHash(World->DensityHash, BlockP, World->DensityBlockCount++);
            }
        }
    }
    real64 TimeGenerateDensity = Clock.GetSecondsElapsed();
    Clock.Reset();
    
    int32 MaxRenderBlocksToGenerateInFrame = 6;
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
    
    // NOTE: 
    // If all the lower blocks of a block are generated, then the lower blocks can be mix rendered
    // If a mix rendered blocks neigbours are done generating their lower blocks, then they can be generated normally
    
    int32 LowestResUsed = FixedResolution[0];
    // NOTE: map all the blocks in range, if they arent already.
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < ResolutionCount;
        ResolutionIndex++)
    {
        density_block_pos_array *BlockPositions = World->DensityPositionStore + ResolutionIndex;
        for(size_t BlockPosIndex = 0; 
            (BlockPosIndex < BlockPositions->Count);
            ++BlockPosIndex)
        {
            // NOTE: Map this block, and its neighbours
            world_block_pos *BlockP = BlockPositions->Pos + BlockPosIndex;
            
            block_hash *ResHash = GetHash(World->ResolutionMapping, BlockP);
            if(HashIsEmpty(ResHash))
            {
                world_block_pos BiggerP = GetBiggerResBlockPosition(BlockP);
                if(BiggerP.Resolution <= FixedResolution[0])
                {
                    block_hash *BigPResHash = GetHash(World->ResolutionMapping, &BiggerP);
                    Assert(!HashIsEmpty(BigPResHash));
                    ResHash = WriteHash(World->ResolutionMapping, BlockP, (int32)BigPResHash->Index);
                    World->BlockMappedCount++;
                }
                else
                {
                    ResHash = WriteHash(World->ResolutionMapping, BlockP, FixedResolution[0]);
                    World->BlockMappedCount++;
                }
            }
            
            // NOTE: Could do downgrading here, if block is far away from the camera
            if(ResHash->Index < (int32)BlockP->Resolution)
            {
                lower_blocks LowerBlocks;
                GetLowerResBlockPositions(&LowerBlocks, BlockP);
                bool32 ShouldDowngrade = true;
                for(int32 LowerIndex = 0;
                    LowerIndex < ArrayCount(LowerBlocks.Pos);
                    LowerIndex++)
                {
                    world_block_pos *LowerP = LowerBlocks.Pos + LowerIndex;
                    int32 ResIndex = GetResolutionIndex((int32)LowerP->Resolution);
                    if(ManhattanDistance(WorldCameraP+ResIndex, LowerP) < RENDERED_BLOCK_RADIUS)
                    {
                        ShouldDowngrade = false;
                    }
                }
                
                if(ShouldDowngrade)
                {
                    DowngradeMapping(World, BlockP, BlockP->Resolution);
                }
            }
            Assert(!HashIsEmpty(ResHash));
            LowestResUsed = Min(LowestResUsed, ResHash->Index);
        }
    }
    
    bool32 EverybodyIsRenderedOnCorrectResolution = true;
    // NOTE: Select the next blocks that we can render.
    const uint32 BlockRenderMaxCount = 1000;
    world_block_pos BlocksToRender[BlockRenderMaxCount];
    uint32 RenderCount = 0;
    
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < ResolutionCount;
        ResolutionIndex++)
    {
        block_pos_array *BlockPositions = World->RenderPositionStore + ResolutionIndex;
        for(size_t BlockPosIndex = 0; 
            (BlockPosIndex < BlockPositions->Count) && (MaxRenderBlocksToGenerateInFrame > 0) ;
            ++BlockPosIndex)
        {
            world_block_pos *BlockP = BlockPositions->Pos + BlockPosIndex;
            block_hash *ResHash = GetHash(World->ResolutionMapping, BlockP);
            Assert(!HashIsEmpty(ResHash));
            bool32 AreOnSameRes = BlockP->Resolution == ResHash->Index;
            
            if(AreOnSameRes && !BlockWasRendered(World, BlockP))
            {
                EverybodyIsRenderedOnCorrectResolution = false;
                block_same_res_neighbours NPositions;
                GetNeighbourBlockPositionsOnSameRes(&NPositions, BlockP);
                bool32 DidLoad = DidBiggerMappedDensitiesLoad(World, NPositions.Pos, ArrayCount(NPositions.Pos));
                if(DidLoad)
                {
                    MaxRenderBlocksToGenerateInFrame--;
                    Assert(RenderCount < BlockRenderMaxCount);
                    BlocksToRender[RenderCount++] = *BlockP;
                }
            }
        }
    }
    
    // NOTE: if everbody is rendered on the resolution it should be rendered, 
    // then we can upgrade a block to a new resolution
    // TODO: Downgrading of resolutions, when we go away from a previously upgraded block
    if(EverybodyIsRenderedOnCorrectResolution && 
       (MaxRenderBlocksToGenerateInFrame >= 4))
    {    
        // NOTE: if everybody with a Resolution of LowestResUsed is rendered, we can upgrade LowestResUsed
        bool32 CanUpgradeLowestResolution = true;
        for(uint32 ResolutionIndex = 0;
            ResolutionIndex < ResolutionCount;
            ResolutionIndex++)
        {
            block_pos_array *BlockPositions = World->RenderPositionStore + ResolutionIndex;
            for(size_t BlockPosIndex = 0; 
                (BlockPosIndex < BlockPositions->Count);
                ++BlockPosIndex)
            {
                world_block_pos *BlockP = BlockPositions->Pos + BlockPosIndex;
                block_hash *ResHash = GetHash(World->ResolutionMapping, BlockP);
                Assert(!HashIsEmpty(ResHash));
                
                CanUpgradeLowestResolution = CanUpgradeLowestResolution &&
                    (((BlockP->Resolution == LowestResUsed) && BlockWasRendered(World, BlockP)) ||
                    ((BlockP->Resolution != LowestResUsed) && !BlockWasRendered(World, BlockP)));
            }
        }
        
        uint32 LowestResUsedIndex = GetResolutionIndex((uint32)LowestResUsed);
        if(CanUpgradeLowestResolution)
        {
            uint32 NewResIndex = LowestResUsedIndex+1;
            if(NewResIndex < ResolutionCount)
            {
                LowestResUsed = FixedResolution[NewResIndex];
                LowestResUsedIndex = NewResIndex;
            }
            // TODO: Else we are done, so dont upgrade anything for a while
        }
            
        block_pos_array *BlockPositions = World->RenderPositionStore + LowestResUsedIndex;
        for(size_t BlockPosIndex = 0; 
            (BlockPosIndex < BlockPositions->Count) && (MaxRenderBlocksToGenerateInFrame > 0) ;
            ++BlockPosIndex)
        {
            world_block_pos *BlockP = BlockPositions->Pos + BlockPosIndex;
            block_hash *ResHash = GetHash(World->ResolutionMapping, BlockP);
            Assert(!HashIsEmpty(ResHash));
            
            world_block_pos BiggerP = GetBiggerResBlockPosition(BlockP);
            
            lower_blocks Siblings;
            GetLowerResBlockPositions(&Siblings, &BiggerP);
            bool32 SiblingDensitiesLoaded = DidDensityBlocksLoaded(World, Siblings.Pos, ArrayCount(Siblings.Pos));
            if(ResHash->Index > LowestResUsed && SiblingDensitiesLoaded)
            {
                // NOTE: We can upgrade it, and its siblings to a smaller resolution
                for(uint32 SiblingIndex = 0;
                    SiblingIndex < ArrayCount(Siblings.Pos);
                    SiblingIndex++)
                {
                    // NOTE: Ensure that every sibling is mapped right
                    world_block_pos *SiblingP = Siblings.Pos + SiblingIndex;
                    auto SiblingHash = GetHash(World->ResolutionMapping, SiblingP);
                    Assert(!HashIsEmpty(SiblingHash));                    
                    Assert(SiblingHash->Index == LowestResUsed*2 || SiblingHash->Index == LowestResUsed);
                    SiblingHash->Index = LowestResUsed;
                }
                
                for(uint32 SiblingIndex = 0;
                    SiblingIndex < ArrayCount(Siblings.Pos);
                    SiblingIndex++)
                {
                    world_block_pos *SiblingP = Siblings.Pos + SiblingIndex;
                    block_same_res_neighbours SiblingNeighbours;
                    GetNeighbourBlockPositionsOnSameRes(&SiblingNeighbours, SiblingP);
                    for(uint32 NIndex = 0;
                        NIndex < ArrayCount(SiblingNeighbours.Pos);
                        NIndex++)
                    {
                        world_block_pos *NPos = SiblingNeighbours.Pos + NIndex;
                        block_hash *NHash = GetHash(World->ResolutionMapping, NPos);
                        Assert(!HashIsEmpty(NHash));
                        
                        //NOTE: Have to examine the densities of neighbour's neighbours too
                        block_same_res_neighbours SiblingNeighbourNeighbours;
                        GetNeighbourBlockPositionsOnSameRes(&SiblingNeighbourNeighbours, NPos);
                        bool32 NNsLoaded = DidBiggerMappedDensitiesLoad(World, 
                            SiblingNeighbourNeighbours.Pos, ArrayCount(SiblingNeighbourNeighbours.Pos));
                        if(NNsLoaded && (NHash->Index == LowestResUsed))
                        {
                            MaxRenderBlocksToGenerateInFrame--;
                            Assert(RenderCount < BlockRenderMaxCount);
                            BlocksToRender[RenderCount++] = *NPos;
                        }
                    }
                    if(DidBiggerMappedDensitiesLoad(World, SiblingNeighbours.Pos, ArrayCount(SiblingNeighbours.Pos)))
                    {
                        MaxRenderBlocksToGenerateInFrame--;
                        Assert(RenderCount < BlockRenderMaxCount);
                        BlocksToRender[RenderCount++] = *SiblingP;
                    }
                }
                
                DeleteRenderedBlock(World, &BiggerP);
                while(BiggerP.Resolution <= FixedResolution[0])
                {
                    block_hash *BiggerHash = GetHash(World->ResolutionMapping, &BiggerP);
                    Assert(!HashIsEmpty(BiggerHash));
                    BiggerHash->Index = LowestResUsed;
                    BiggerP = GetBiggerResBlockPosition(&BiggerP);
                }
            }
        }
    }
    
    // NOTE: Remove duplicates from BlocksToRender
    for(uint32 RenderIndex = 0;
        RenderIndex < RenderCount;
        RenderIndex++)
    {
        world_block_pos *BlockP = BlocksToRender + RenderIndex;
        for(uint32 RenderInnerIndex = 0;
            RenderInnerIndex < RenderCount;
            RenderInnerIndex++)
        {
            // TODO: Start inner loop from RenderIndex
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
    
    win32_clock AvgClock;
    // NOTE: These blocks may have been rendered once, but now they have to be generated again
    for(uint32 RenderIndex = 0;
        RenderIndex < RenderCount;
        RenderIndex++)
    {
        world_block_pos *BlockP = BlocksToRender + RenderIndex;
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
    
    GameState->RenderBlockCount = 0;
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < ResolutionCount;
        ResolutionIndex++)
    {
        block_pos_array *BlockPositions = World->RenderPositionStore + ResolutionIndex;
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
    DXResources->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
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
    
    // NOTE: Draw cube
    DXResources->DeviceContext->PSSetShader(DXResources->LinePS, 0, 0);
    
    DXResources->SetTransformations(GameState->CubePos);
    DXResources->DrawTriangles(GameState->Cube.Vertices, CubeVertexCount);
    DXResources->SetTransformations(v3{});
    
    
    // NOTE: Draw debug resolution blocks
    GameState->ResCubeCount = 0;
    DXResources->SetDrawModeWireframe();
    DXResources->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < ResolutionCount;
        ResolutionIndex++)
    {
        density_block_pos_array *BlockPositions = World->DensityPositionStore + ResolutionIndex;
        for(size_t BlockPosIndex = 0; 
            (BlockPosIndex < BlockPositions->Count);
            ++BlockPosIndex)
        {
            world_block_pos *BlockP = BlockPositions->Pos + BlockPosIndex;            
            block_hash *ResHash = GetHash(World->ResolutionMapping, BlockP);
            if(ResHash->Index == BlockP->Resolution)
            {
                v3 RenderPos = V3FromWorldPos(*BlockP);
                real32 Size = (RENDER_SPACE_UNIT*TERRAIN_BLOCK_SIZE*BlockP->Resolution);
                RenderPos += v3{0.5f, 0.5f, 0.5f}*Size;
                
                v4 Color{1.0f, 1.0f, 0.0f, 1.0f};
                if(BlockP->Resolution > 4)
                {
                    Color = v4{0.0f, 0.3f, 0.0f, 1.0f};
                }
                
                cube_frame *BlockCube = GameState->ResolutionCubes + GameState->ResCubeCount++;
                AddCubeWireframe(BlockCube, RenderPos, Size-0.1f, Color);
            }
        }
    }
    
    DXResources->DrawTriangles(GameState->ResolutionCubes->Vertices, CubeFrameVertexCount*GameState->ResCubeCount);
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








