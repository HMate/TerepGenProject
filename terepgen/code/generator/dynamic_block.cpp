

compressed_block *
CompressBlock(memory_arena *TranArena, terrain_density_block *Block)
{
    compressed_block *Result = PushStruct(TranArena, compressed_block);
    Result->Pos = Block->Pos;
    Result->NodeCount = 1;
    
    compressed_node *CurrentNode = PushStruct(TranArena, compressed_node);
    CurrentNode->Count = 1;
    CurrentNode->Value = Block->Grid.Elements[0];
    
    int32 GridSize = Block->Grid.Dimension * Block->Grid.Dimension * Block->Grid.Dimension;
        
    for(int32 Index = 1; 
        Index < GridSize; 
        Index++)
    {
        real32 Value = Block->Grid.Elements[Index];
        if(CurrentNode->Value != Value)
        {
            CurrentNode = PushStruct(TranArena, compressed_node);
            CurrentNode->Count = 0;
            CurrentNode->Value = Value;
            Result->NodeCount++;
        }
        CurrentNode->Count++;
    }
    return Result;
};

internal void
DecompressBlock(terrain_density_block *Block, compressed_block *Source)
{
    Block->Pos = Source->Pos;
    Block->Grid.Dimension = GRID_DIMENSION;
    int32 GridSize = Block->Grid.Dimension * Block->Grid.Dimension * Block->Grid.Dimension;
    uint32 NodesVisited = 0;
    uint32 ValueCount = 0;
    
    compressed_node *Node = Source->Nodes;
        
    for(int32 Index = 0; 
        Index < GridSize; 
        Index++)
    {
        Block->Grid.Elements[Index] = Node->Value;
        ValueCount++;
        if(Node->Count <= ValueCount)
        {
            Node = Node + 1;
            ValueCount = 0;
            NodesVisited++;
        }
    }
    Assert(NodesVisited == Source->NodeCount);
};

internal FileHandle
OpenBlocksFile(char* Filename, uint32 SessionId)
{
    FileHandle Handle = PlatformOpenOrCreateFileForWrite(Filename);
    if(FileIsEmpty(Handle))
    {
        // NOTE: The file didn't exist before, so now we create its header
        // NOTE: Write out SessionId
        uint32 *Data = &SessionId;
        uint32 Length = 4;
        PlatformWriteFile(Handle, Data, Length);
    }
    PlatformSetFilePosition(Handle, 0);
    return Handle;
}

internal void
ReadBlockFileHeader(FileHandle Handle, uint32 SessionId)
{
    // NOTE: Read SessionId
    char HeaderValue[4];
    uint32 HeaderLength = 4;
    uint32 BytesRead = PlatformReadFile(Handle, HeaderValue, HeaderLength);
    Assert(HeaderLength == BytesRead);
    
    uint32 GameID = *(uint32*)HeaderValue;
    Assert(GameID == SessionId);
}

inline compressed_block *
NextCompressedBlock(compressed_block *Current)
{
    compressed_block *Result = (compressed_block *)((uint8 *)Current + sizeof(compressed_block) + 
                    (sizeof(compressed_node) * Current->NodeCount));
    return Result;
}

internal bool32
CompressedBlockArrayContainsWorldPos(compressed_block *BlockArray, uint32 ArraySize, compressed_block *BlockToFind)
{
    compressed_block *Current = BlockArray;
    for(uint32 Index = 0; 
        Index < ArraySize; 
        Index++)
    {
        if(WorldPosEquals(&BlockToFind->Pos, &Current->Pos))
        {
            return true;
        }
        Current = NextCompressedBlock(Current);
    }
    return false;
}

