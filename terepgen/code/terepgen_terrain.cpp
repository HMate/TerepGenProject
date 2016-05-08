/*
    Terep generátor by Hidvégi Máté @2015
*/

// NOTE: Block Resolution gives how frequent is the sampling from the noise function
// This way a bigger area can be stored in the same block, 
// if at rendering we only use every BlockResolution'th value too.
internal void 
GenerateDensityGrid(terrain_density_block *DensityBlock, perlin_noise_array *PNArray, 
                    world_block_pos *WorldP)
{
    DensityBlock->Pos = *WorldP;
    real32 BlockResolution = (real32)WorldP->Resolution * RENDER_SPACE_UNIT;
    
    v3 BlockPos = V3FromWorldPos(*WorldP);
    
    uint32 TerrainDimension = DensityBlock->Grid.Dimension;
    for(uint32 X = 0;
        X < TerrainDimension;
        ++X) 
    {
        for(uint32 Y = 0;
            Y < TerrainDimension;
            ++Y)
        {
            for(uint32 Z = 0;
                Z < TerrainDimension;
                ++Z)
            {
                real32 DensityValue = BlockPos.Y + ((Y) * BlockResolution);
                // real32 DensityValue = 0;
                
                real32 WorldX = BlockPos.X + ((X) * BlockResolution);
                real32 WorldY = BlockPos.Y + ((Y) * BlockResolution);
                real32 WorldZ = BlockPos.Z + ((Z) * BlockResolution);
                
                v3 WorldPos = v3{WorldX, WorldY, WorldZ} / 32.0f;
                real32 Scale = 50.0f;
                             
                //win32_clock Clock;
                // DensityValue += RandomFloat(&PNArray->Noise[0], WorldPos * 256.03f) * Scale * 0.0036025f;
                // DensityValue += RandomFloat(&PNArray->Noise[1], WorldPos * 128.96f) * Scale * 0.0078125f;
                DensityValue += RandomFloat(&PNArray->Noise[2], WorldPos * 64.01f)  * Scale * 0.015625f;
                //Clock.PrintMiliSeconds("Perlin gen time:");
                
                DensityValue += RandomFloat(&PNArray->Noise[0], WorldPos * 32.03f) * Scale * 0.03125f;
                DensityValue += RandomFloat(&PNArray->Noise[1], WorldPos * 16.16f) * Scale * 0.0625f;
                // DensityValue += RandomFloat(&PNArray->Noise[2], WorldPos * 7.91f)  * Scale * 0.125f;
                
                // DensityValue += RandomFloat(&PNArray->Noise[0], WorldPos * 4.03f) * Scale * 0.25f;
                DensityValue += RandomFloat(&PNArray->Noise[1], WorldPos * 1.96f) * Scale * 0.5f;
                // DensityValue += RandomFloat(&PNArray->Noise[2], WorldPos * 1.01f) * Scale * 1.0f;
                DensityValue += RandomFloat(&PNArray->Noise[2], WorldPos * 0.491f) * Scale * 1.0f;
                
                DensityValue += RandomFloat(&PNArray->Noise[0], WorldPos * 0.023f) * Scale * 4.0f;
                DensityValue += RandomFloat(&PNArray->Noise[1], WorldPos * 0.00646f) * Scale * 16.0f;
                
                SetGrid(&DensityBlock->Grid, X, Y, Z, DensityValue);
            }
        }
    }
}

// NOTE: XYZ are relative to the block position
internal real32
GetWorldGrid(world_density *World, world_block_pos *BlockP, int32 X, int32 Y, int32 Z)
{
    block_node Node = GetActualBlockNode(BlockP, X, Y, Z);
    //block_hash *ResHash = GetHash(World->ResolutionMapping, &Node.BlockP);
    
    block_hash *DensityHash = GetHash(World->DensityHash, &Node.BlockP);
    // TODO: What if this block wasnt generated? 
    // maybe create an IsBlockValid(world_block_pos)->bool32 ?
    real32 Result = 0.0f; 
    if(!HashIsEmpty(DensityHash))
    {
        terrain_density_block *ActDensityBlock = World->DensityBlocks + DensityHash->Index;
        Result = GetGrid(&ActDensityBlock->Grid, Node.X, Node.Y, Node.Z);
    }
    
    return Result;
}

