/*
    Terep generátor by Hidvégi Máté @2015

*/

bool32 WorldPosEquals(world_block_pos *A, world_block_pos *B)
{
    bool32 Result = (A->BlockX == B->BlockX) && 
                    (A->BlockY == B->BlockY) && 
                    (A->BlockZ == B->BlockZ) && 
                    (A->Resolution == B->Resolution);
    return Result;
}

inline bool32
HashIsEmpty(block_hash *BlockHash)
{
    bool32 Result = (BlockHash->Index == HASH_UNINITIALIZED) ||
       (BlockHash->Index == HASH_DELETED);
    return Result;
}

inline uint32 GetHashValue(world_block_pos *P)
{
    uint32 Result = 5*P->Resolution + 2557*P->BlockX + 151*P->BlockY + 37*P->BlockZ;
    return Result;
}

// NOTE: This can give back a deleted hash, if it had the same key as this block,
// and it was already deleted once, and wasn't overwritten since.
internal block_hash *
GetHash(block_hash *HashArray, world_block_pos *P)
{
    block_hash *Result = 0;

    uint32 HashValue = GetHashValue(P);
    uint32 HashMask = (BLOCK_HASH_SIZE - 1);
    
    uint32 Offset = 0;
    for(;
        Offset < BLOCK_HASH_SIZE;
        ++Offset)
    {
        uint32 HashIndex = (HashValue + Offset) & HashMask;
        Assert(HashIndex < BLOCK_HASH_SIZE);
        block_hash *Hash = HashArray + HashIndex;
        
        if(Hash->Index == HASH_UNINITIALIZED || 
           WorldPosEquals(P, &Hash->Key))
        {
            Result = Hash;
            break;
        }
    }
    Assert(Result);
    
    return Result;
}
    
internal block_hash *
WriteHash(block_hash *HashArray, world_block_pos *P, int32 NewBlockIndex)
{
    block_hash *Result = 0;

    uint32 HashValue =  GetHashValue(P);
    uint32 HashMask = (BLOCK_HASH_SIZE - 1);
    
	uint32 Offset = 0;
    for(;
        Offset < BLOCK_HASH_SIZE;
        ++Offset)
    {
        uint32 HashIndex = (HashValue + Offset) & HashMask;
        Assert(HashIndex < BLOCK_HASH_SIZE);
        block_hash *Hash = HashArray + HashIndex;
        
        if(Hash->Index == HASH_UNINITIALIZED || Hash->Index == HASH_DELETED ||
           WorldPosEquals(P, &Hash->Key))
        {
            Result = Hash;
            break;
        }
    }
    Assert(Result);
    Assert(Result->Index == HASH_UNINITIALIZED || Result->Index == HASH_DELETED);
        
    Result->Key = *P;
    Result->Index = NewBlockIndex;
    
    return Result;
}

inline uint32 GetZeroHashValue(world_block_pos *P)
{
    uint32 Result = 2579*P->Resolution + 757*P->BlockX + 89*P->BlockY + 5*P->BlockZ;
    return Result;
}

// NOTE: This can give back a deleted hash, if it had the same key as this block,
// and it was already deleted once, and wasn't overwritten since.
internal block_hash *
GetZeroHash(world_density *World, world_block_pos *P)
{
    block_hash *Result = 0;

    uint32 HashValue = GetZeroHashValue(P);
    uint32 HashMask = (ArrayCount(World->ZeroHash) - 1);
    
    for(uint32 Offset = 0;
        Offset < ArrayCount(World->ZeroHash);
        ++Offset)
    {
        uint32 HashIndex = (HashValue + Offset) & HashMask;
        Assert(HashIndex < ArrayCount(World->ZeroHash));
        block_hash *Hash = World->ZeroHash + HashIndex;
        
        // NOTE: return hash, if its uninited, or it has the position we are looking for
        if(Hash->Index == HASH_UNINITIALIZED || WorldPosEquals(P, &Hash->Key))
        {
            Result = Hash;
            break;
        }
    }
    Assert(Result);
    
    return Result;
}