void
SaveCompressedBlockArrayToFile(memory_arena *Arena, session_description *Session,
                               compressed_block *BlockArray, uint32 ArraySize)
{
    char *FileName = Session->DynamicStore;
    uint32 SessionId = Session->Id;
    char TempFileName[256];
    StringConcat(TempFileName, FileName, ".temp");
	
    FileHandle Handle = OpenBlocksFile(FileName, SessionId);
    FileHandle TempHandle = OpenBlocksFile(TempFileName, SessionId);
    ReadBlockFileHeader(Handle, SessionId);
    ReadBlockFileHeader(TempHandle, SessionId);
        
    //NOTE: Move blocks that are not modified to a new file
    bool32 EndOfFile = false;
    compressed_block ReadBlock = {0};
    const uint32 BlockSizeInBytes = sizeof(compressed_block);
    const uint32 NodeSizeInBytes = sizeof(compressed_node);
    while(!EndOfFile)
    {
        uint32 BytesRead = PlatformReadFile(Handle, &ReadBlock, BlockSizeInBytes);
        EndOfFile = (BytesRead == 0);
        Assert(BytesRead == BlockSizeInBytes || EndOfFile);
                
        if(!EndOfFile && !CompressedBlockArrayContainsWorldPos(BlockArray, ArraySize, &ReadBlock))
        {
            uint32 BytesWritten = PlatformWriteFile(TempHandle, &ReadBlock, BlockSizeInBytes);
            Assert(BytesWritten == BlockSizeInBytes);
            
            compressed_node *ReadData = PushArray(Arena, compressed_node, ReadBlock.NodeCount);
            uint32 DataSize = sizeof(compressed_node)*ReadBlock.NodeCount;
            BytesRead = PlatformReadFile(Handle, ReadData, DataSize);
            EndOfFile = (BytesRead == 0);
            Assert(BytesRead == DataSize);
            
            BytesWritten = PlatformWriteFile(TempHandle, ReadData, DataSize);
            Assert(BytesWritten == DataSize);
        }
        else if(!EndOfFile)
        {
            // NOTE: If this isn't the block we need, skip to next block
            PlatformIncrementFilePosition(Handle, sizeof(compressed_node)*ReadBlock.NodeCount);
        }
    }
    
    // NOTE: Now write out the modified blocks
    compressed_block *Current = BlockArray;
    for(uint32 Index = 0; Index < ArraySize; Index++)
    {
        uint32 BytesWritten = PlatformWriteFile(TempHandle, Current, BlockSizeInBytes);
        Assert(BytesWritten == BlockSizeInBytes);
        
        uint32 DataSize = sizeof(compressed_node)*Current->NodeCount;
        BytesWritten = PlatformWriteFile(TempHandle, Current->Nodes, DataSize);
        Assert(BytesWritten == DataSize);
        Current = NextCompressedBlock(Current);
    }
    
    PlatformCloseFile(Handle);
    PlatformCloseFile(TempHandle);
    
    PlatformDeleteFile(FileName);
    PlatformRenameFile(TempFileName, FileName);
}

// NOTE: Loads a block from a file
// If the block was not in the file, returns false
internal bool32
LoadCompressedBlockFromFile(memory_arena *Arena, session_description *Session,
                            terrain_density_block *DestinationBlock, world_block_pos *BlockP)
{
    FileHandle Handle = OpenBlocksFile(Session->DynamicStore, Session->Id);
    ReadBlockFileHeader(Handle, Session->Id);
    
    //NOTE: Read blocks until we find the one we need
    bool32 NotFound = true;
    bool32 EndOfFile = false;
    const uint32 BlockSizeInBytes = sizeof(compressed_block);
    compressed_block *ReadBlock = PushStruct(Arena, compressed_block);
    while(NotFound && !EndOfFile)
    {
        uint32 BytesRead = PlatformReadFile(Handle, ReadBlock, BlockSizeInBytes);
        EndOfFile = (BytesRead == 0);
        Assert(BytesRead == BlockSizeInBytes || EndOfFile);
        
        if(!EndOfFile && WorldPosEquals(&ReadBlock->Pos, BlockP))
        {
            compressed_node *ReadData = PushArray(Arena, compressed_node, ReadBlock->NodeCount);
            uint32 DataSize = sizeof(compressed_node)*ReadBlock->NodeCount;
            BytesRead = PlatformReadFile(Handle, ReadData, DataSize);
            Assert(BytesRead == DataSize);
            
            DecompressBlock(DestinationBlock, ReadBlock);
            
            NotFound = false;
        }
        
        // NOTE: If this isn't the block we need, skip to next block
        PlatformIncrementFilePosition(Handle, sizeof(compressed_node)*ReadBlock->NodeCount);
    }
    
    PlatformCloseFile(Handle);
    return !NotFound;
}


