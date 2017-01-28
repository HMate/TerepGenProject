/*
    Terep generátor by Hidvégi Máté @2017
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
    
    DensityBlock->Grid.Dimension = GRID_DIMENSION;
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

internal bool32
DidDensityBlocksLoaded(terrain *Terrain, world_block_pos *Positions, uint32 Count)
{
    bool32 Result = true;
    for(uint32 PosIndex = 0;
        PosIndex < Count;
        ++PosIndex)
    {
        block_hash *DensityHash = GetHash(Terrain->DensityHash, Positions + PosIndex);
        Result = Result && !HashIsEmpty(DensityHash);
    }
    
    return Result;
}

internal bool32
DidBiggerMappedDensitiesLoad(terrain *Terrain, world_block_pos *Positions, uint32 Count)
{
    bool32 Result = true;
    for(uint32 PosIndex = 0;
        PosIndex < Count;
        ++PosIndex)
    {
        world_block_pos *Pos = Positions + PosIndex;
        block_hash *ResHash = GetHash(Terrain->ResolutionMapping, Pos);
        if(HashIsEmpty(ResHash))
        {
            ResHash = MapBlockPositionAfterParent(Terrain, Pos);
        }
        Assert(!HashIsEmpty(ResHash));
        Assert(ResHash->Index >= Pos->Resolution);
        
        world_block_pos MappedPos = GetAndSetBiggerMappedPosition(Terrain, Pos);
        block_hash *DensityHash = GetHash(Terrain->DensityHash, &MappedPos);
        Result = Result && !HashIsEmpty(DensityHash);
    }
    
    return Result;
}



// NOTE: XYZ are relative to the block position
internal real32
GetWorldGrid(terrain *Terrain, world_block_pos *BlockP, int32 X, int32 Y, int32 Z)
{
    block_node Node = GetActualBlockNode(BlockP, X, Y, Z);
    //block_hash *ResHash = GetHash(Terrain->ResolutionMapping, &Node.BlockP);
    
    block_hash *DensityHash = GetHash(Terrain->DensityHash, &Node.BlockP);
    // TODO: What if this block wasnt generated? 
    // maybe create an IsBlockValid(world_block_pos)->bool32 ?
    real32 Result = 0.0f; 
    if(!HashIsEmpty(DensityHash))
    {
        terrain_density_block *ActDensityBlock = Terrain->DensityBlocks + DensityHash->Index;
        Result = GetGrid(&ActDensityBlock->Grid, Node.X, Node.Y, Node.Z);
    }
    
    return Result;
}

// NOTE: floating values here are terrain grid positions cast to float, not render space values!
internal real32
GetInterpolatedWorldGrid(terrain *Terrain, world_block_pos *BlockP, 
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
        return GetWorldGrid(Terrain, BlockP, XFloor, YFloor, ZFloor);
    else if(XRemainder < 0.0001f && YRemainder < 0.0001f)
    {
        real32 Elem1 = GetWorldGrid(Terrain, BlockP, XFloor, YFloor, ZFloor);
        real32 Elem2 = GetWorldGrid(Terrain, BlockP, XFloor, YFloor, ZFloor + 1);
    
        real32 Result = Elem1 + ZRemainder * (Elem2 - Elem1);
        return Result;
    }
    else if(XRemainder < 0.0001f)
    {
        real32 Elem1 = GetInterpolatedWorldGrid(Terrain, BlockP, X, (real32)YFloor, Z);
        real32 Elem2 = GetInterpolatedWorldGrid(Terrain, BlockP, X, (real32)(YFloor+1), Z);
        
        real32 Result = Elem1 + YRemainder * (Elem2 - Elem1);
        return Result;
    }
    else
    {
        real32 Elem1 = GetInterpolatedWorldGrid(Terrain, BlockP, (real32)XFloor, Y, Z);
        real32 Elem2 = GetInterpolatedWorldGrid(Terrain, BlockP, (real32)(XFloor+1), Y, Z);
        
        real32 Result = Elem1 + XRemainder * (Elem2 - Elem1);
        return Result;
    }
}

// NOTE: V3 contains renderspace values here
internal real32
GetWorldGridValueFromV3(terrain *Terrain, v3 Pos, int32 Resolution)
{
    // TODO: Resolution
    world_block_pos WorldOrigo{0, 0, 0, Resolution};
    v3 BlockPos = Pos/((real32)Resolution * RENDER_SPACE_UNIT);
    
    real32 Result = GetInterpolatedWorldGrid(Terrain, &WorldOrigo, BlockPos.X, BlockPos.Y, BlockPos.Z);
    return Result;
}


// NOTE: Functions related to getting density values from nighbouring blocks

// NOTE: XYZ are relative to the block position
// Handles bigger resolution neighbours by rounding to smaller index
internal real32
GetFromNeighbours(terrain_density_block *Neighbours[], terrain_density_block *DynNeighbours[],
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
GetInterpolatedNeighbour(terrain_density_block *Neighbours[], terrain_density_block *DynNeighbours[], 
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
GetInterpolatedValueByResolution(terrain_density_block *Neighbours[], terrain_density_block *DynNeighbours[], 
                                 world_block_pos *BlockP, int32 NewRes, 
                                 real32 X, real32 Y, real32 Z, 
                                 int32 origX, int32 origY, int32 origZ)
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
GetExactFromNeighbours(terrain_density_block *Neighbours[], terrain_density_block *DynNeighbours[],
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