// NOTE: floating values here are world grid positions in float, not render space values!
internal real32
GetInterpolatedWorldGrid(world_density *World, world_block_pos *BlockP, 
                         real32 X, real32 Y, real32 Z)
{
    int32 XFloor = FloorInt32(X);
    real32 XRemainder = X - (real32)XFloor;
    int32 YFloor = FloorInt32(Y);
    real32 YRemainder = Y - (real32)YFloor;
    int32 ZFloor = FloorInt32(Z);
    real32 ZRemainder = Z - (real32)ZFloor;
    
    // NOTE: If every parameter is whole number, we can just give back the grid value
    if(XRemainder < 0.0001f && YRemainder < 0.0001f && ZRemainder < 0.0001f)
        return GetWorldGrid(World, BlockP, XFloor, YFloor, ZFloor);
    else if(XRemainder < 0.0001f && YRemainder < 0.0001f)
    {
        real32 Elem1 = GetWorldGrid(World, BlockP, XFloor, YFloor, ZFloor);
        real32 Elem2 = GetWorldGrid(World, BlockP, XFloor, YFloor, ZFloor + 1);
    
        real32 Result = Elem1 + ZRemainder * (Elem2 - Elem1);
        return Result;
    }
    else if(XRemainder < 0.0001f)
    {
        real32 Elem1 = GetInterpolatedWorldGrid(World, BlockP, X, (real32)YFloor, Z);
        real32 Elem2 = GetInterpolatedWorldGrid(World, BlockP, X, (real32)(YFloor+1), Z);
        
        real32 Result = Elem1 + YRemainder * (Elem2 - Elem1);
        return Result;
    }
    else
    {
        real32 Elem1 = GetInterpolatedWorldGrid(World, BlockP, (real32)XFloor, Y, Z);
        real32 Elem2 = GetInterpolatedWorldGrid(World, BlockP, (real32)(XFloor+1), Y, Z);
        
        real32 Result = Elem1 + XRemainder * (Elem2 - Elem1);
        return Result;
    }
}

// NOTE: V3 contains renderspace values here
internal real32
GetWorldGridValueFromV3(world_density *World, v3 Pos, int32 Resolution)
{
    // TODO: Resolution
    world_block_pos WorldOrigo{0, 0, 0, Resolution};
    v3 BlockPos = Pos/((real32)Resolution * RENDER_SPACE_UNIT);
    
    real32 Result = GetInterpolatedWorldGrid(World, &WorldOrigo, BlockPos.X, BlockPos.Y, BlockPos.Z);
    return Result;
}

