/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_random.cpp"
#include "terepgen_units.cpp"
#include "terepgen_hash.cpp"
#include "terepgen_resolutions.cpp"
#include "terepgen_terrain.cpp"
#include "terepgen_dx_renderer.cpp"

internal int32 
getGridSize(const int32 n)
{
    return ((int32)(((4.0*n*n*n) + n*8.0 )/3.0 ) + (2*n*n) + 1);
}

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
    const int32 Resolution = Block->WPos.Resolution;
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
        bool32 AlreadyHaveBlock = false;
        for(uint32 BlockIndex = 0; 
            BlockIndex < GameState->RenderBlockCount; 
            BlockIndex++)
        {
            terrain_render_block *RBlock = GameState->RenderBlocks[BlockIndex];
            if(WorldPosEquals(&RBlock->WPos, &Block->WPos))
            {
                AlreadyHaveBlock = true;
            }
        }
        if(!AlreadyHaveBlock)
        {
            GameState->RenderBlocks[GameState->RenderBlockCount++] = Block;
            Assert(GameState->RenderBlockCount < ArrayCount(GameState->RenderBlocks));
        }
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
        if(HashIsEmpty(ResHash))
        {
            ResHash = MapBlockPositionAfterParent(World, Pos);
        }
        Assert(!HashIsEmpty(ResHash));
        Assert(ResHash->Index >= Pos->Resolution);
        
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
    world_block_pos BlockP = Block->WPos;
    world_block_pos LastP = Last->WPos;
    
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
QueueBlockToRender(world_density *World, world_block_pos *BlockP, 
    int32 *BlocksToGenerate, world_block_pos* BlocksToRender, uint32 *RenderCount, uint32 BlockRenderMaxCount)
{
    block_lower_neighbours Neighbours;
    GetNeighbourBlockPositionsOnLowerRes(&Neighbours, BlockP);
    bool32 DidLoad = DidBiggerMappedDensitiesLoad(World, Neighbours.Pos, ArrayCount(Neighbours.Pos));
    if(DidLoad)
    {
        (*BlocksToGenerate)--;
        Assert(*RenderCount < BlockRenderMaxCount);
        BlocksToRender[(*RenderCount)++] = *BlockP;
    }
}

internal uint32
StringLength(char *Text)
{
    uint32 Length = 0;
    char *P = Text;
    while(*P != '\0')
    {
        Length++; P++;
    }
    return Length;
}

typedef HANDLE FileHandle;

internal FileHandle
OpenFileForBlocks(game_state *GameState, char *FileName)
{
    FileHandle Handle = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(Handle == INVALID_HANDLE_VALUE)
    {
        uint32 Error = GetLastError();
        if(Error == ERROR_FILE_NOT_FOUND)
        {
            // NOTE: The file didn't exist before, so now we create its header
            Handle = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            Assert(Handle != INVALID_HANDLE_VALUE);
            
            uint32 *Data = &GameState->GameID;
            uint32 Length = 4;
            uint32 BytesWritten;
            WriteFile(Handle, Data, Length, (LPDWORD)&BytesWritten, NULL);
            SetFilePointer(Handle, 0, NULL, FILE_BEGIN);
        }
    }
    return Handle;
}

