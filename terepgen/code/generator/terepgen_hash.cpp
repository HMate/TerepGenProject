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
    World->DynamicBlockCount = 0;
    World->PoligonisedBlockCount = 0;
    World->DeletedDensityBlockCount = 0;
    World->DeletedDynamicBlockCount = 0;
    World->DeletedRenderBlockCount = 0;
    
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(World->DensityHash);
        ++HashIndex)
    {
        block_hash *Hash = World->DensityHash + HashIndex;
        Hash->Index = HASH_UNINITIALIZED;
    }
    
    for(uint32 HashIndex = 0;
        HashIndex < ArrayCount(World->DynamicHash);
        ++HashIndex)
    {
        block_hash *Hash = World->DynamicHash + HashIndex;
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

internal void
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