// NOTE: XYZ are relative to the block position
// Handles bigger resolution neighbours by rounding to smaller index
internal real32
GetFromNeighbours(terrain_density_block **Neighbours, terrain_density_block **DynNeighbours,
                  world_block_pos *BlockP, 
                  int32 X, int32 Y, int32 Z)
{
    real32 BlockSize = (real32)TERRAIN_BLOCK_SIZE/2;
    int32 DiffX = FloorInt32(X / BlockSize);
    int32 DiffY = FloorInt32(Y / BlockSize);
    int32 DiffZ = FloorInt32(Z / BlockSize);
    
    // NOTE: Neighbours are stored in X,Y,Z order, where Z is the least significant dimension
    // 21, 22, 25,26, 37,38, 41,42 are the current/center block
    uint32 NIndex = 21 + DiffX*16 + DiffY*4 + DiffZ;
    Assert(NIndex < 64);
    
    terrain_density_block *ActDensityBlock = Neighbours[NIndex];
    terrain_density_block *ActDynamicBlock = DynNeighbours[NIndex];
    
    world_block_pos ParentPos = ConvertToResolution(BlockP, ActDensityBlock->Pos.Resolution);
    
    const int32 GridStep = (int32)TERRAIN_BLOCK_SIZE;
    BlockSize = (real32)TERRAIN_BLOCK_SIZE;
    const int32 OriginalRes = BlockP->Resolution;
    const int32 NewRes = ActDensityBlock->Pos.Resolution;
    
    world_block_pos ParentPosInOriginalRes = ConvertToResolution(&ParentPos, BlockP->Resolution);
    int32 OffsetInParentX = (BlockP->BlockX - ParentPosInOriginalRes.BlockX) * (GridStep * OriginalRes / NewRes);
    int32 OffsetInParentY = (BlockP->BlockY - ParentPosInOriginalRes.BlockY) * (GridStep * OriginalRes / NewRes);
    int32 OffsetInParentZ = (BlockP->BlockZ - ParentPosInOriginalRes.BlockZ) * (GridStep * OriginalRes / NewRes);
    
    int32 NewResX = OffsetInParentX + FloorInt32((real32)X * OriginalRes / NewRes);
    int32 NewResY = OffsetInParentY + FloorInt32((real32)Y * OriginalRes / NewRes);
    int32 NewResZ = OffsetInParentZ + FloorInt32((real32)Z * OriginalRes / NewRes);
    
    int32 NewResDiffX = FloorInt32(NewResX / BlockSize);
    int32 NewResDiffY = FloorInt32(NewResY / BlockSize);
    int32 NewResDiffZ = FloorInt32(NewResZ / BlockSize);
    
    uint32 NewX = (uint32)(NewResX - (NewResDiffX * GridStep));
    uint32 NewY = (uint32)(NewResY - (NewResDiffY * GridStep));
    uint32 NewZ = (uint32)(NewResZ - (NewResDiffZ * GridStep));
    
    real32 Density = GetGrid(&ActDensityBlock->Grid, NewX, NewY, NewZ);
    real32 Dynamic = GetGrid(&ActDynamicBlock->Grid, NewX, NewY, NewZ);
    
    real32 Result = Density + Dynamic;
    return Result;
}

// NOTE: floating values here are world grid positions in float, not render space values!
internal real32
GetInterpolatedNeighbour(terrain_density_block **Neighbours, terrain_density_block **DynNeighbours, 
                         world_block_pos *BlockP, real32 X, real32 Y, real32 Z)
{
    int32 XFloor = FloorInt32(X);
    real32 XRemainder = X - (real32)XFloor;
    int32 YFloor = FloorInt32(Y);
    real32 YRemainder = Y - (real32)YFloor;
    int32 ZFloor = FloorInt32(Z);
    real32 ZRemainder = Z - (real32)ZFloor;
    
    real32 V0 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XFloor    , YFloor    , ZFloor    );
    real32 V1 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XFloor    , YFloor    , ZFloor + 1);
    real32 V2 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XFloor    , YFloor + 1, ZFloor    );
    real32 V3 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XFloor    , YFloor + 1, ZFloor + 1);
    real32 V4 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XFloor + 1, YFloor    , ZFloor    );
    real32 V5 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XFloor + 1, YFloor    , ZFloor + 1);
    real32 V6 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XFloor + 1, YFloor + 1, ZFloor    );
    real32 V7 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XFloor + 1, YFloor + 1, ZFloor + 1);
    
    real32 Z0 = V0 + ZRemainder *(V1-V0);
    real32 Z1 = V2 + ZRemainder *(V3-V2);
    real32 Z2 = V4 + ZRemainder *(V5-V4);
    real32 Z3 = V6 + ZRemainder *(V7-V6);
    
    real32 Y0 = Z0 + YRemainder *(Z1-Z0);
    real32 Y1 = Z2 + YRemainder *(Z3-Z2);
    
    real32 Result = Y0 + XRemainder * (Y1 - Y0);
    return Result;
}