internal block_hash *
WriteZeroHash(world_density *World, world_block_pos *P)
{
    block_hash *Result = 0;

    uint32 HashValue = GetZeroHashValue(P);
    uint32 HashMask = (ArrayCount(World->ZeroHash) - 1);
    
    for(uint32 Offset = 0;
        Offset < ArrayCount(World->ZeroHash);
        ++Offset)
    {
        uint32 HashIndex = (HashValue + Offset) & HashMask;
        Assert(HashIndex < ArrayCount(World->ZeroHash));
        block_hash *Hash = World->ZeroHash + HashIndex;
        
        // NOTE: return hash, if its uninited, or it has the position we are looking for
        if(Hash->Index == HASH_UNINITIALIZED || Hash->Index == HASH_DELETED ||
           WorldPosEquals(P, &Hash->Key))
        {
            Result = Hash;
            break;
        }
    }
    Assert(Result);
    Assert(Result->Index == HASH_UNINITIALIZED || Result->Index == HASH_DELETED);
    
    Result->Key = *P;
    Result->Index = HASH_ZERO_BLOCK;
    
    return Result;
}

internal void
InitResolutionMapping(world_density *World)
{
    World->BlockMappedCount = 0;
    
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(World->ResolutionMapping);
        ++HashIndex)
    {
        block_hash *Hash = World->ResolutionMapping + HashIndex;
        Hash->Index = HASH_UNINITIALIZED;
    }
}

internal void
InitBlockHash(world_density *World)
{
    // TODO: Does zeroing out stored block count belong here?
    World->DensityBlockCount = 0;
    World->PoligonisedBlockCount = 0;
    World->DeletedDensityBlockCount = 0;
    World->DeletedRenderBlockCount = 0;
    
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(World->DensityHash);
        ++HashIndex)
    {
        block_hash *Hash = World->DensityHash + HashIndex;
        Hash->Index = HASH_UNINITIALIZED;
    }
    
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(World->RenderHash);
        ++HashIndex)
    {
        block_hash *Hash = World->RenderHash + HashIndex;
        Hash->Index = HASH_UNINITIALIZED;
    }
    InitResolutionMapping(World);
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

inline world_block_pos
WorldPosFromV3(v3 Pos, int32 Resolution)
{
    world_block_pos Result = {};
    real32 BlockSize = (real32)TERRAIN_BLOCK_SIZE;
    
    v3 CentralBlockPos = Pos / (BlockSize * Resolution * RENDER_SPACE_UNIT);
    Result.BlockX = FloorInt32(CentralBlockPos.X); 
    Result.BlockY = FloorInt32(CentralBlockPos.Y);
    Result.BlockZ = FloorInt32(CentralBlockPos.Z);
    Result.Resolution = Resolution;
     
    return Result;
}

inline v3
V3FromWorldPos(world_block_pos Pos)
{
    v3 Result = {};
    real32 BlockSize = (real32)TERRAIN_BLOCK_SIZE;
    Result.X = (real32)Pos.BlockX * BlockSize * Pos.Resolution * RENDER_SPACE_UNIT;
    Result.Y = (real32)Pos.BlockY * BlockSize * Pos.Resolution * RENDER_SPACE_UNIT;
    Result.Z = (real32)Pos.BlockZ * BlockSize * Pos.Resolution * RENDER_SPACE_UNIT;
    
    return Result;
}

// NOTE: Block Resolution gives how many density values are skipped
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

internal block_node
GetActualBlockNode(world_block_pos *Original, int32 X, int32 Y, int32 Z)
{
    // TODO: What to do with Resolution ??
    block_node Result;
    
    Result.BlockP = *Original;
    
    real32 BlockSize = (real32)TERRAIN_BLOCK_SIZE;
    int32 DiffX = FloorInt32(X / BlockSize);
    int32 DiffY = FloorInt32(Y / BlockSize);
    int32 DiffZ = FloorInt32(Z / BlockSize);
    
    Result.BlockP.BlockX += DiffX;
    Result.BlockP.BlockY += DiffY;
    Result.BlockP.BlockZ += DiffZ;
    
    int32 GridStep = (int32)TERRAIN_BLOCK_SIZE;
    Result.X = (uint32)(X - (DiffX * GridStep));
    Result.Y = (uint32)(Y - (DiffY * GridStep));
    Result.Z = (uint32)(Z - (DiffZ * GridStep));
    
    return Result;
}

internal block_node
ConvertRenderPosToBlockNode(v3 RenderPos, int32 Resolution)
{
    world_block_pos WorldOrigo{0, 0, 0, Resolution};
    v3 NodeFromOrigo = RenderPos/((real32)Resolution * RENDER_SPACE_UNIT);
    
    int32 XFloor = FloorInt32(NodeFromOrigo.X);
    int32 YFloor = FloorInt32(NodeFromOrigo.Y);
    int32 ZFloor = FloorInt32(NodeFromOrigo.Z);
    
    block_node Node = GetActualBlockNode(&WorldOrigo, XFloor, YFloor, ZFloor);
    return Node;
}

