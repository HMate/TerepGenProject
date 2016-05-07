/*
    Terep generátor by Hidvégi Máté @2016
*/

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

// TODO: Deleting blocks shouldn't be here
internal void 
DowngradeMapping(world_density *World, world_block_pos *BlockP, int32 MappingValue, 
                 world_block_pos *DeleteQueue, int32 *DeleteCount)
{
    uint32 ResIndex = GetResolutionIndex(BlockP->Resolution);
    if(ResIndex < RESOLUTION_COUNT-1)
    {
        lower_blocks LowerBlocks;
        GetLowerResBlockPositions(&LowerBlocks, BlockP);
        for(int32 LowerIndex = 0;
            LowerIndex < ArrayCount(LowerBlocks.Pos);
            LowerIndex++)
        {
            world_block_pos *LowerP = LowerBlocks.Pos + LowerIndex;
            DowngradeMapping(World, LowerP, MappingValue, DeleteQueue, DeleteCount);
        }
        block_hash *ResHash = GetHash(World->ResolutionMapping, BlockP);
        if(!HashIsEmpty(ResHash))
        {
            Assert(ResHash->Index < MappingValue);
            ResHash->Index = MappingValue;
        }
        DeleteQueue[*DeleteCount] = *BlockP;
        (*DeleteCount)++;
    }
}

internal void
UpdateLowerBlocksMapping(world_density *World, world_block_pos *BlockP, int32 ResMapping)
{
    uint32 ResIndex = GetResolutionIndex(BlockP->Resolution);
    if(ResIndex < RESOLUTION_COUNT-1)
    {
        lower_blocks LowerBlocks;
        GetLowerResBlockPositions(&LowerBlocks, BlockP);
        for(int32 LowerIndex = 0;
            LowerIndex < ArrayCount(LowerBlocks.Pos);
            LowerIndex++)
        {
            world_block_pos *LowerP = LowerBlocks.Pos + LowerIndex;
            block_hash *ResHash = GetHash(World->ResolutionMapping, LowerP);
            if(HashIsEmpty(ResHash))
            {
                MapBlockPosition(World, LowerP, ResMapping);
            }
            else
            {
                ResHash->Index = ResMapping;
            }
            Assert(ResHash->Index == ResMapping);
        }
    }
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
