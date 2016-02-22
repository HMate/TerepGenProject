/*
    Terep generátor by Hidvégi Máté @2015

*/

inline world_block_pos
WorldPosFromV3(world_density *World, v3 Pos, int32 Resolution)
{
    world_block_pos Result = {};
    
    v3 CentralBlockPos = Pos / (World->BlockSize*Resolution);
    Result.BlockX = FloorInt32(CentralBlockPos.X); 
    Result.BlockY = FloorInt32(CentralBlockPos.Y);
    Result.BlockZ = FloorInt32(CentralBlockPos.Z);
    Result.Resolution = Resolution;
     
    return Result;
}

inline v3
V3FromWorldPos(world_density *World, world_block_pos Pos)
{
    v3 Result = {};
    Result.X = (real32)Pos.BlockX * World->BlockSize * Pos.Resolution;
    Result.Y = (real32)Pos.BlockY * World->BlockSize * Pos.Resolution;
    Result.Z = (real32)Pos.BlockZ * World->BlockSize * Pos.Resolution;
    
    return Result;
}

// NOTE: Block Resolution gives how many density values are skipped
// This way a bigger area can be stored in the same block, 
// if at rendering we only use every BlockResolution'th value too.
internal void 
GenerateDensityGrid(world_density *World, terrain_density_block *DensityBlock, perlin_noise_array *PNArray, world_block_pos WorldP)
{
    DensityBlock->Pos = WorldP;
    real32 BlockResolution = (real32)WorldP.Resolution;
    
    v3 BlockPos = V3FromWorldPos(World, WorldP);
    
    uint32 TerrainDimension = DensityBlock->Grid.Dimension;
    for(uint32 Plane = 0;
        Plane < TerrainDimension;
        ++Plane) 
    {
        for(uint32 Row = 0;
            Row < TerrainDimension;
            ++Row)
        {
            for(uint32 Column = 0;
                Column < TerrainDimension;
                ++Column)
            {
                real32 DensityValue = BlockPos.Y + (real32)((Row) * BlockResolution);
                // real32 DensityValue = 0;
                
                real32 WorldX = BlockPos.X + ((Plane) * BlockResolution);
                real32 WorldY = BlockPos.Y + ((Row) * BlockResolution);
                real32 WorldZ = BlockPos.Z + ((Column) * BlockResolution);
                
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
                
                SetGrid(&DensityBlock->Grid, Plane, Row, Column, DensityValue);
            }
        }
    }
}

inline bool32
HashIsEmpty(block_hash *BlockHash)
{
    bool32 Result = (BlockHash->Index == HASH_UNINITIALIZED) ||
       (BlockHash->Index == HASH_DELETED);
    return Result;
}
    
