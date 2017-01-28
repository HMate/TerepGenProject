/*
    Terep generátor by Hidvégi Máté @2015
*/

void InitializeTerrain(terrain *Terrain)
{
    InitBlockHash(Terrain);
    InitZeroHash(Terrain);
}

void SaveTerrain(memory_arena* TranArena, terrain* Terrain, session_description* Session)
{
    compressed_block *CompressedBlocks = 0;
    for(uint32 Index = 0; 
        Index < Terrain->DynamicBlockCount; 
        Index++)
    {
        compressed_block *Compressed = CompressBlock(TranArena, Terrain->DynamicBlocks + Index);
        if(CompressedBlocks == 0)
        {
            CompressedBlocks = Compressed;
        }
    }
    if(CompressedBlocks != 0)
    {
        SaveCompressedBlockArrayToFile(TranArena, Session, CompressedBlocks, Terrain->DynamicBlockCount);
    }
}