internal void 
FillDynamic(terrain_density_block *Dynamic, world_block_pos *BlockP, real32 Value)
{
    Dynamic->Pos = *BlockP;
    Dynamic->Grid.Dimension = GRID_DIMENSION;
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

// NOTE: Creates a dynamic block, or loads it from a file
// TODO: Separate loading in compressed blocks to a separate function, 
// and load in every block at once, if they are in the area of the camera
internal block_hash*
CreateNewDynamicBlock(memory_arena *Arena, terrain *Terrain, 
                      world_block_pos *BlockP, session_description *Session)
{
    terrain_density_block *DynamicB = Terrain->DynamicBlocks + Terrain->DynamicBlockCount;
    // NOTE: Load from file, if it was saved previously!
    // bool32 Loaded = LoadBlockFromFile(GameState, GameState->Session.DynamicStore, DynamicB, BlockP);
    bool32 Loaded = LoadCompressedBlockFromFile(Arena, Session, DynamicB, BlockP);
    if(!Loaded)
    {
        // TODO: If this block already have a lower resolution parent, values should be taken from there
        /*if(BlockP->Resolution < Terrain->FixedResolution[0])
        {
            // NOTE: Parent block should be existing at this pointer
            world_block_pos *BiggerP = GetBiggerMappedPosition(BlockP);
            block_hash *ParentHash = GetHash(Terrain->DynamicHash, BiggerP);
            Assert(!HashIsEmpty(DynamicHash));
            terrain_density_block *Parent = Terrain->DynamicBlocks + ParentHash->Index;
            
            uint32 XOffset = (BlockP->BlockX - Parent->BlockX*2) * 4;
            uint32 YOffset = (BlockP->BlockY - Parent->BlockY*2) * 4;
            uint32 ZOffset = (BlockP->BlockZ - Parent->BlockZ*2) * 4;
                        
            Dynamic->Pos = *BlockP;
            uint32 Dim = Dynamic->Grid.Dimension;
            for(uint32 X = 0; X < Dim; X++)
            {
                for(uint32 Y = 0; Y < Dim; Y++)
                {
                    for(uint32 Z = 0; Z < Dim; Z++)
                    {
                        // TODO: Instead of loading neighbour block, just include one value from neighbour in this block
                        // this means we have to rewrite every block indexing
                        real32 G000 = GetGrid(&Parent->Grid, XOffset+X/2  , YOffset+Y/2  , ZOffset+Z/2  );
                        real32 G001 = GetGrid(&Parent->Grid, XOffset+X/2  , YOffset+Y/2  , ZOffset+Z/2+1);
                        real32 G010 = GetGrid(&Parent->Grid, XOffset+X/2  , YOffset+Y/2+1, ZOffset+Z/2  );
                        real32 G011 = GetGrid(&Parent->Grid, XOffset+X/2  , YOffset+Y/2+1, ZOffset+Z/2+1);
                        real32 G100 = GetGrid(&Parent->Grid, XOffset+X/2+1, YOffset+Y/2  , ZOffset+Z/2  );
                        real32 G101 = GetGrid(&Parent->Grid, XOffset+X/2+1, YOffset+Y/2  , ZOffset+Z/2+1);
                        real32 G110 = GetGrid(&Parent->Grid, XOffset+X/2+1, YOffset+Y/2+1, ZOffset+Z/2  );
                        real32 G111 = GetGrid(&Parent->Grid, XOffset+X/2+1, YOffset+Y/2+1, ZOffset+Z/2+1);
                        
                        SetGrid(&Dynamic->Grid, X, Y, Z, Value);
                    }
                }
            }
            
        }
        else*/
        {
            FillDynamic(DynamicB, BlockP, 0.0f);
        }
    }
    Assert(Terrain->DynamicBlockCount < ArrayCount(Terrain->DynamicBlocks));
    block_hash *DynamicHash = WriteHash(Terrain->DynamicHash, BlockP, Terrain->DynamicBlockCount++);
    
    return DynamicHash;
}

internal terrain_density_block*
GetDynamicBlock(memory_arena *Arena, terrain *Terrain, world_block_pos *BlockP,
                session_description *Session)
{
    block_hash *DynamicHash = GetHash(Terrain->DynamicHash, BlockP);
    if(HashIsEmpty(DynamicHash))
    {
        DynamicHash = CreateNewDynamicBlock(Arena, Terrain, BlockP, Session);
    }
    terrain_density_block *Result = Terrain->DynamicBlocks + DynamicHash->Index;
    return Result;
}