internal v3
ConvertBlockNodeToRenderPos(block_node Node)
{
    v3 Result = V3FromWorldPos(Node.BlockP);
    Result = Result + v3{(real32)Node.X, (real32)Node.Y, (real32)Node.Z} 
                        * (real32)Node.BlockP.Resolution * RENDER_SPACE_UNIT;
    
    return Result;
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

internal world_block_pos
ConvertToResolution(world_block_pos *P, uint32 NewRes)
{
    world_block_pos Result;
    Result.Resolution = NewRes;
    
    real32 Ratio = (real32)P->Resolution / NewRes;
    Result.BlockX = FloorInt32(P->BlockX * Ratio);
    Result.BlockY = FloorInt32(P->BlockY * Ratio);
    Result.BlockZ = FloorInt32(P->BlockZ * Ratio);
    
    return Result;
}

// NOTE: XYZ are relative to the block position
// Handles bigger resolution neighbours by rounding to smaller index
internal real32
GetFromNeighbours(terrain_density_block **Neighbours,
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
    
    int32 NewResX = OffsetInParentX + FloorInt32((real32)X * OriginalRes / NewRes);
    int32 NewResY = OffsetInParentY + FloorInt32((real32)Y * OriginalRes / NewRes);
    int32 NewResZ = OffsetInParentZ + FloorInt32((real32)Z * OriginalRes / NewRes);
    
    int32 NewResDiffX = FloorInt32(NewResX / BlockSize);
    int32 NewResDiffY = FloorInt32(NewResY / BlockSize);
    int32 NewResDiffZ = FloorInt32(NewResZ / BlockSize);
    
    uint32 NewX = (uint32)(NewResX - (NewResDiffX * GridStep));
    uint32 NewY = (uint32)(NewResY - (NewResDiffY * GridStep));
    uint32 NewZ = (uint32)(NewResZ - (NewResDiffZ * GridStep));
    
    real32 Result = GetGrid(&ActDensityBlock->Grid, NewX, NewY, NewZ);
    
    return Result;
}

// NOTE: floating values here are world grid positions in float, not render space values!
internal real32
GetInterpolatedNeighbour(terrain_density_block **Neighbours, world_block_pos *BlockP, 
                         real32 X, real32 Y, real32 Z)
{
    int32 XFloor = FloorInt32(X);
    real32 XRemainder = X - (real32)XFloor;
    int32 YFloor = FloorInt32(Y);
    real32 YRemainder = Y - (real32)YFloor;
    int32 ZFloor = FloorInt32(Z);
    real32 ZRemainder = Z - (real32)ZFloor;
    
    real32 V0 = GetFromNeighbours(Neighbours, BlockP, XFloor    , YFloor    , ZFloor    );
    real32 V1 = GetFromNeighbours(Neighbours, BlockP, XFloor    , YFloor    , ZFloor + 1);
    real32 V2 = GetFromNeighbours(Neighbours, BlockP, XFloor    , YFloor + 1, ZFloor    );
    real32 V3 = GetFromNeighbours(Neighbours, BlockP, XFloor    , YFloor + 1, ZFloor + 1);
    real32 V4 = GetFromNeighbours(Neighbours, BlockP, XFloor + 1, YFloor    , ZFloor    );
    real32 V5 = GetFromNeighbours(Neighbours, BlockP, XFloor + 1, YFloor    , ZFloor + 1);
    real32 V6 = GetFromNeighbours(Neighbours, BlockP, XFloor + 1, YFloor + 1, ZFloor    );
    real32 V7 = GetFromNeighbours(Neighbours, BlockP, XFloor + 1, YFloor + 1, ZFloor + 1);
    
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
GetInterpolatedValueByResolution(terrain_density_block **Neighbours, world_block_pos *BlockP,
                                 int32 NewRes, real32 X, real32 Y, real32 Z, int32 origX, int32 origY, int32 origZ)
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
    
    real32 V0 = GetFromNeighbours(Neighbours, BlockP, XF0, YF0, ZF0);
    real32 V1 = GetFromNeighbours(Neighbours, BlockP, XF0, YF0, ZF1);
    real32 V2 = GetFromNeighbours(Neighbours, BlockP, XF0, YF1, ZF0);
    real32 V3 = GetFromNeighbours(Neighbours, BlockP, XF0, YF1, ZF1);
    real32 V4 = GetFromNeighbours(Neighbours, BlockP, XF1, YF0, ZF0);
    real32 V5 = GetFromNeighbours(Neighbours, BlockP, XF1, YF0, ZF1);
    real32 V6 = GetFromNeighbours(Neighbours, BlockP, XF1, YF1, ZF0);
    real32 V7 = GetFromNeighbours(Neighbours, BlockP, XF1, YF1, ZF1);
    
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
GetExactFromNeighbours(terrain_density_block **Neighbours,
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
    
    real32 Result = GetInterpolatedValueByResolution(Neighbours, BlockP, NewRes, NewResX, NewResY, NewResZ, X, Y, Z);
    
    return Result;
}

internal v3
GetPointNormal(terrain_density_block **Neighbours, world_block_pos *BlockP, v3 Point)
{
    real32 Diff = 0.5f;
    
    real32 DiffXMin = Point.X - Diff;
    real32 DiffXMax = Point.X + Diff;
    
    real32 DiffYMin = Point.Y - Diff;
    real32 DiffYMax = Point.Y + Diff;
    
    real32 DiffZMin = Point.Z - Diff;
    real32 DiffZMax = Point.Z + Diff;
    
    real32 XP = GetInterpolatedNeighbour(Neighbours, BlockP, DiffXMax, Point.Y, Point.Z);
    real32 XM = GetInterpolatedNeighbour(Neighbours, BlockP, DiffXMin, Point.Y, Point.Z);
    real32 NormalX = XP - XM;
    real32 YP = GetInterpolatedNeighbour(Neighbours, BlockP, Point.X, DiffYMax, Point.Z);
    real32 YM = GetInterpolatedNeighbour(Neighbours, BlockP, Point.X, DiffYMin, Point.Z);
    real32 NormalY = YP - YM;
    real32 ZP = GetInterpolatedNeighbour(Neighbours, BlockP, Point.X, Point.Y, DiffZMax);
    real32 ZM = GetInterpolatedNeighbour(Neighbours, BlockP, Point.X, Point.Y, DiffZMin);
    real32 NormalZ = ZP - ZM;
    
    v3 Result = v3{NormalX, NormalY, NormalZ};
    Result = Normalize(Result);
    return Result;
}

internal world_block_pos
GetBiggerResBlockPosition(world_block_pos *BlockP)
{
    world_block_pos Result = ConvertToResolution(BlockP, BlockP->Resolution*2);
    return Result;
}

internal void
GetLowerResBlockPositions(lower_blocks *LowerBlockPositions, world_block_pos *BlockP)
{
    int32 Coords[LowerBlockCount][3] = {{0, 0, 0},
                                        {1, 0, 0},
                                        {0, 1, 0},
                                        {1, 1, 0},
                                        {0, 0, 1},
                                        {1, 0, 1},
                                        {0, 1, 1},
                                        {1, 1, 1}};
    for(int32 i = 0; i < LowerBlockCount; i++)
    {
        LowerBlockPositions->Pos[i].Resolution = BlockP->Resolution/2;
        LowerBlockPositions->Pos[i].BlockX = BlockP->BlockX * 2 + Coords[i][0];
        LowerBlockPositions->Pos[i].BlockY = BlockP->BlockY * 2 + Coords[i][1];
        LowerBlockPositions->Pos[i].BlockZ = BlockP->BlockZ * 2 + Coords[i][2];
    }
}

internal void
GetNeighbourBlockPositionsOnSameRes(block_same_res_neighbours *NPositions, world_block_pos *CenterBlockP)
{
    uint32 Index = 0;
    for(int32 DiffX = -1; DiffX < 2; ++DiffX)
    {
        for(int32 DiffY = -1; DiffY < 2; ++DiffY)
        {
            for(int32 DiffZ = -1; DiffZ < 2; ++DiffZ)
            {
                world_block_pos NeighbourP = *CenterBlockP;
                NeighbourP.BlockX += DiffX;
                NeighbourP.BlockY += DiffY;
                NeighbourP.BlockZ += DiffZ;
                NPositions->Pos[Index++] = NeighbourP;
            }
        }
    }
    Assert(Index == NeighbourSameResCount);
}

internal void
GetNeighbourBlockPositionsOnLowerRes(block_lower_neighbours *NPositions, world_block_pos *CenterBlockP)
{
    world_block_pos SmallerResCenter = ConvertToResolution(CenterBlockP, CenterBlockP->Resolution/2);
    uint32 Index = 0;
    for(int32 DiffX = -1; DiffX < 3; ++DiffX)
    {
        for(int32 DiffY = -1; DiffY < 3; ++DiffY)
        {
            for(int32 DiffZ = -1; DiffZ < 3; ++DiffZ)
            {
                world_block_pos NeighbourP = SmallerResCenter;
                NeighbourP.BlockX += DiffX;
                NeighbourP.BlockY += DiffY;
                NeighbourP.BlockZ += DiffZ;
                NPositions->Pos[Index++] = NeighbourP;
            }
        }
    }
    Assert(Index == NeighbourLowerCount);
}

internal block_hash*
MapBlockPosition(world_density *World, world_block_pos *BlockP, int32 MappingValue)
{
    block_hash *ResHash = WriteHash(World->ResolutionMapping, BlockP, MappingValue);
    World->BlockMappedCount++;
    
    return ResHash;
}

internal block_hash*
MapBlockPositionAfterParent(world_density *World, world_block_pos *BlockP)
{
    world_block_pos BiggerP = GetBiggerResBlockPosition(BlockP);
    block_hash *BPResHash = GetHash(World->ResolutionMapping, &BiggerP);
    Assert(!HashIsEmpty(BPResHash));
    block_hash *ResHash = MapBlockPosition(World, BlockP, BPResHash->Index);
    return ResHash;
}

internal world_block_pos 
GetBiggerMappedPosition(world_density *World, world_block_pos *BlockP)
{
    world_block_pos Result = *BlockP;

    block_hash *ResHash = GetHash(World->ResolutionMapping, BlockP);
    if(HashIsEmpty(ResHash))
    {
        ResHash = MapBlockPositionAfterParent(World, BlockP);
    }
    // NOTE: If neighbour should be considered on another resolution, search for that resolution density.
    // If Index is smaller than our resolution, then we are trying to render from a smaller neighbour
    // which we doesn't handle here
    while(Result.Resolution != ResHash->Index)
    {
        Assert(ResHash->Index > Result.Resolution);
        Result = GetBiggerResBlockPosition(&Result);
    }
    
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

#define DENSITY_ISO_LEVEL 0.0f
internal void
PoligoniseBlock(world_density *World, terrain_render_block *RenderBlock, world_block_pos *BlockP)
{
    block_lower_neighbours NPositions;
    GetNeighbourBlockPositionsOnLowerRes(&NPositions, BlockP);
    terrain_density_block *Neighbours[ArrayCount(NPositions.Pos)];
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
                Cell.val[0] = GetExactFromNeighbours(Neighbours, BlockP, X  , Y+1, Z  );
                Cell.val[1] = GetExactFromNeighbours(Neighbours, BlockP, X  , Y+1, Z+1);
                Cell.val[2] = GetExactFromNeighbours(Neighbours, BlockP, X  , Y  , Z+1);
                Cell.val[3] = GetExactFromNeighbours(Neighbours, BlockP, X  , Y  , Z  );
                Cell.val[4] = GetExactFromNeighbours(Neighbours, BlockP, X+1, Y+1, Z  );
                Cell.val[5] = GetExactFromNeighbours(Neighbours, BlockP, X+1, Y+1, Z+1);
                Cell.val[6] = GetExactFromNeighbours(Neighbours, BlockP, X+1, Y  , Z+1);
                Cell.val[7] = GetExactFromNeighbours(Neighbours, BlockP, X+1, Y  , Z  );
                TRIANGLE Triangles[5];
                uint32 TriangleCount = Polygonise(Cell, DENSITY_ISO_LEVEL, Triangles);
                
                for(uint32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
                {
                    v3 Point0 = Triangles[TriangleIndex].p[0];
                    v3 Point1 = Triangles[TriangleIndex].p[1];
                    v3 Point2 = Triangles[TriangleIndex].p[2];
                                        
                    v3 Normal0 = GetPointNormal(Neighbours, BlockP, Point0);
                    v3 Normal1 = GetPointNormal(Neighbours, BlockP, Point1);
                    v3 Normal2 = GetPointNormal(Neighbours, BlockP, Point2);
                    
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