internal void
SaveBlockToFile(game_state *GameState, terrain_density_block *Block)
{
    char *FileName = "dynamicStore.txt";
    FileHandle Handle = OpenFileForBlocks(GameState, FileName);
    
    SetFilePointer(Handle, 0, NULL, FILE_BEGIN);
    uint32 BytesRead;
    
    // NOTE: Read header
    char HeaderValue[4];
    uint32 HeaderLength = 4;
    ReadFile(Handle, HeaderValue, HeaderLength, (LPDWORD)&BytesRead, NULL);
    Assert(HeaderLength == BytesRead);
    
    uint32 GameID = *(uint32*)HeaderValue;
    Assert(GameID == GameState->GameID);
    
    //NOTE: Read blocks until we find the one we need
    bool32 NotFound = true;
    bool32 EndOfFile = false;
    terrain_density_block ReadBlock;
    const uint32 BlockSizeInBytes = sizeof(terrain_density_block);
    while(NotFound && !EndOfFile)
    {
        ReadFile(Handle, &ReadBlock, BlockSizeInBytes, (LPDWORD)&BytesRead, NULL);
        EndOfFile = (BytesRead == 0);
        Assert(BytesRead == BlockSizeInBytes || EndOfFile);
        if(WorldPosEquals(&ReadBlock.Pos, &Block->Pos))
        {
            NotFound = false;
            // NOTE: Set the file pointer to the begging of the block, to overwrite it
            int32 Offset = sizeof(terrain_density_block);
            SetFilePointer(Handle, -Offset, 0, FILE_CURRENT);
        }
    }
    
    //NOTE: Write Block
    uint32 BytesWritten;
    WriteFile(Handle, Block, BlockSizeInBytes, (LPDWORD)&BytesWritten, NULL);
    Assert(BytesWritten == BlockSizeInBytes);
    
    CloseHandle(Handle);
}


internal void
LoadBlockFromFile(game_state *GameState, terrain_density_block *Block, world_block_pos *BlockP)
{
    char *FileName = "dynamicStore.txt";
    FileHandle Handle = OpenFileForBlocks(GameState, FileName);
    
    SetFilePointer(Handle, 0, NULL, FILE_BEGIN);
    uint32 BytesRead;
    
    // NOTE: Read header
    char HeaderValue[4];
    uint32 HeaderLength = 4;
    ReadFile(Handle, HeaderValue, HeaderLength, (LPDWORD)&BytesRead, NULL);
    Assert(HeaderLength == BytesRead);
    
    uint32 GameID = *(uint32*)HeaderValue;
    Assert(GameID == GameState->GameID);
    
    //NOTE: Read blocks until we find the one we need
    bool32 NotFound = true;
    bool32 EndOfFile = false;
    const uint32 BlockSizeInBytes = sizeof(terrain_density_block);
    while(NotFound && !EndOfFile)
    {
        ReadFile(Handle, Block, BlockSizeInBytes, (LPDWORD)&BytesRead, NULL);
        EndOfFile = (BytesRead == 0);
        Assert(BytesRead == BlockSizeInBytes || EndOfFile);
        if(WorldPosEquals(&Block->Pos, BlockP))
        {
            NotFound = false;
        }
    }
    
    CloseHandle(Handle);
}

internal void
TestFileWriting(game_state *GameState)
{
    world_density *World = &GameState->WorldDensity;
    
    world_block_pos DebugPos1{-1, 0, 1, 8};
    world_block_pos DebugPos2{-2, 1, 0, 8};
    world_block_pos DebugPos3{-3, -1, 1, 8};
    block_hash *DynamicHash = CreateNewDynamicBlock(World, &DebugPos1);
    terrain_density_block *DynB1 = World->DynamicBlocks + DynamicHash->Index;
    SetGrid(&DynB1->Grid, 0, 0, 1, 3.5f);
    SetGrid(&DynB1->Grid, 0, 0, 2, 5.5f);
    SetGrid(&DynB1->Grid, 0, 0, 7, 2.39f);
    SetGrid(&DynB1->Grid, 0, 1, 0, 10.312f);
    SetGrid(&DynB1->Grid, 1, 0, 0, 11.312f);
    SaveBlockToFile(GameState, DynB1);
    
    DynamicHash = CreateNewDynamicBlock(World, &DebugPos2);
    terrain_density_block *DynB2 = World->DynamicBlocks + DynamicHash->Index;
    SetGrid(&DynB2->Grid, 0, 0, 0, 1.111f);
    SetGrid(&DynB2->Grid, 0, 0, 2, 5.41f);
    SetGrid(&DynB2->Grid, 0, 0, 3, 1.169f);
    SaveBlockToFile(GameState, DynB2);
    
    DynamicHash = CreateNewDynamicBlock(World, &DebugPos3);
    terrain_density_block *DynB3 = World->DynamicBlocks + DynamicHash->Index;
    SetGrid(&DynB3->Grid, 0, 0, 0, 3.5f);
    SetGrid(&DynB3->Grid, 0, 0, 2, 5.5f);
    SetGrid(&DynB3->Grid, 0, 0, 7, 2.39f);
    SetGrid(&DynB3->Grid, 0, 1, 0, 10.312f);
    SetGrid(&DynB3->Grid, 1, 0, 0, 11.312f);
    SaveBlockToFile(GameState, DynB3);
    
    terrain_density_block ReferenceB;
    LoadBlockFromFile(GameState, &ReferenceB, &DebugPos2);
    
    LoadBlockFromFile(GameState, &ReferenceB, &DebugPos1);
    
    int asd = 7;
}