internal real32
GetInterpolatedValueByResolution(terrain_density_block **Neighbours, terrain_density_block **DynNeighbours, 
                                 world_block_pos *BlockP, int32 NewRes, 
                                 real32 X, real32 Y, real32 Z, int32 origX, int32 origY, int32 origZ)
{
    int32 OldRes = BlockP->Resolution;
    real32 Ratio = (real32)NewRes/OldRes;
    
    int32 XFloor = FloorInt32(X);
    real32 XRemainder = X - (real32)XFloor;
    int32 YFloor = FloorInt32(Y);
    real32 YRemainder = Y - (real32)YFloor;
    int32 ZFloor = FloorInt32(Z);
    real32 ZRemainder = Z - (real32)ZFloor;
    
    int32 XF0 = origX - (int32)(Ratio*XRemainder);
    int32 XF1 = origX + (int32)(Ratio*XRemainder);
    int32 YF0 = origY - (int32)(Ratio*YRemainder);
    int32 YF1 = origY + (int32)(Ratio*YRemainder);
    int32 ZF0 = origZ - (int32)(Ratio*ZRemainder);
    int32 ZF1 = origZ + (int32)(Ratio*ZRemainder);
    
    real32 V0 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XF0, YF0, ZF0);
    real32 V1 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XF0, YF0, ZF1);
    real32 V2 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XF0, YF1, ZF0);
    real32 V3 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XF0, YF1, ZF1);
    real32 V4 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XF1, YF0, ZF0);
    real32 V5 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XF1, YF0, ZF1);
    real32 V6 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XF1, YF1, ZF0);
    real32 V7 = GetFromNeighbours(Neighbours, DynNeighbours, BlockP, XF1, YF1, ZF1);
    
    real32 Z0 = V0 + ZRemainder *(V1-V0);
    real32 Z1 = V2 + ZRemainder *(V3-V2);
    real32 Z2 = V4 + ZRemainder *(V5-V4);
    real32 Z3 = V6 + ZRemainder *(V7-V6);
    
    real32 Y0 = Z0 + YRemainder *(Z1-Z0);
    real32 Y1 = Z2 + YRemainder *(Z3-Z2);
    
    real32 Result = Y0 + XRemainder * (Y1 - Y0);
    return Result;
}

internal real32
GetExactFromNeighbours(terrain_density_block **Neighbours, terrain_density_block **DynNeighbours,
                      world_block_pos *BlockP, 
                      int32 X, int32 Y, int32 Z)
{
    real32 BlockSize = (real32)TERRAIN_BLOCK_SIZE/2;
    int32 DiffX = FloorInt32(X / BlockSize);
    int32 DiffY = FloorInt32(Y / BlockSize);
    int32 DiffZ = FloorInt32(Z / BlockSize);
    
    // NOTE: Neighbours are stored in X,Y,Z order, where Z is the least significant dimension
    // 21, 22, 25,26, 37,38, 41,42 are the current/center block
    uint32 NIndex = 21 + DiffX*16 + DiffY*4 + DiffZ;
    Assert(NIndex < 64);
    
    terrain_density_block *ActDensityBlock = Neighbours[NIndex];
    
    world_block_pos ParentPos = ConvertToResolution(BlockP, ActDensityBlock->Pos.Resolution);
    
    const int32 GridStep = (int32)TERRAIN_BLOCK_SIZE;
    BlockSize = (real32)TERRAIN_BLOCK_SIZE;
    const int32 OriginalRes = BlockP->Resolution;
    const int32 NewRes = ActDensityBlock->Pos.Resolution;
    
    world_block_pos ParentPosInOriginalRes = ConvertToResolution(&ParentPos, BlockP->Resolution);
    int32 OffsetInParentX = (BlockP->BlockX - ParentPosInOriginalRes.BlockX) * (GridStep * OriginalRes / NewRes);
    int32 OffsetInParentY = (BlockP->BlockY - ParentPosInOriginalRes.BlockY) * (GridStep * OriginalRes / NewRes);
    int32 OffsetInParentZ = (BlockP->BlockZ - ParentPosInOriginalRes.BlockZ) * (GridStep * OriginalRes / NewRes);
    
    real32 NewResX = OffsetInParentX + ((real32)X * OriginalRes / NewRes);
    real32 NewResY = OffsetInParentY + ((real32)Y * OriginalRes / NewRes);
    real32 NewResZ = OffsetInParentZ + ((real32)Z * OriginalRes / NewRes);
    
    real32 Result = GetInterpolatedValueByResolution(Neighbours, DynNeighbours, BlockP, NewRes, NewResX, NewResY, NewResZ, X, Y, Z);
    
    return Result;
}

