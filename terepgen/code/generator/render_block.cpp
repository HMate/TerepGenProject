/*
    Terep generátor by Hidvégi Máté @2017
*/

internal void
AddToRenderBlocks(world_density *World, terrain_render_block *Block, v3 CameraP, v3 CamDir)
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
            BlockIndex < World->RenderBlockCount; 
            BlockIndex++)
        {
            terrain_render_block *RBlock = World->RenderBlocks[BlockIndex];
            if(WorldPosEquals(&RBlock->WPos, &Block->WPos))
            {
                AlreadyHaveBlock = true;
            }
        }
        if(!AlreadyHaveBlock)
        {
            World->RenderBlocks[World->RenderBlockCount++] = Block;
            Assert(World->RenderBlockCount < ArrayCount(World->RenderBlocks));
        }
    }  
}

// NOTE: Delete block from render hash
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
    
    // NOTE: If we are not deleting the last block, 
    // than we are filling the deleted blocks space with the contents of the last block
    if(StoreIndex != (int32)World->PoligonisedBlockCount)
    {
        *Block = *Last;
    }
}

// NOTE: Delete block that was rendered, either as a render block or zero block
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

inline bool32 
BlockWasRendered(world_density *World, world_block_pos *BlockP)
{
    bool32 Result = false;
    
    block_hash *RenderHash = GetHash(World->RenderHash, BlockP);
    block_hash *ZeroHash = GetZeroHash(World, BlockP);
    Result = !(HashIsEmpty(RenderHash) && HashIsEmpty(ZeroHash));
    
    return Result;
}

internal bool32
BlocksWereRendered(world_density *World, world_block_pos *Positions, uint32 Count)
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