internal block_hash *
GetHash(block_hash *HashArray, world_block_pos P)
{
    block_hash *Result = 0;

    uint32 HashValue = 2557*P.Resolution + 151*P.BlockX + 37*P.BlockY + 5*P.BlockZ;
    uint32 HashMask = (BLOCK_HASH_SIZE - 1);
    
    for(uint32 Offset = 0;
        Offset < BLOCK_HASH_SIZE;
        ++Offset)
    {
        uint32 HashIndex = (HashValue + Offset) & HashMask;
        Assert(HashIndex < BLOCK_HASH_SIZE);
        block_hash *Hash = HashArray + HashIndex;
        
        if(Hash->Index == HASH_UNINITIALIZED || 
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
WriteHash(block_hash *HashArray, world_block_pos P, int32 NewBlockIndex)
{
    block_hash *Result = 0;

    uint32 HashValue =  2557*P.Resolution + 151*P.BlockX + 37*P.BlockY + 5*P.BlockZ;
    uint32 HashMask = (BLOCK_HASH_SIZE - 1);
    
    for(uint32 Offset = 0;
        Offset < BLOCK_HASH_SIZE;
        ++Offset)
    {
        uint32 HashIndex = (HashValue + Offset) & HashMask;
        Assert(HashIndex < BLOCK_HASH_SIZE);
        block_hash *Hash = HashArray + HashIndex;
        
        if(Hash->Index == HASH_UNINITIALIZED || Hash->Index == HASH_DELETED ||
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
    Assert(Result->Index == HASH_UNINITIALIZED || Result->Index == HASH_DELETED);
        
    Result->Key = P;
    Result->Index = NewBlockIndex;
    
    return Result;
}

internal block_hash *
GetZeroHash(world_density *World, world_block_pos P)
{
    block_hash *Result = 0;

    uint32 HashValue = 2579*P.Resolution + 757*P.BlockX + 89*P.BlockY + 5*P.BlockZ;
    uint32 HashMask = (ArrayCount(World->ZeroHash) - 1);
    
    for(uint32 Offset = 0;
        Offset < ArrayCount(World->ZeroHash);
        ++Offset)
    {
        uint32 HashIndex = (HashValue + Offset) & HashMask;
        Assert(HashIndex < ArrayCount(World->ZeroHash));
        block_hash *Hash = World->ZeroHash + HashIndex;
        
        // NOTE: return hash, if its uninited, or it has the position we are looking for
        if(Hash->Index == HASH_UNINITIALIZED || 
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
InitBlockHash(world_density *World)
{
    // TODO: Does zeroing out stored block count belong here?
    World->DensityBlockCount = 0;
    World->PoligonisedBlockCount = 0;
    // GameState->PoligonisedBlock2Count = 0;
    // GameState->PoligonisedBlock4Count = 0;
    World->DeletedBlockCount = 0;
    
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(World->BlockHash);
        ++HashIndex)
    {
        block_hash *Hash = World->BlockHash + HashIndex;
        Hash->Index = HASH_UNINITIALIZED;
    }
    
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(World->RenderHash);
        ++HashIndex)
    {
        block_hash *Hash = World->RenderHash + HashIndex;
        Hash->Index = HASH_UNINITIALIZED;
    }
}

inline void
InitZeroHash(world_density *World)
{
    World->ZeroBlockCount = 0;
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(World->ZeroHash);
        ++HashIndex)
    {
        block_hash *Hash = World->ZeroHash + HashIndex;
        Hash->Index = HASH_UNINITIALIZED;
    }
}

// NOTE: XYZ are relative to the block position
internal real32
GetWorldGrid(world_density *World, terrain_density_block *DensityBlock, int32 X, int32 Y, int32 Z)
{
    world_block_pos ActualWPos = DensityBlock->Pos;;
    
    int32 GridStep = (int32)World->BlockSize;
    int32 DiffX = X / GridStep;
    int32 DiffY = Y / GridStep;
    int32 DiffZ = Z / GridStep;
    
    ActualWPos.BlockX += DiffX;
    ActualWPos.BlockY += DiffY;
    ActualWPos.BlockZ += DiffZ;
    
    uint32 ActX = (uint32)(X - (DiffX * GridStep));
    uint32 ActY = (uint32)(Y - (DiffY * GridStep));
    uint32 ActZ = (uint32)(Z - (DiffZ * GridStep));
    
    block_hash *BlockHash = GetHash((block_hash*)&World->BlockHash, ActualWPos);
    real32 Result = 2.0f;
    if(!HashIsEmpty(BlockHash))
    {
        terrain_density_block *ActDensityBlock = World->DensityBlocks + BlockHash->Index;
        Result = GetGrid(&ActDensityBlock->Grid, ActX, ActY, ActZ);
    }
    
    return Result;
}

internal real32
GetInterpolatedWorldGrid(world_density *World, terrain_density_block *DensityBlock, 
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
        return GetWorldGrid(World, DensityBlock, XFloor, YFloor, ZFloor);
    else if(XRemainder < 0.0001f && YRemainder < 0.0001f)
    {
        real32 Elem1 = GetWorldGrid(World, DensityBlock, XFloor, YFloor, ZFloor);
        real32 Elem2 = GetWorldGrid(World, DensityBlock, XFloor, YFloor, ZFloor + 1);
    
        real32 Result = Elem1 + ZRemainder * (Elem2 - Elem1);
        return Result;
    }
    else if(XRemainder < 0.0001f)
    {
        real32 Elem1 = GetInterpolatedWorldGrid(World, DensityBlock, X, (real32)YFloor, Z);
        real32 Elem2 = GetInterpolatedWorldGrid(World, DensityBlock, X, (real32)(YFloor+1), Z);
        
        real32 Result = Elem1 + YRemainder * (Elem2 - Elem1);
        return Result;
    }
    else
    {
        real32 Elem1 = GetInterpolatedWorldGrid(World, DensityBlock, (real32)XFloor, Y, Z);
        real32 Elem2 = GetInterpolatedWorldGrid(World, DensityBlock, (real32)(XFloor+1), Y, Z);
        
        real32 Result = Elem1 + XRemainder * (Elem2 - Elem1);
        return Result;
    }
}

internal vertex
Get3DVertex(v3 LocalPos, v3 Normal, v4 Color)
{
    vertex Result = {LocalPos.X, LocalPos.Y, LocalPos.Z, 
                     Normal.X, Normal.Y, Normal.Z,
                     Color};
    return Result;
}

internal v3
GetPointNormal(world_density *World, terrain_density_block *DensityBlock, v3 Point)
{
    real32 Diff = 0.5f;
    real32 DimensionBound = (real32)DensityBlock->Grid.Dimension-1.0f;
    
    real32 DiffXMin = Point.X - Diff;
    DiffXMin = ClampReal32(DiffXMin, 0.0f, DimensionBound);
    real32 DiffXMax = Point.X + Diff;
    DiffXMax = ClampReal32(DiffXMax, 0.0f, DimensionBound);
    
    real32 DiffYMin = Point.Y - Diff;
    DiffYMin = ClampReal32(DiffYMin, 0.0f, DimensionBound);
    real32 DiffYMax = Point.Y + Diff;
    DiffYMax = ClampReal32(DiffYMax, 0.0f, DimensionBound);
    
    real32 DiffZMin = Point.Z - Diff;
    DiffZMin = ClampReal32(DiffZMin, 0.0f, DimensionBound);
    real32 DiffZMax = Point.Z + Diff;
    DiffZMax = ClampReal32(DiffZMax, 0.0f, DimensionBound);
    
    real32 XP = GetInterpolatedWorldGrid(World, DensityBlock, DiffXMax, Point.Y, Point.Z);
    real32 XM = GetInterpolatedWorldGrid(World, DensityBlock, DiffXMin, Point.Y, Point.Z);
    real32 NormalX = XP - XM;
    real32 YP = GetInterpolatedWorldGrid(World, DensityBlock, Point.X, DiffYMax, Point.Z);
    real32 YM = GetInterpolatedWorldGrid(World, DensityBlock, Point.X, DiffYMin, Point.Z);
    real32 NormalY = YP - YM;
    real32 ZP = GetInterpolatedWorldGrid(World, DensityBlock, Point.X, Point.Y, DiffZMax);
    real32 ZM = GetInterpolatedWorldGrid(World, DensityBlock, Point.X, Point.Y, DiffZMin);
    real32 NormalZ = ZP - ZM;
        
    v3 Result = v3{NormalX, NormalY, NormalZ};
    Result = Normalize(Result);
    return Result;
}

#define DENSITY_ISO_LEVEL 0.0f
/*
    NOTE: The dimension of the grids required to be 5 units bigger
        than the size of the final terrain
        +1 in each dimension for marching cubes
        +2 at each side to calculate proper normals
*/
internal void
PoligoniseBlock(world_density *World, terrain_render_block *RenderBlock, terrain_density_block *DensityBlock)
{
    Assert(DensityBlock->Pos.Resolution > 0);
    real32 CellDiff = (real32)DensityBlock->Pos.Resolution;
    v4 GreenColor = v4{0.0, 1.0f, 0.0f, 1.0f};
    
    RenderBlock->Pos = V3FromWorldPos(World, DensityBlock->Pos);
    uint32 TerrainDimension = DensityBlock->Grid.Dimension;
    
    uint32 VertexCount = 0;
    for(uint32 Plane = 0;
        Plane < TerrainDimension;
        Plane += 1)
    {
        for(uint32 Row = 0;
            Row < TerrainDimension;
            Row += 1)
        {
            for(uint32 Column = 0;
                Column < TerrainDimension;
                Column += 1)
            {                
                GRIDCELL Cell;
                real32 Planef = (real32)Plane;
                real32 Rowf = (real32)Row;
                real32 Columnf = (real32)Column;
                Cell.p[0] = v3{Planef     , Rowf+1.0f, Columnf     };
                Cell.p[1] = v3{Planef     , Rowf+1.0f, Columnf+1.0f};
                Cell.p[2] = v3{Planef     , Rowf     , Columnf+1.0f};
                Cell.p[3] = v3{Planef     , Rowf     , Columnf     };
                Cell.p[4] = v3{Planef+1.0f, Rowf+1.0f, Columnf     };
                Cell.p[5] = v3{Planef+1.0f, Rowf+1.0f, Columnf+1.0f};
                Cell.p[6] = v3{Planef+1.0f, Rowf     , Columnf+1.0f};
                Cell.p[7] = v3{Planef+1.0f, Rowf     , Columnf     };
                Cell.val[0] = GetWorldGrid(World, DensityBlock, Plane  , Row+1, Column  );
                Cell.val[1] = GetWorldGrid(World, DensityBlock, Plane  , Row+1, Column+1);
                Cell.val[2] = GetWorldGrid(World, DensityBlock, Plane  , Row  , Column+1);
                Cell.val[3] = GetWorldGrid(World, DensityBlock, Plane  , Row  , Column  );
                Cell.val[4] = GetWorldGrid(World, DensityBlock, Plane+1, Row+1, Column  );
                Cell.val[5] = GetWorldGrid(World, DensityBlock, Plane+1, Row+1, Column+1);
                Cell.val[6] = GetWorldGrid(World, DensityBlock, Plane+1, Row  , Column+1);
                Cell.val[7] = GetWorldGrid(World, DensityBlock, Plane+1, Row  , Column  );
                TRIANGLE Triangles[5];
                uint32 TriangleCount = Polygonise(Cell, DENSITY_ISO_LEVEL, Triangles);
                
                for(uint32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
                {
                    v3 Point0 = Triangles[TriangleIndex].p[0];
                    v3 Point1 = Triangles[TriangleIndex].p[1];
                    v3 Point2 = Triangles[TriangleIndex].p[2];
                                        
                    v3 Normal0 = GetPointNormal(World, DensityBlock, Point0);
                    v3 Normal1 = GetPointNormal(World, DensityBlock, Point1);
                    v3 Normal2 = GetPointNormal(World, DensityBlock, Point2);
                    
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
    
#if 0 //TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Current Vertex Count: %d\n", VertexCount);
    OutputDebugStringA(DebugBuffer);
#endif
}