internal v3
GetPointNormal(terrain_density_block **Neighbours, terrain_density_block **DynNeighbours, 
               world_block_pos *BlockP, v3 Point)
{
    real32 Diff = 0.5f;
    
    real32 DiffXMin = Point.X - Diff;
    real32 DiffXMax = Point.X + Diff;
    
    real32 DiffYMin = Point.Y - Diff;
    real32 DiffYMax = Point.Y + Diff;
    
    real32 DiffZMin = Point.Z - Diff;
    real32 DiffZMax = Point.Z + Diff;
    
    real32 XP = GetInterpolatedNeighbour(Neighbours, DynNeighbours, BlockP, DiffXMax, Point.Y, Point.Z);
    real32 XM = GetInterpolatedNeighbour(Neighbours, DynNeighbours, BlockP, DiffXMin, Point.Y, Point.Z);
    real32 NormalX = XP - XM;
    real32 YP = GetInterpolatedNeighbour(Neighbours, DynNeighbours, BlockP, Point.X, DiffYMax, Point.Z);
    real32 YM = GetInterpolatedNeighbour(Neighbours, DynNeighbours, BlockP, Point.X, DiffYMin, Point.Z);
    real32 NormalY = YP - YM;
    real32 ZP = GetInterpolatedNeighbour(Neighbours, DynNeighbours, BlockP, Point.X, Point.Y, DiffZMax);
    real32 ZM = GetInterpolatedNeighbour(Neighbours, DynNeighbours, BlockP, Point.X, Point.Y, DiffZMin);
    real32 NormalZ = ZP - ZM;
    
    v3 Result = v3{NormalX, NormalY, NormalZ};
    Result = Normalize(Result);
    return Result;
}

internal vertex
Get3DVertex(v3 LocalPos, v3 Normal, v4 Color)
{
    vertex Result = {LocalPos.X, LocalPos.Y, LocalPos.Z, 
                     Normal.X, Normal.Y, Normal.Z,
                     Color};
    return Result;
}

internal void 
FillDynamic(terrain_density_block *Dynamic, world_block_pos *BlockP, real32 Value)
{
    Dynamic->Pos = *BlockP;
    uint32 Dim = Dynamic->Grid.Dimension;
    for(uint32 X = 0; X < Dim; X++)
    {
        for(uint32 Y = 0; Y < Dim; Y++)
        {
            for(uint32 Z = 0; Z < Dim; Z++)
            {
                SetGrid(&Dynamic->Grid, X, Y, Z, Value);
            }
        }
    }
}



internal FileHandle
OpenBlocksFile(game_state *GameState, char *FileName)
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
            
            uint32 *Data = &GameState->Session.ID;
            uint32 Length = 4;
            uint32 BytesWritten;
            WriteFile(Handle, Data, Length, (LPDWORD)&BytesWritten, NULL);
            SetFilePointer(Handle, 0, NULL, FILE_BEGIN);
        }
    }
    return Handle;
}

