/*
    Terep generátor by Hidvégi Máté @2017
*/


internal v3
GetPointNormal(terrain_density_block *Neighbours[], terrain_density_block *DynNeighbours[], 
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

#define DENSITY_ISO_LEVEL 0.0f
internal void
CreateRenderBlock(terrain *Terrain, terrain_render_block *RenderBlock, world_block_pos *BlockP)
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
        world_block_pos MappedP = GetBiggerMappedPosition(Terrain, NeighbourP);
        Assert((MappedP.Resolution >= BlockP->Resolution/2) &&
               (MappedP.Resolution <= BlockP->Resolution*2));
        
        block_hash *NeighbourHash = GetHash(Terrain->DensityHash, &MappedP);
        Assert(!HashIsEmpty(NeighbourHash));
        Neighbours[NeighbourIndex] = Terrain->DensityBlocks + NeighbourHash->Index;
        
        block_hash *DynamicHash = GetHash(Terrain->DynamicHash, &MappedP);
        Assert(!HashIsEmpty(DynamicHash));
        DynNeighbours[NeighbourIndex] = Terrain->DynamicBlocks + DynamicHash->Index;
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
                        Vertex(Point0 * CellDiff, Normal0, GreenColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Vertex(Point1 * CellDiff, Normal1, GreenColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Vertex(Point2 * CellDiff, Normal2, GreenColor);
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
                        Vertex(Pos * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Vertex((Pos + Normal0) * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Vertex((Pos + 0.9f*Normal0 + 0.1f*NormalPerpend) * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Vertex(Pos * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Vertex((Pos + 0.9f*Normal0 + 0.1f*NormalPerpend) * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Vertex((Pos + Normal0) * CellDiff, NormalNormal, BlueColor);
#endif
                }
            }
        }
    }
    RenderBlock->VertexCount = VertexCount;
    //logger::DebugPrint("Current Vertex Count: %d", VertexCount);
}


// NOTE: Add render block to the terrain 3d model for rendering
internal void
QueueForRendering(terrain *Terrain, terrain_block_model *Block, v3 CameraP, v3 CamDir)
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
            BlockIndex < Terrain->RenderBlockCount; 
            BlockIndex++)
        {
            terrain_block_model *RBlock = Terrain->RenderBlocks[BlockIndex];
            if(WorldPosEquals(&RBlock->WPos, &Block->WPos))
            {
                AlreadyHaveBlock = true;
            }
        }
        if(!AlreadyHaveBlock)
        {
            Terrain->RenderBlocks[Terrain->RenderBlockCount++] = Block;
            Assert(Terrain->RenderBlockCount < ArrayCount(Terrain->RenderBlocks));
        }
    }  
}

// NOTE: Delete block from render hash
internal void
DeleteRenderBlock(terrain *Terrain, int32 StoreIndex)
{
    terrain_block_model *Block = Terrain->PoligonisedBlocks + StoreIndex;
    terrain_block_model *Last = Terrain->PoligonisedBlocks + (--Terrain->PoligonisedBlockCount);
    world_block_pos BlockP = Block->WPos;
    world_block_pos LastP = Last->WPos;
    
    block_hash *RemovedHash = GetHash(Terrain->RenderHash, &BlockP);
    Assert(StoreIndex == RemovedHash->Index);
    Assert(!HashIsEmpty(RemovedHash));
    block_hash *LastHash = GetHash(Terrain->RenderHash, &LastP);
    Assert(!HashIsEmpty(LastHash));
    
    LastHash->Index = StoreIndex;
    RemovedHash->Index = HASH_DELETED;
    Terrain->DeletedRenderBlockCount++;
    
    // NOTE: If we are not deleting the last block, 
    // than we are filling the deleted blocks space with the contents of the last block
    if(StoreIndex != (int32)Terrain->PoligonisedBlockCount)
    {
        if(Block->Buffer) 
        {
            Block->Buffer->Release();
        }
        *Block = *Last;
    }
}

// NOTE: Delete block that was rendered, either as a render block or zero block
internal void
DeleteFromTerrainModel(terrain *Terrain, world_block_pos *BlockP)
{
    block_hash *RenderHash = GetHash(Terrain->RenderHash, BlockP);
    block_hash *ZeroHash = GetZeroHash(Terrain, BlockP);
    if(!HashIsEmpty(RenderHash))
    {
        DeleteRenderBlock(Terrain, RenderHash->Index);
    }
    if(!HashIsEmpty(ZeroHash))
    {
        ZeroHash->Index = HASH_DELETED;
        Terrain->ZeroBlockCount--;
    }
}

inline bool32 
IsBlockInTerrainModel(terrain *Terrain, world_block_pos *BlockP)
{
    bool32 Result = false;
    
    block_hash *RenderHash = GetHash(Terrain->RenderHash, BlockP);
    block_hash *ZeroHash = GetZeroHash(Terrain, BlockP);
    Result = !(HashIsEmpty(RenderHash) && HashIsEmpty(ZeroHash));
    
    return Result;
}

internal bool32
AreBlocksInTerrainModel(terrain *Terrain, world_block_pos *Positions, uint32 Count)
{
    bool32 Result = true;
    for(uint32 PosIndex = 0;
        PosIndex < Count;
        ++PosIndex)
    {
        world_block_pos *Pos = Positions + PosIndex;
        block_hash *RenderHash = GetHash(Terrain->RenderHash, Pos);
        block_hash *ZeroHash = GetZeroHash(Terrain, Pos);
        if(HashIsEmpty(RenderHash) && HashIsEmpty(ZeroHash))
        {
            Result = false;
        }
    }
    
    return Result;
}