internal void
UpdateAndRenderGame(game_state *GameState, game_input *Input, camera *Camera, screen_info ScreenInfo)
{
    const uint32 ResolutionCount = RESOLUTION_COUNT;
    const int32 FixedResolution[ResolutionCount] = {8, 4, 2};
    const int32 StoreResolutionCount = ResolutionCount-1;
    
    int32 debugGS = getGridSize(13);
    
    world_density *World = &GameState->WorldDensity;
    if(GameState->Initialized == false)
    {
        GameState->GameID = 321421;
        GameState->MaxResolutionToRender = ResolutionCount-2;
        GameState->Seed = 1000;
        GameState->WorldDensity.BlockSize = real32(TERRAIN_BLOCK_SIZE);
        SetSeed(&GameState->PerlinArray.Noise[0], GameState->Seed);
        SetSeed(&GameState->PerlinArray.Noise[1], GameState->Seed+1);
        SetSeed(&GameState->PerlinArray.Noise[2], GameState->Seed+2);
        InitBlockHash(World);
        InitZeroHash(World);
        
        GameState->Initialized = true;
        
        TestFileWriting(GameState);
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
        WorldPosFromV3(CameraP, FixedResolution[1]),
        WorldPosFromV3(CameraP, FixedResolution[2])
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
    if(World->DensityBlockCount > (ArrayCount(World->DensityBlocks) - 100))
    {
        int32 LoadSpaceRadius = DENSITY_BLOCK_RADIUS + 1;
        for(uint32 StoreIndex = 0; 
            StoreIndex < World->DensityBlockCount; 
            ++StoreIndex)
        {
            terrain_density_block *Block = World->DensityBlocks + StoreIndex;
            world_block_pos *BlockP = &Block->Pos;
            uint32 ResIndex = GetResolutionIndex(BlockP->Resolution);
            // TODO: Maybe instead of checking manhattan distance, we need bigger hash and arrays
            if(ManhattanDistance(WorldCameraP + ResIndex, BlockP) > LoadSpaceRadius)
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
    
    if(World->DynamicBlockCount > (ArrayCount(World->DynamicBlocks) - 7000))
    {
        win32_printer::DebugPrint("Clearing Dynamic Blocks! count: %d", World->DynamicBlockCount);
        int32 LoadSpaceRadius = DENSITY_BLOCK_RADIUS + 1;
        for(uint32 StoreIndex = 0; 
            StoreIndex < World->DynamicBlockCount; 
            ++StoreIndex)
        {
            terrain_density_block *Block = World->DynamicBlocks + StoreIndex;
            world_block_pos *BlockP = &Block->Pos;
            uint32 ResIndex = GetResolutionIndex(BlockP->Resolution);
            // NOTE: Check manhattan distance, or need bigger hash and arrays
            if(ManhattanDistance(WorldCameraP + ResIndex, BlockP) > LoadSpaceRadius)
            {
                // TODO: Save to file!
                terrain_density_block *Last = World->DynamicBlocks + (--World->DynamicBlockCount);
                world_block_pos *LastP = &Last->Pos;
                
                block_hash *RemovedHash = GetHash(World->DynamicHash, BlockP);
                Assert(!HashIsEmpty(RemovedHash));
                block_hash *LastHash = GetHash(World->DynamicHash, LastP);
                Assert(!HashIsEmpty(LastHash));
                
                LastHash->Index = StoreIndex;
                RemovedHash->Index = HASH_DELETED;
                World->DeletedDynamicBlockCount++;
                
                if(StoreIndex != (int32)World->DynamicBlockCount)
                {
                    *Block = *Last;
                }
            }
        }
    }
    Clock.Reset();
    
    if(World->PoligonisedBlockCount > (ArrayCount(World->PoligonisedBlocks) - 1300))
    {
        int32 LoadSpaceRadius = RENDERED_BLOCK_RADIUS + 2;
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
        int32 LoadSpaceRadius = DENSITY_BLOCK_RADIUS + 1;
        
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
        ResolutionIndex < StoreResolutionCount;
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
    const uint32 BlockRenderMaxCount = 4000;
    world_block_pos BlocksToRender[BlockRenderMaxCount];
    uint32 RenderCount = 0;

    if(Input->MouseRightButton)
    {
        v3 RayDirection = Normalize(WorldMousePos - GameState->CameraOrigo);
        for(real32 RayLength = 0.5f; 
            RayLength < 2000.0f; 
            RayLength += 0.5f)
        {
            v3 CheckPos = GameState->CameraOrigo + (RayLength*RayDirection);
            real32 PosValue = GetWorldGridValueFromV3(World, CheckPos, FixedResolution[0]);
            if(PosValue < DENSITY_ISO_LEVEL)
            {
                real32 SphereRadius = 30.0f;
                v3 StartBlockRP = CheckPos - v3{SphereRadius, SphereRadius, SphereRadius};
                v3 EndBlockRP = CheckPos + v3{SphereRadius, SphereRadius, SphereRadius};
                block_node StartNode = ConvertRenderPosToBlockNode(StartBlockRP, FixedResolution[0]);
                block_node EndNode = ConvertRenderPosToBlockNode(EndBlockRP, FixedResolution[0]);
                
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
                            real32 DistanceFromClick = Length(Diff);
                            if(DistanceFromClick < SphereRadius)
                            {
                                // block_hash *DynamicHash = GetHash(World->DensityHash, &Node.BlockP);
                                block_hash *DynamicHash = GetHash(World->DynamicHash, &Node.BlockP);
                                if(HashIsEmpty(DynamicHash))
                                {
                                    DynamicHash = CreateNewDynamicBlock(World, &Node.BlockP);
                                }
                                
                                // terrain_density_block *ActDynamicBlock = World->DensityBlocks + DynamicHash->Index;
                                terrain_density_block *ActDynamicBlock = World->DynamicBlocks + DynamicHash->Index;
                                real32 GridVal = GetGrid(&ActDynamicBlock->Grid, Node.X, Node.Y, Node.Z);
                                SetGrid(&ActDynamicBlock->Grid, Node.X, Node.Y, Node.Z, GridVal + 1.0f);
                                
                                block_same_res_neighbours Neighbours;
                                GetNeighbourBlockPositionsOnSameRes(&Neighbours, &Node.BlockP);
                                for(uint32 NIndex = 0;
                                    NIndex < ArrayCount(Neighbours.Pos);
                                    NIndex++)
                                {
                                    world_block_pos *NPos = Neighbours.Pos + NIndex;
                                    bool32 AlreadyHave = false;
                                    for(uint32 RenderIndex = 0;
                                        RenderIndex < RenderCount;
                                        RenderIndex++)
                                    {
                                        world_block_pos *RenderP = BlocksToRender + RenderIndex;
                                        if(WorldPosEquals(RenderP, NPos))
                                        {
                                            AlreadyHave = true;
                                        }
                                    }
                                    if(!AlreadyHave)
                                    {
                                        QueueBlockToRender(World, NPos, 
                                            &MaxRenderBlocksToGenerateInFrame, 
                                            BlocksToRender, &RenderCount, BlockRenderMaxCount);
                                        BlocksTouched++;
                                    }
                                }
                            }
                        }
                    }
                }
                AddCube(&GameState->Cube, CheckPos, 1.0f, 
                        v4{1.0f, 0.0f, 0.0f, 1.0f}, 
                        v4{0.0f, 1.0f, 0.0f, 1.0f}, 
                        v4{0.0f, 0.0f, 1.0f, 1.0f});
                break;
            }
        }
        real64 TimeRightClickInner = Clock.GetSecondsElapsed();
        win32_printer::DebugPrint("Right click time: %f", TimeRightClickInner * 1000.0);
    }

    real64 TimeRightClick = Clock.GetSecondsElapsed();
    Clock.Reset();
    
    // NOTE: 
    // If all the lower blocks of a block are generated, then the lower blocks can be mix rendered
    // If a mix rendered blocks neigbours are done generating their lower blocks, then they can be generated normally
    
    int32 DeleteRenderBlockCount = 0;
    world_block_pos DeleteRenderBlockQueue[1000];
    
    int32 LowestResUsed = FixedResolution[0];
    // NOTE: map all the blocks in range, if they arent already.
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < StoreResolutionCount;
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
                    ResHash = MapBlockPosition(World, BlockP, BigPResHash->Index);
                }
                else
                {
                    ResHash = MapBlockPosition(World, BlockP, FixedResolution[0]);
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
                    int32 ResIndex = GetResolutionIndex(LowerP->Resolution);
                    if(ManhattanDistance(WorldCameraP+ResIndex, LowerP) <= RENDERED_BLOCK_RADIUS)
                    {
                        ShouldDowngrade = false;
                    }
                }
                
                if(ShouldDowngrade)
                {
                    DowngradeMapping(World, BlockP, BlockP->Resolution, DeleteRenderBlockQueue, &DeleteRenderBlockCount);
                }
            }
            Assert(!HashIsEmpty(ResHash));
            LowestResUsed = Min(LowestResUsed, ResHash->Index);
        }
    }
    
    bool32 EverybodyIsRenderedOnCorrectResolution = true;
    // NOTE: Select the next blocks that we can render.
    
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < StoreResolutionCount;
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
                
                QueueBlockToRender(World, BlockP, 
                    &MaxRenderBlocksToGenerateInFrame, 
                    BlocksToRender, &RenderCount, BlockRenderMaxCount);
            }
        }
    }
    
    // NOTE: if everbody is rendered on the resolution it should be rendered, 
    // then we can upgrade a block to a new resolution
    if(EverybodyIsRenderedOnCorrectResolution && 
       (MaxRenderBlocksToGenerateInFrame >= 4))
    {    
        // NOTE: if everybody with a Resolution of LowestResUsed is rendered, we can upgrade LowestResUsed
        bool32 CanUpgradeLowestResolution = true;
        for(uint32 ResolutionIndex = 0;
            ResolutionIndex < StoreResolutionCount;
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
            if(NewResIndex < GameState->MaxResolutionToRender)
            {
                LowestResUsed = FixedResolution[NewResIndex];
                LowestResUsedIndex = NewResIndex;
            }
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
                    UpdateLowerBlocksMapping(World, SiblingP, LowestResUsed);
                }
                
                DeleteRenderedBlock(World, &BiggerP);
                while(BiggerP.Resolution <= FixedResolution[0])
                {
                    block_hash *BiggerHash = GetHash(World->ResolutionMapping, &BiggerP);
                    Assert(!HashIsEmpty(BiggerHash));
                    BiggerHash->Index = LowestResUsed;
                    BiggerP = GetBiggerResBlockPosition(&BiggerP);
                }
                
                // NOTE: We need to render the siblings
                for(uint32 SiblingIndex = 0;
                    SiblingIndex < ArrayCount(Siblings.Pos);
                    SiblingIndex++)
                {
                    world_block_pos *SiblingP = Siblings.Pos + SiblingIndex;
                    // NOTE: We want to rerender its neighbours too if we can
                    block_lower_neighbours SiblingNeighbours;
                    GetNeighbourBlockPositionsOnLowerRes(&SiblingNeighbours, SiblingP);
                    for(uint32 NIndex = 0;
                        NIndex < ArrayCount(SiblingNeighbours.Pos);
                        NIndex++)
                    {
                        world_block_pos *NPos = SiblingNeighbours.Pos + NIndex;
                        block_hash *NHash = GetHash(World->ResolutionMapping, NPos);                        
                        if(HashIsEmpty(NHash))
                        {
                            NHash = MapBlockPositionAfterParent(World, NPos);
                        }
                        Assert(!HashIsEmpty(NHash));
                        
                        //NOTE: Have to examine the densities of neighbour's neighbours too
                        world_block_pos MappedNPos = GetBiggerMappedPosition(World, NPos);
                        Assert(MappedNPos.Resolution == NHash->Index);
                        QueueBlockToRender(World, &MappedNPos, 
                            &MaxRenderBlocksToGenerateInFrame, 
                            BlocksToRender, &RenderCount, BlockRenderMaxCount);
                    }
                    QueueBlockToRender(World, SiblingP,
                        &MaxRenderBlocksToGenerateInFrame, 
                        BlocksToRender, &RenderCount, BlockRenderMaxCount);
                }
            }
        }
    }
    
    for(int32 DelIndex = 0; DelIndex < DeleteRenderBlockCount; DelIndex++)
    {
        world_block_pos *BlockP = DeleteRenderBlockQueue + DelIndex;
        DeleteRenderedBlock(World, BlockP);
    }
    
    // NOTE: Remove duplicates from BlocksToRender
    for(uint32 RenderIndex = 0;
        RenderIndex < RenderCount;
        RenderIndex++)
    {
        world_block_pos *BlockP = BlocksToRender + RenderIndex;
        for(uint32 RenderInnerIndex = RenderIndex+1;
            RenderInnerIndex < RenderCount;
            RenderInnerIndex++)
        {
            world_block_pos *InnerBlockP = BlocksToRender + RenderInnerIndex;
            if(WorldPosEquals(BlockP, InnerBlockP))
            {
                world_block_pos LastP = BlocksToRender[--RenderCount];
                BlocksToRender[RenderInnerIndex] = LastP;
                --RenderInnerIndex;
            }
        }
    }
    
    // TODO: Before rendering, check for every blck that their neighbours are mapped right 
    // and their densities are loaded.
    
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
        ResolutionIndex < StoreResolutionCount;
        ResolutionIndex++)
    {
        block_pos_array *BlockPositions = World->RenderPositionStore + ResolutionIndex;
        for(size_t PosIndex = 0; 
            (PosIndex < BlockPositions->Count);
            ++PosIndex)
        {
            world_block_pos *BlockP = BlockPositions->Pos + PosIndex;
            // NOTE: Siblings may be too far from camera, so we have to add them here to render
            world_block_pos BiggerP = GetBiggerResBlockPosition(BlockP);
            lower_blocks Siblings;
            GetLowerResBlockPositions(&Siblings, &BiggerP);
            for(uint32 SiblingIndex = 0;
                SiblingIndex < ArrayCount(Siblings.Pos);
                SiblingIndex++)
            {
                world_block_pos *SiblingP = Siblings.Pos + SiblingIndex;
                block_hash *ZeroHash = GetZeroHash(World, SiblingP);
                if(HashIsEmpty(ZeroHash))
                {
                    block_hash *Hash = GetHash(World->RenderHash, SiblingP);
                    if(!HashIsEmpty(Hash))
                    {
                        AddToRenderBlocks(GameState, World->PoligonisedBlocks + Hash->Index, CameraP, CamDir);
                    }
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
    
    // DXResources->SetTransformations(GameState->CubePos);
    DXResources->DrawTriangles(GameState->Cube.Vertices, CubeVertexCount);
    DXResources->SetTransformations(v3{});
    
    
    // NOTE: Draw debug resolution blocks
    GameState->ResCubeCount = 0;
    DXResources->SetDrawModeWireframe();
    DXResources->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < StoreResolutionCount;
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
    // win32_printer::PerfPrint("Generate render: %f for %d blocks", TimeGenerateRender * 1000.0, RenderCount);
    // win32_printer::PerfPrint("Add to render time: %f", TimeAddToRender * 1000.0);
    // win32_printer::PerfPrint("Render time: %f", TimeToRender * 1000.0);
    
}








