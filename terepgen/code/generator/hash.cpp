/*
    Terep generátor by Hidvégi Máté @2016
*/

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
GetZeroHash(terrain *Terrain, world_block_pos *P)
{
    block_hash *Result = 0;

    uint32 HashValue = GetZeroHashValue(P);
    uint32 HashMask = (ArrayCount(Terrain->ZeroHash) - 1);
    
    for(uint32 Offset = 0;
        Offset < ArrayCount(Terrain->ZeroHash);
        ++Offset)
    {
        uint32 HashIndex = (HashValue + Offset) & HashMask;
        Assert(HashIndex < ArrayCount(Terrain->ZeroHash));
        block_hash *Hash = Terrain->ZeroHash + HashIndex;
        
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
WriteZeroHash(terrain *Terrain, world_block_pos *P)
{
    block_hash *Result = 0;

    uint32 HashValue = GetZeroHashValue(P);
    uint32 HashMask = (ArrayCount(Terrain->ZeroHash) - 1);
    
    for(uint32 Offset = 0;
        Offset < ArrayCount(Terrain->ZeroHash);
        ++Offset)
    {
        uint32 HashIndex = (HashValue + Offset) & HashMask;
        Assert(HashIndex < ArrayCount(Terrain->ZeroHash));
        block_hash *Hash = Terrain->ZeroHash + HashIndex;
        
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
InitResolutionMapping(terrain *Terrain)
{
    Terrain->BlockMappedCount = 0;
    
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(Terrain->ResolutionMapping);
        ++HashIndex)
    {
        block_hash *Hash = Terrain->ResolutionMapping + HashIndex;
        Hash->Index = HASH_UNINITIALIZED;
    }
}

internal void
InitBlockHash(terrain *Terrain)
{
    // TODO: Does zeroing out stored block count belong here?
    Terrain->DensityBlockCount = 0;
    Terrain->DynamicBlockCount = 0;
    Terrain->PoligonisedBlockCount = 0;
    Terrain->DeletedDensityBlockCount = 0;
    Terrain->DeletedDynamicBlockCount = 0;
    Terrain->DeletedRenderBlockCount = 0;
    
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(Terrain->DensityHash);
        ++HashIndex)
    {
        block_hash *Hash = Terrain->DensityHash + HashIndex;
        Hash->Index = HASH_UNINITIALIZED;
    }
    
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(Terrain->DynamicHash);
        ++HashIndex)
    {
        block_hash *Hash = Terrain->DynamicHash + HashIndex;
        Hash->Index = HASH_UNINITIALIZED;
    }
    
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(Terrain->RenderHash);
        ++HashIndex)
    {
        block_hash *Hash = Terrain->RenderHash + HashIndex;
        Hash->Index = HASH_UNINITIALIZED;
    }
    InitResolutionMapping(Terrain);
}

internal void
InitZeroHash(terrain *Terrain)
{
    Terrain->ZeroBlockCount = 0;
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(Terrain->ZeroHash);
        ++HashIndex)
    {
        block_hash *Hash = Terrain->ZeroHash + HashIndex;
        Hash->Index = HASH_UNINITIALIZED;
    }
}