internal void
SaveBlockToFile(game_state *GameState, char *FileName, terrain_density_block *Block)
{
    FileHandle Handle = OpenBlocksFile(GameState, FileName);
    
    SetFilePointer(Handle, 0, NULL, FILE_BEGIN);
    uint32 BytesRead;
    
    // NOTE: Read header
    char HeaderValue[4];
    uint32 HeaderLength = 4;
    ReadFile(Handle, HeaderValue, HeaderLength, (LPDWORD)&BytesRead, NULL);
    Assert(HeaderLength == BytesRead);
    
    uint32 GameID = *(uint32*)HeaderValue;
    Assert(GameID == GameState->Session.ID);
    
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

// NOTE: Loads a block from a file
// If the file was not in the file, returns false
internal bool32
LoadBlockFromFile(game_state *GameState, char *FileName, terrain_density_block *Block, world_block_pos *BlockP)
{
    FileHandle Handle = OpenBlocksFile(GameState, FileName);
    
    SetFilePointer(Handle, 0, NULL, FILE_BEGIN);
    uint32 BytesRead;
    
    // NOTE: Read header
    char HeaderValue[4];
    uint32 HeaderLength = 4;
    ReadFile(Handle, HeaderValue, HeaderLength, (LPDWORD)&BytesRead, NULL);
    Assert(HeaderLength == BytesRead);
    
    uint32 GameID = *(uint32*)HeaderValue;
    Assert(GameID == GameState->Session.ID);
    
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
    return !NotFound;
}

// NOTE: Creates a dynamic block, or loads it from a file
internal block_hash*
CreateNewDynamicBlock(game_state *GameState, world_density *World, world_block_pos *BlockP)
{
    terrain_density_block *DynamicB = World->DynamicBlocks + World->DynamicBlockCount;
    // NOTE: Load from file, if it was saved previously!
    bool32 Loaded = LoadBlockFromFile(GameState, GameState->Session.DynamicStore, DynamicB, BlockP);
    if(!Loaded)
    {
        FillDynamic(DynamicB, BlockP, 0.0f);
    }
    Assert(World->DynamicBlockCount < ArrayCount(World->DynamicBlocks));
    block_hash *DynamicHash = WriteHash(World->DynamicHash, BlockP, World->DynamicBlockCount++);
    
    return DynamicHash;
}

#define DENSITY_ISO_LEVEL 0.0f
internal void
PoligoniseBlock(game_state *GameState, world_density *World, terrain_render_block *RenderBlock, world_block_pos *BlockP)
{
    block_lower_neighbours NPositions;
    GetNeighbourBlockPositionsOnLowerRes(&NPositions, BlockP);
    terrain_density_block *Neighbours[ArrayCount(NPositions.Pos)];
    terrain_density_block *DynNeighbours[ArrayCount(NPositions.Pos)];
    for(uint32 NeighbourIndex = 0;
        NeighbourIndex < ArrayCount(NPositions.Pos);
        NeighbourIndex++)
    {
        world_block_pos *NeighbourP = NPositions.Pos + NeighbourIndex;
        world_block_pos MappedP = GetBiggerMappedPosition(World, NeighbourP);
        Assert((MappedP.Resolution >= BlockP->Resolution/2) &&
               (MappedP.Resolution <= BlockP->Resolution*2));
        
        block_hash *NeighbourHash = GetHash(World->DensityHash, &MappedP);
        Assert(!HashIsEmpty(NeighbourHash));
        Neighbours[NeighbourIndex] = World->DensityBlocks + NeighbourHash->Index;
        
        block_hash *DynamicHash = GetHash(World->DynamicHash, &MappedP);
        // TODO: This should be an assert
        if(HashIsEmpty(DynamicHash))
        {
            DynamicHash = CreateNewDynamicBlock(GameState, World, &MappedP);
        }
        DynNeighbours[NeighbourIndex] = World->DynamicBlocks + DynamicHash->Index;
    }
    
    Assert(BlockP->Resolution > 0);
    real32 CellDiff = (real32)BlockP->Resolution  * RENDER_SPACE_UNIT;
    v4 GreenColor = v4{0.0, 1.0f, 0.0f, 1.0f};
    
    RenderBlock->Pos = V3FromWorldPos(*BlockP);
    RenderBlock->WPos = *BlockP;
    uint32 TerrainDimension = GRID_DIMENSION;
    
    uint32 VertexCount = 0;
    for(uint32 X = 0; X < TerrainDimension; X++)
    {
        for(uint32 Y = 0; Y < TerrainDimension; Y++)
        {
            for(uint32 Z = 0; Z < TerrainDimension; Z++)
            {
                GRIDCELL Cell;
                real32 fX = (real32)X;
                real32 fY = (real32)Y;
                real32 fZ = (real32)Z;
                Cell.p[0] = v3{fX     , fY+1.0f, fZ     };
                Cell.p[1] = v3{fX     , fY+1.0f, fZ+1.0f};
                Cell.p[2] = v3{fX     , fY     , fZ+1.0f};
                Cell.p[3] = v3{fX     , fY     , fZ     };
                Cell.p[4] = v3{fX+1.0f, fY+1.0f, fZ     };
                Cell.p[5] = v3{fX+1.0f, fY+1.0f, fZ+1.0f};
                Cell.p[6] = v3{fX+1.0f, fY     , fZ+1.0f};
                Cell.p[7] = v3{fX+1.0f, fY     , fZ     };
                Cell.val[0] = GetExactFromNeighbours(Neighbours, DynNeighbours, BlockP, X  , Y+1, Z  );
                Cell.val[1] = GetExactFromNeighbours(Neighbours, DynNeighbours, BlockP, X  , Y+1, Z+1);
                Cell.val[2] = GetExactFromNeighbours(Neighbours, DynNeighbours, BlockP, X  , Y  , Z+1);
                Cell.val[3] = GetExactFromNeighbours(Neighbours, DynNeighbours, BlockP, X  , Y  , Z  );
                Cell.val[4] = GetExactFromNeighbours(Neighbours, DynNeighbours, BlockP, X+1, Y+1, Z  );
                Cell.val[5] = GetExactFromNeighbours(Neighbours, DynNeighbours, BlockP, X+1, Y+1, Z+1);
                Cell.val[6] = GetExactFromNeighbours(Neighbours, DynNeighbours, BlockP, X+1, Y  , Z+1);
                Cell.val[7] = GetExactFromNeighbours(Neighbours, DynNeighbours, BlockP, X+1, Y  , Z  );
                TRIANGLE Triangles[5];
                uint32 TriangleCount = Polygonise(Cell, DENSITY_ISO_LEVEL, Triangles);
                
                for(uint32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
                {
                    v3 Point0 = Triangles[TriangleIndex].p[0];
                    v3 Point1 = Triangles[TriangleIndex].p[1];
                    v3 Point2 = Triangles[TriangleIndex].p[2];
                                        
                    v3 Normal0 = GetPointNormal(Neighbours, DynNeighbours, BlockP, Point0);
                    v3 Normal1 = GetPointNormal(Neighbours, DynNeighbours, BlockP, Point1);
                    v3 Normal2 = GetPointNormal(Neighbours, DynNeighbours, BlockP, Point2);
                    
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex(Point0 * CellDiff, Normal0, GreenColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex(Point1 * CellDiff, Normal1, GreenColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex(Point2 * CellDiff, Normal2, GreenColor);
#if 0
                    // NOTE: Draw normals for debug purposes
                    v4 BlueColor = v4{0.0, 0.0f, 1.0f, 1.0f};
                    v3 NormalNormal = {0, 1, 0};
                    v3 NormalPerpend = Cross(Normal0, v3{1, 0, 0});
                    if(Length(NormalPerpend) < 0.01f)
                    {
                        NormalPerpend = Cross(Normal0, v3{0, 0, 1});
                    }
                    NormalPerpend = Normalize(NormalPerpend);
                    v3 Pos = Point0;
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex(Pos * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex((Pos + Normal0) * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex((Pos + 0.9f*Normal0 + 0.1f*NormalPerpend) * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex(Pos * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex((Pos + 0.9f*Normal0 + 0.1f*NormalPerpend) * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex((Pos + Normal0) * CellDiff, NormalNormal, BlueColor);
#endif
                }
            }
        }
    }
    RenderBlock->VertexCount = VertexCount;
    //win32_printer::DebugPrint("Current Vertex Count: %d", VertexCount);
}









