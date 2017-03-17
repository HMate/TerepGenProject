/*
    Terep generátor by Hidvégi Máté @2017

*/
#include "generator.h"

#include "..\terepgen_units.cpp"
#include "hash.cpp"
#include "marching_cubes.cpp"
#include "resolutions.cpp"
#include "density_block.cpp"
#include "dynamic_block.cpp"
#include "render_block.cpp"
#include "terrain.cpp"

// Select which blocks can be potentially generated this frame / 
// which blocks are seen at this resolution
internal void 
FillBlockPositions(block_pos_array *PosArray, uint32 MaxArraySize, 
                    world_block_pos *CentralBlockPos, int32 Radius)
{
    PosArray->Count = 0;
    for(int32 Dist = 0;
        Dist <= Radius;
        Dist++)
    {
        int32 YSign = 1, YDiff = 1, YIndex = 0;
        while(Abs(YIndex) <= Dist)
        {
            int32 XSign = 1, XDiff = 1, XIndex = 0;
            while(Abs(XIndex) <= Dist-Abs(YIndex))
            {
                int32 ZIndex = Dist-Abs(XIndex)-Abs(YIndex);
                int32 ZSign = -1;
                int32 ZDiff = (ZIndex) ? 2*ZIndex : 1;
                while(Abs(ZIndex) <= Dist-Abs(XIndex)-Abs(YIndex))
                {
                    PosArray->Pos[PosArray->Count].BlockX = CentralBlockPos->BlockX + XIndex;
                    PosArray->Pos[PosArray->Count].BlockY = CentralBlockPos->BlockY + YIndex;
                    PosArray->Pos[PosArray->Count].BlockZ = CentralBlockPos->BlockZ + ZIndex;
                    PosArray->Pos[PosArray->Count].Resolution = CentralBlockPos->Resolution;
                    PosArray->Count++;
                    
                    ZIndex += ZSign * (ZDiff++);
                    ZSign *= -1;
                }
                
                XIndex += XSign * (XDiff++);
                XSign *= -1;
            }
            YIndex += YSign * (YDiff++);
            YSign *= -1;
        }
    }
    
    Assert(PosArray->Count <= MaxArraySize);
}

// Determines which block contains the camera in every resolution
generator_position CalculateTerrainGeneratorPositon(terrain* Terrain, v3 CameraP)
{
    generator_position Result;
    Result.Centers[0] = WorldPosFromV3(CameraP, Terrain->FixedResolution[0]);
    Result.Centers[1] = WorldPosFromV3(CameraP, Terrain->FixedResolution[1]);
    Result.Centers[2] = WorldPosFromV3(CameraP, Terrain->FixedResolution[2]);
    
    FillBlockPositions(Terrain->RenderPositionStore, 
                        ArrayCount(Terrain->RenderPositionStore->Pos), 
                        Result.Centers, RENDERED_BLOCK_RADIUS);
    FillBlockPositions(Terrain->RenderPositionStore + 1, 
                        ArrayCount(Terrain->RenderPositionStore->Pos), 
                        Result.Centers + 1, RENDERED_BLOCK_RADIUS);
    
    FillBlockPositions((block_pos_array*)Terrain->DensityPositionStore, 
                        ArrayCount(Terrain->DensityPositionStore->Pos), 
                        Result.Centers, DENSITY_BLOCK_RADIUS);
    FillBlockPositions((block_pos_array*)(Terrain->DensityPositionStore + 1), 
                        ArrayCount(Terrain->DensityPositionStore->Pos), 
                        Result.Centers + 1, DENSITY_BLOCK_RADIUS);
    return Result;
}

internal int32
ManhattanDistance(world_block_pos *A, world_block_pos *B)
{
    int32 Result = 0;
    Result = Abs(A->BlockX - B->BlockX) + Abs(A->BlockY - B->BlockY) + Abs(A->BlockZ - B->BlockZ);
    return Result;
}

internal bool32
DoRectangleContains(world_block_pos *Center, int32 Radius, world_block_pos *P)
{
    bool32 Result = (Center->BlockX + Radius > P->BlockX) &&
                    (Center->BlockX - Radius < P->BlockX) &&
                    (Center->BlockY + Radius > P->BlockY) &&
                    (Center->BlockY - Radius < P->BlockY) &&
                    (Center->BlockZ + Radius > P->BlockZ) &&
                    (Center->BlockZ - Radius < P->BlockZ);
    return Result;
}

// NOTE: Delete blocks that are too far from the generator center
void ClearFarawayBlocks(memory_arena *Arena, terrain* Terrain, 
                        session_description * Session,
                        generator_position *GeneratorPos)
{
    world_block_pos *Center = (world_block_pos *)GeneratorPos;
    // TODO: Maybe we need to reinitialize the block hash, if there are too many deleted blocks?
    // Clear density blocks
    if(Terrain->DensityBlockCount > (ArrayCount(Terrain->DensityBlocks) - 100))
    {
        int32 LoadSpaceRadius = DENSITY_BLOCK_RADIUS + 1;
        for(uint32 StoreIndex = 0; 
            StoreIndex < Terrain->DensityBlockCount; 
            ++StoreIndex)
        {
            terrain_density_block *Block = Terrain->DensityBlocks + StoreIndex;
            world_block_pos *BlockP = &Block->Pos;
            uint32 ResIndex = GetResolutionIndex(BlockP->Resolution);
            // TODO: Maybe instead of checking manhattan distance, we need bigger hash and arrays
            if(ManhattanDistance(Center + ResIndex, BlockP) > LoadSpaceRadius)
            {
                terrain_density_block *Last = Terrain->DensityBlocks + (--Terrain->DensityBlockCount);
                world_block_pos *LastP = &Last->Pos;
                
                block_hash *RemovedHash = GetHash(Terrain->DensityHash, BlockP);
                Assert(!HashIsEmpty(RemovedHash));
                block_hash *LastHash = GetHash(Terrain->DensityHash, LastP);
                Assert(!HashIsEmpty(LastHash));
                
                LastHash->Index = StoreIndex;
                RemovedHash->Index = HASH_DELETED;
                Terrain->DeletedDensityBlockCount++;
                
                if(StoreIndex != (int32)Terrain->DensityBlockCount)
                {
                    *Block = *Last;
                }
            }
        }
    }
    
    // Clear dynamic blocks
    uint32 DynamicClearThreshold = (ArrayCount(Terrain->DynamicBlocks) - 100);
    if(Terrain->DynamicBlockCount > DynamicClearThreshold)
    {
        logger::DebugPrint("Clearing Dynamic Blocks! count: %d", Terrain->DynamicBlockCount);
        
        uint32 SaveCount = 0;
        // terrain_density_block *SavedBlocks = 0;
        compressed_block *SavedBlocks = 0;
        
        int32 LoadSpaceRadius = DENSITY_BLOCK_RADIUS + 1;
        for(uint32 StoreIndex = 0; 
            StoreIndex < Terrain->DynamicBlockCount; 
            ++StoreIndex)
        {
            terrain_density_block *Block = Terrain->DynamicBlocks + StoreIndex;
            world_block_pos *BlockP = &Block->Pos;
            uint32 ResIndex = GetResolutionIndex(BlockP->Resolution);
            // NOTE: Check manhattan distance, or need bigger hash and arrays
            if(ManhattanDistance(Center + ResIndex, BlockP) > LoadSpaceRadius)
            {                
                compressed_block *Compressed = CompressBlock(Arena, Block);
                if(SavedBlocks == 0)
                {
                    SavedBlocks = Compressed;
                }
                SaveCount++;
                
                terrain_density_block *Last = Terrain->DynamicBlocks + (--Terrain->DynamicBlockCount);
                world_block_pos *LastP = &Last->Pos;
                
                block_hash *RemovedHash = GetHash(Terrain->DynamicHash, BlockP);
                Assert(!HashIsEmpty(RemovedHash));
                block_hash *LastHash = GetHash(Terrain->DynamicHash, LastP);
                Assert(!HashIsEmpty(LastHash));
                
                LastHash->Index = StoreIndex;
                RemovedHash->Index = HASH_DELETED;
                Terrain->DeletedDynamicBlockCount++;
                
                if(StoreIndex != (int32)Terrain->DynamicBlockCount)
                {
                    *Block = *Last;
                }
            }
        }
        
        if(SavedBlocks != 0)
        {
            SaveCompressedBlockArrayToFile(Arena, Session,
                                           SavedBlocks, SaveCount);
        }
    }
    
    // Clear render blocks
    uint32 PoligonisedBlockClearThreshold = (ArrayCount(Terrain->PoligonisedBlocks) - 100);
    if(Terrain->PoligonisedBlockCount > PoligonisedBlockClearThreshold)
    {
        int32 LoadSpaceRadius = RENDERED_BLOCK_RADIUS + 2;
        for(uint32 StoreIndex = 0; 
            StoreIndex < Terrain->PoligonisedBlockCount; 
            ++StoreIndex)
        {
            terrain_block_model *Block = Terrain->PoligonisedBlocks + StoreIndex;
            world_block_pos BlockP = WorldPosFromV3(Block->Pos, Terrain->FixedResolution[0]);
            uint32 ResIndex = GetResolutionIndex(BlockP.Resolution);
            if(!DoRectangleContains(Center + ResIndex, LoadSpaceRadius, &BlockP))
            {
                DeleteRenderBlock(Terrain, StoreIndex);
            }
        }
    }
    
    // Clear resolution mapping blocks
    if(Terrain->BlockMappedCount > (ArrayCount(Terrain->ResolutionMapping) * 7/8))
    {
        int32 LoadSpaceRadius = DENSITY_BLOCK_RADIUS + 1;
        
        block_hash *NewMappingHash = PushArray(Arena, block_hash, BLOCK_HASH_SIZE);
        for(uint32 StoreIndex = 0; 
            StoreIndex < ArrayCount(Terrain->ResolutionMapping); 
            ++StoreIndex)
        {
            NewMappingHash[StoreIndex] = Terrain->ResolutionMapping[StoreIndex];
        }
        
        InitResolutionMapping(Terrain);
        
        for(uint32 StoreIndex = 0; 
            StoreIndex < ArrayCount(Terrain->ResolutionMapping); 
            ++StoreIndex)
        {
            block_hash *Hash = NewMappingHash + StoreIndex;
            world_block_pos *HashP = &Hash->Key;
            uint32 ResIndex = GetResolutionIndex(Hash->Key.Resolution);
            if(DoRectangleContains(Center + ResIndex, LoadSpaceRadius, HashP) &&
               Hash->Index > 0)
            {
                MapBlockPosition(Terrain, HashP, Hash->Index);
            }
        }
    }
    
    // Clear Zero blocks
    int32 ZeroGridTotalSize = POS_GRID_SIZE(ZERO_BLOCK_RADIUS);
    Assert(ZeroGridTotalSize < ZERO_HASH_SIZE);
    if(Terrain->ZeroBlockCount > (ArrayCount(Terrain->ZeroHash)*7/8))
    {
        int32 ZeroSpaceRadius = ZERO_BLOCK_RADIUS;
        block_hash *NewZeroHash = PushArray(Arena, block_hash, ZERO_HASH_SIZE);
        for(uint32 ZeroIndex = 0; 
            ZeroIndex < ArrayCount(Terrain->ZeroHash); 
            ++ZeroIndex)
        {
            NewZeroHash[ZeroIndex] = Terrain->ZeroHash[ZeroIndex];
        }
        
        InitZeroHash(Terrain);
        
        for(uint32 ZeroIndex = 0; 
            ZeroIndex < ArrayCount(Terrain->ZeroHash); 
            ++ZeroIndex)
        {
            block_hash *Entry = NewZeroHash + ZeroIndex;
            world_block_pos *ZeroP = &Entry->Key;
            uint32 ResIndex = GetResolutionIndex(ZeroP->Resolution);
            if((Entry->Index == HASH_ZERO_BLOCK) && 
               !DoRectangleContains(Center + ResIndex, ZeroSpaceRadius, ZeroP))
            {
                block_hash *ZeroHash = WriteZeroHash(Terrain, ZeroP);
                Terrain->ZeroBlockCount++;
            }
        }
    }
}


internal void
QueueBlockToRender(terrain *Terrain, world_block_pos *BlockP, 
    int32 *BlocksToGenerate, density_block_pos_array *BlocksToRender)
{
    block_lower_neighbours Neighbours;
    GetNeighbourBlockPositionsOnLowerRes(&Neighbours, BlockP);
    bool32 DidLoad = DidBiggerMappedDensitiesLoad(Terrain, Neighbours.Pos, ArrayCount(Neighbours.Pos));
    if(DidLoad)
    {
        (*BlocksToGenerate)--;
        const int32 Size = ArrayCount(BlocksToRender->Pos);
        Assert(BlocksToRender->Count < Size);
        BlocksToRender->Pos[BlocksToRender->Count++] = *BlockP;
    }
}

internal bool32
IsAffectedByDeformer(block_deformer *Deformer, block_node *Node)
{
    bool32 Result = false;
    if(Deformer->Type == DeformerTypeSphere || Deformer->Type == DeformerTypeGradualSphere)
    {
        v3 NodeRenderP = ConvertBlockNodeToRenderPos(Node);
        v3 Diff = NodeRenderP - Deformer->Center;
        real32 DistanceFromClick = Length(Diff);
        Result = DistanceFromClick < Deformer->Radius;
    }
    else if(Deformer->Type == DeformerTypeCube)
    {
        v3 NodeRenderP = ConvertBlockNodeToRenderPos(Node);
        v3 Diff = NodeRenderP - Deformer->Center;
        Result = Abs(Diff.X) < Deformer->Radius && 
                 Abs(Diff.Y) < Deformer->Radius &&
                 Abs(Diff.Z) < Deformer->Radius;
    }
    else Assert(!"Invalid code path");
    
    return Result;
}

internal real32
ChangeDynamicValue(block_deformer *Deformer, real32 OldVal, block_node *Node)
{
    real32 Result = 0.0f;
    if(Deformer->Type == DeformerTypeSphere || Deformer->Type == DeformerTypeCube)
    {
        Result = OldVal + Deformer->Sign * Deformer->Strength;
    }
    else if(Deformer->Type == DeformerTypeGradualSphere)
    {
        v3 NodeRenderP = ConvertBlockNodeToRenderPos(Node);
        v3 Diff = NodeRenderP - Deformer->Center;
        real32 DistanceFromClick = Length(Diff);
        real32 Scale = (Deformer->Radius - DistanceFromClick) / Deformer->Radius;
        Result = OldVal + Scale * Deformer->Sign * Deformer->Strength;
    }
    else
    {
        Assert(!"Invalid code path");
    }
    return Result;
}

internal void 
DeformBlocks(terrain *Terrain, memory_arena *Arena, block_deformer *Deformer, 
             density_block_pos_array *BlocksToRender, int32 *MaxRenderBlocksToGenerateInFrame,
             session_description *Session)
{
    v3 StartBlockRP = Deformer->Center - v3{Deformer->Radius, Deformer->Radius, Deformer->Radius};
    v3 EndBlockRP = Deformer->Center + v3{Deformer->Radius, Deformer->Radius, Deformer->Radius};
    block_node StartNode = ConvertRenderPosToBlockNode(StartBlockRP, Terrain->FixedResolution[0]);
    block_node EndNode = ConvertRenderPosToBlockNode(EndBlockRP, Terrain->FixedResolution[0]);

    uint32 BlocksTouched = 0;
    block_node Node = StartNode;
    for(uint32 XIndex = 0;
        (Node.BlockP.BlockX != EndNode.BlockP.BlockX) || (Node.X != EndNode.X);
        XIndex++)
    {
        Node = GetActualBlockNode(&StartNode.BlockP, 
                    StartNode.X+XIndex, StartNode.Y, StartNode.Z);
        for(uint32 YIndex = 0;
            (Node.BlockP.BlockY != EndNode.BlockP.BlockY) || (Node.Y != EndNode.Y);
            YIndex++)
        {
            Node = GetActualBlockNode(&StartNode.BlockP, 
                        StartNode.X+XIndex, StartNode.Y+YIndex, StartNode.Z);
            for(uint32 ZIndex = 0; 
                (Node.BlockP.BlockZ != EndNode.BlockP.BlockZ) || (Node.Z != EndNode.Z);
                ZIndex++)
            {
                Node = GetActualBlockNode(&StartNode.BlockP, 
                            StartNode.X+XIndex, StartNode.Y+YIndex, StartNode.Z+ZIndex);
                // NOTE: Change node density and rerender the render block
                if(IsAffectedByDeformer(Deformer, &Node))
                {
                    terrain_density_block *ActDynamicBlock = GetDynamicBlock(Arena, Terrain, &Node.BlockP,
                                                                            Session);
                    real32 GridVal = GetGrid(&ActDynamicBlock->Grid, Node.X, Node.Y, Node.Z);
                    real32 ChangedGridVal = ChangeDynamicValue(Deformer, GridVal, &Node);
                    SetGrid(&ActDynamicBlock->Grid, Node.X, Node.Y, Node.Z, ChangedGridVal);
                    
                    // NOTE: Have to rerender all negihbours too beacuse of tears in geometry
                    block_same_res_neighbours Neighbours;
                    GetNeighbourBlockPositionsOnSameRes(&Neighbours, &Node.BlockP);
                    for(uint32 NIndex = 0;
                        NIndex < ArrayCount(Neighbours.Pos);
                        NIndex++)
                    {
                        world_block_pos *NPos = Neighbours.Pos + NIndex;
                        bool32 AlreadyHave = false;
                        for(uint32 RenderIndex = 0;
                            RenderIndex < BlocksToRender->Count;
                            RenderIndex++)
                        {
                            world_block_pos *RenderP = BlocksToRender->Pos + RenderIndex;
                            if(WorldPosEquals(RenderP, NPos))
                            {
                                AlreadyHave = true;
                            }
                        }
                        if(!AlreadyHave)
                        {
                            QueueBlockToRender(Terrain, NPos, 
                                MaxRenderBlocksToGenerateInFrame, 
                                BlocksToRender);
                            BlocksTouched++;
                        }
                    }
                }
            }
        }
    }
}

internal void
AddCube(cube *Cube, v3 WorldMousePos, real32 Size,
        v4 ColorR, v4 ColorG, v4 ColorB)
{
    const v3 Corner0 = WorldMousePos + v3{-0.5f, -0.5f, -0.5f}*Size;
    const v3 Corner1 = WorldMousePos + v3{-0.5f, 0.5f, -0.5f}*Size;
    const v3 Corner2 = WorldMousePos + v3{0.5f, -0.5f, -0.5f}*Size;
    const v3 Corner3 = WorldMousePos + v3{0.5f, 0.5f, -0.5f}*Size;
    const v3 Corner4 = WorldMousePos + v3{-0.5f, -0.5f, 0.5f}*Size;
    const v3 Corner5 = WorldMousePos + v3{-0.5f, 0.5f, 0.5f}*Size;
    const v3 Corner6 = WorldMousePos + v3{0.5f, -0.5f, 0.5f}*Size;
    const v3 Corner7 = WorldMousePos + v3{0.5f, 0.5f, 0.5f}*Size;
    
    const v3 Normal0 = v3{0.0f, 0.0f, -1.0f}; //front
    const v3 Normal1 = v3{1.0f, 0.0f, 0.0f};  //right
    const v3 Normal2 = v3{0.0f, 0.0f, 1.0f};  //back
    const v3 Normal3 = v3{-1.0f, 0.0f, 0.0f}; //left
    const v3 Normal4 = v3{0.0f, -1.0f, 0.0f}; // down
    const v3 Normal5 = v3{0.0f, 1.0f, 0.0f};  //up
    
    Cube->Vertices[0] = Vertex(Corner0, Normal0, ColorB);
    Cube->Vertices[1] = Vertex(Corner1, Normal0, ColorB);
    Cube->Vertices[2] = Vertex(Corner2, Normal0, ColorB);
    Cube->Vertices[3] = Vertex(Corner2, Normal0, ColorB);
    Cube->Vertices[4] = Vertex(Corner1, Normal0, ColorB);
    Cube->Vertices[5] = Vertex(Corner3, Normal0, ColorB);
    
    Cube->Vertices[6] = Vertex(Corner2, Normal1, ColorR);
    Cube->Vertices[7] = Vertex(Corner3, Normal1, ColorR);
    Cube->Vertices[8] = Vertex(Corner6, Normal1, ColorR);
    Cube->Vertices[9] = Vertex(Corner6, Normal1, ColorR);
    Cube->Vertices[10] = Vertex(Corner3, Normal1, ColorR);
    Cube->Vertices[11] = Vertex(Corner7, Normal1, ColorR);
    
    Cube->Vertices[12] = Vertex(Corner6, Normal2, ColorB);
    Cube->Vertices[13] = Vertex(Corner7, Normal2, ColorB);
    Cube->Vertices[14] = Vertex(Corner4, Normal2, ColorB);
    Cube->Vertices[15] = Vertex(Corner4, Normal2, ColorB);
    Cube->Vertices[16] = Vertex(Corner7, Normal2, ColorB);
    Cube->Vertices[17] = Vertex(Corner5, Normal2, ColorB);
    
    Cube->Vertices[18] = Vertex(Corner4, Normal3, ColorR);
    Cube->Vertices[19] = Vertex(Corner5, Normal3, ColorR);
    Cube->Vertices[20] = Vertex(Corner0, Normal3, ColorR);
    Cube->Vertices[21] = Vertex(Corner0, Normal3, ColorR);
    Cube->Vertices[22] = Vertex(Corner5, Normal3, ColorR);
    Cube->Vertices[23] = Vertex(Corner1, Normal3, ColorR);
    
    Cube->Vertices[24] = Vertex(Corner4, Normal4, ColorG);
    Cube->Vertices[25] = Vertex(Corner0, Normal4, ColorG);
    Cube->Vertices[26] = Vertex(Corner6, Normal4, ColorG);
    Cube->Vertices[27] = Vertex(Corner6, Normal4, ColorG);
    Cube->Vertices[28] = Vertex(Corner0, Normal4, ColorG);
    Cube->Vertices[29] = Vertex(Corner2, Normal4, ColorG);
    
    Cube->Vertices[30] = Vertex(Corner1, Normal5, ColorG);
    Cube->Vertices[31] = Vertex(Corner5, Normal5, ColorG);
    Cube->Vertices[32] = Vertex(Corner3, Normal5, ColorG);
    Cube->Vertices[33] = Vertex(Corner3, Normal5, ColorG);
    Cube->Vertices[34] = Vertex(Corner5, Normal5, ColorG);
    Cube->Vertices[35] = Vertex(Corner7, Normal5, ColorG);
}

// NOTE: Returns number of generated density blocks
int32 GenerateDensityBlocks(terrain* Terrain, int32 MaxBlocksToGenerate)
{
    int32 BlocksToGenerate = MaxBlocksToGenerate;
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < Terrain->StoreResolutionCount;
        ResolutionIndex++)
    {
        density_block_pos_array *BlockPositions = Terrain->DensityPositionStore + ResolutionIndex;
        for(size_t PosIndex = 0; 
            (PosIndex < BlockPositions->Count) && (BlocksToGenerate > 0) ;
            ++PosIndex)
        {
            world_block_pos *BlockP = BlockPositions->Pos + PosIndex;
            block_hash *DensityHash = GetHash(Terrain->DensityHash, BlockP);
            // NOTE: This can give back a deleted hash, if it had the same key as this block,
            // and it was already deleted once, and wasn't overwritten since.
            if(HashIsEmpty(DensityHash))
            {
                BlocksToGenerate--;
                // NOTE: Initialize block
                uint32 BlockIndex = Terrain->DensityBlockCount;
                terrain_density_block *DensityBlock = Terrain->DensityBlocks + BlockIndex;
                
                GenerateDensityBlock(DensityBlock, &Terrain->PerlinArray, BlockP);
                
                Assert(Terrain->DensityBlockCount < ArrayCount(Terrain->DensityBlocks));
                DensityHash = WriteHash(Terrain->DensityHash, BlockP, Terrain->DensityBlockCount++);
            }
        }
    }
    return MaxBlocksToGenerate - BlocksToGenerate;
}

void GenerateTerrainBlocks(memory_arena *Arena, terrain* Terrain, game_input *Input,
                           session_description *Session, generator_position *GeneratorCenter,
                           v3 WorldMousePos, v3 CameraOrigo, cube* Cube, v3 CameraP, v3 CamDir,
                           dx_resource *DXResources)
{
    int32 MaxDensityBlocksToGenerate = 3;
    
    timer DensityClock;
    int32 DensityBlocksGenerated = GenerateDensityBlocks(Terrain, MaxDensityBlocksToGenerate);
    // DensityClock.PrintMiliSeconds("Density:");
    
    int32 MaxRenderBlocksToGenerateInFrame = 4;
    density_block_pos_array BlocksToRender;
    BlocksToRender.Count = 0;

    // NOTE: Handle Mouse click
    // TODO: This shouldnt be in generator, mouse handling and cube placing is not this modules job
    // What happends here? -> raytracing with terrain
    //                     -> Deforming terrain around a point
    //                     -> Placing a cube
    if(Input->MouseRightButton)
    {
        v3 RayDirection = Normalize(WorldMousePos - CameraOrigo);
        for(real32 RayLength = 0.5f; 
            RayLength < 2000.0f; 
            RayLength += 0.5f)
        {
            v3 CheckPos = CameraOrigo + (RayLength*RayDirection);
            
            real32 PosValue = GetWorldGridValueFromV3(Terrain, CheckPos, Terrain->FixedResolution[0]);
            block_node ClickNode = ConvertRenderPosToBlockNode(CheckPos, Terrain->FixedResolution[0]);
            terrain_density_block *DynamicBlock = GetDynamicBlock(Arena, Terrain, &ClickNode.BlockP,
                                                                  Session);
            real32 DynamicVal = GetGrid(&DynamicBlock->Grid, ClickNode.X, ClickNode.Y, ClickNode.Z);
            real32 Value = PosValue + DynamicVal;
            if(Value < DENSITY_ISO_LEVEL)
            {
                real32 DeformerStrength = 50.0f;
                block_deformer SphereDeformer;
                SphereDeformer.Type = DeformerTypeSphere;
                SphereDeformer.Sign = 1.0f * Input->DeformerSign;
                SphereDeformer.Radius = 30.0f;
                SphereDeformer.Center = CheckPos;
                SphereDeformer.Strength = DeformerStrength;
                
                block_deformer GradualSphereDeformer;
                GradualSphereDeformer.Type = DeformerTypeGradualSphere;
                GradualSphereDeformer.Sign = 1.0f * Input->DeformerSign;
                GradualSphereDeformer.Radius = 30.0f;
                GradualSphereDeformer.Center = CheckPos;
                GradualSphereDeformer.Strength = DeformerStrength;
                
                block_deformer CubeDeformer;
                CubeDeformer.Type = DeformerTypeCube;
                CubeDeformer.Sign = 1.0f * Input->DeformerSign;
                CubeDeformer.Radius = 30.0f;
                CubeDeformer.Center = CheckPos;
                CubeDeformer.Strength = DeformerStrength;
                
                block_deformer *UsedDeformer = &GradualSphereDeformer;
                
                DeformBlocks(Terrain, Arena, UsedDeformer, &BlocksToRender, 
                             &MaxRenderBlocksToGenerateInFrame, Session);
                AddCube(Cube, CheckPos, 1.0f, 
                        v4{1.0f, 0.0f, 0.0f, 1.0f}, 
                        v4{0.0f, 1.0f, 0.0f, 1.0f}, 
                        v4{0.0f, 0.0f, 1.0f, 1.0f});
                break;
            }
        }
    }

    
    timer LODClock;
    // NOTE: 
    // If all the lower blocks of a block are generated, then the lower blocks can be mix rendered
    // If a mix rendered blocks neigbours are done generating their lower blocks, 
    // then they can be generated normally
    
    int32 DeleteRenderBlockCount = 0;
    world_block_pos DeleteRenderBlockQueue[1000];
    
    int32 LowestResUsed = Terrain->FixedResolution[0];
    // NOTE: map all the blocks in range, if they arent already.
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < Terrain->StoreResolutionCount;
        ResolutionIndex++)
    {
        density_block_pos_array *BlockPositions = Terrain->DensityPositionStore + ResolutionIndex;
        for(size_t BlockPosIndex = 0; 
            (BlockPosIndex < BlockPositions->Count);
            ++BlockPosIndex)
        {
            // NOTE: Map this block, and its neighbours
            world_block_pos *BlockP = BlockPositions->Pos + BlockPosIndex;
            
            block_hash *ResHash = GetHash(Terrain->ResolutionMapping, BlockP);
            if(HashIsEmpty(ResHash))
            {
                world_block_pos BiggerP = GetBiggerResBlockPosition(BlockP);
                if(BiggerP.Resolution <= Terrain->FixedResolution[0])
                {
                    block_hash *BigPResHash = GetHash(Terrain->ResolutionMapping, &BiggerP);
                    Assert(!HashIsEmpty(BigPResHash));
                    ResHash = MapBlockPosition(Terrain, BlockP, BigPResHash->Index);
                }
                else
                {
                    ResHash = MapBlockPosition(Terrain, BlockP, Terrain->FixedResolution[0]);
                }
            }
            
            // NOTE: Could do downgrading here, if block is far away from the camera
            if(ResHash->Index < (int32)BlockP->Resolution)
            {
                lower_blocks LowerBlocks;
                GetLowerResBlockPositions(&LowerBlocks, BlockP);
                bool32 ShouldDowngrade = true;
                for(int32 LowerIndex = 0;
                    LowerIndex < ArrayCount(LowerBlocks.Pos);
                    LowerIndex++)
                {
                    world_block_pos *LowerP = LowerBlocks.Pos + LowerIndex;
                    int32 ResIndex = GetResolutionIndex(LowerP->Resolution);
                    world_block_pos* Center = (world_block_pos*)GeneratorCenter;
                    if(ManhattanDistance(Center + ResIndex, LowerP) <= RENDERED_BLOCK_RADIUS)
                    {
                        ShouldDowngrade = false;
                    }
                }
                
                if(ShouldDowngrade)
                {
                    DowngradeMapping(Terrain, BlockP, BlockP->Resolution, DeleteRenderBlockQueue, &DeleteRenderBlockCount);
                }
            }
            Assert(!HashIsEmpty(ResHash));
            LowestResUsed = Min(LowestResUsed, ResHash->Index);
        }
    }
    // LODClock.PrintMiliSeconds("Map resolutions:");
    
    bool32 EverybodyIsRenderedOnCorrectResolution = true;
    // NOTE: Select the next blocks that we can render.
    
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < Terrain->StoreResolutionCount;
        ResolutionIndex++)
    {
        block_pos_array *BlockPositions = Terrain->RenderPositionStore + ResolutionIndex;
        for(size_t BlockPosIndex = 0; 
            (BlockPosIndex < BlockPositions->Count) && (MaxRenderBlocksToGenerateInFrame > 0) ;
            ++BlockPosIndex)
        {
            world_block_pos *BlockP = BlockPositions->Pos + BlockPosIndex;
            block_hash *ResHash = GetHash(Terrain->ResolutionMapping, BlockP);
            Assert(!HashIsEmpty(ResHash));
            bool32 AreOnSameRes = BlockP->Resolution == ResHash->Index;
            
            if(AreOnSameRes && !IsBlockInTerrainModel(Terrain, BlockP))
            {
                EverybodyIsRenderedOnCorrectResolution = false;
                
                QueueBlockToRender(Terrain, BlockP, 
                    &MaxRenderBlocksToGenerateInFrame, 
                    &BlocksToRender);
            }
        }
    }
    // LODClock.PrintMiliSeconds("Select render_blocks:");
    
    // NOTE: if everbody is rendered on the resolution it should be rendered, 
    // then we can upgrade a block to a new resolution
    if(EverybodyIsRenderedOnCorrectResolution && 
       (MaxRenderBlocksToGenerateInFrame >= 4))
    {    
        // NOTE: if everybody with a Resolution of LowestResUsed is rendered, we can upgrade LowestResUsed
        bool32 CanUpgradeLowestResolution = true;
        for(uint32 ResolutionIndex = 0;
            ResolutionIndex < Terrain->StoreResolutionCount;
            ResolutionIndex++)
        {
            block_pos_array *BlockPositions = Terrain->RenderPositionStore + ResolutionIndex;
            for(size_t BlockPosIndex = 0; 
                (BlockPosIndex < BlockPositions->Count);
                ++BlockPosIndex)
            {
                world_block_pos *BlockP = BlockPositions->Pos + BlockPosIndex;
                block_hash *ResHash = GetHash(Terrain->ResolutionMapping, BlockP);
                Assert(!HashIsEmpty(ResHash));
                
                CanUpgradeLowestResolution = CanUpgradeLowestResolution &&
                    (((BlockP->Resolution == LowestResUsed) && IsBlockInTerrainModel(Terrain, BlockP)) ||
                    ((BlockP->Resolution != LowestResUsed) && !IsBlockInTerrainModel(Terrain, BlockP)));
            }
        }
        
        uint32 LowestResUsedIndex = GetResolutionIndex((uint32)LowestResUsed);
        if(CanUpgradeLowestResolution)
        {
            uint32 NewResIndex = LowestResUsedIndex+1;
            if(NewResIndex < Terrain->MaxResolutionToRender)
            {
                LowestResUsed = Terrain->FixedResolution[NewResIndex];
                LowestResUsedIndex = NewResIndex;
            }
        }
            
        block_pos_array *BlockPositions = Terrain->RenderPositionStore + LowestResUsedIndex;
        for(size_t BlockPosIndex = 0; 
            (BlockPosIndex < BlockPositions->Count) && (MaxRenderBlocksToGenerateInFrame > 0) ;
            ++BlockPosIndex)
        {
            world_block_pos *BlockP = BlockPositions->Pos + BlockPosIndex;
            block_hash *ResHash = GetHash(Terrain->ResolutionMapping, BlockP);
            Assert(!HashIsEmpty(ResHash));
            
            world_block_pos BiggerP = GetBiggerResBlockPosition(BlockP);
            lower_blocks Siblings;
            GetLowerResBlockPositions(&Siblings, &BiggerP);
            bool32 SiblingDensitiesLoaded = DidDensityBlocksLoaded(Terrain, Siblings.Pos, 
                                                                    ArrayCount(Siblings.Pos));
            if(ResHash->Index > LowestResUsed && SiblingDensitiesLoaded)
            {
                // NOTE: We can upgrade it, and its siblings to a smaller resolution
                for(uint32 SiblingIndex = 0;
                    SiblingIndex < ArrayCount(Siblings.Pos);
                    SiblingIndex++)
                {
                    // NOTE: Ensure that every sibling is mapped right
                    world_block_pos *SiblingP = Siblings.Pos + SiblingIndex;
                    auto SiblingHash = GetHash(Terrain->ResolutionMapping, SiblingP);
                    Assert(!HashIsEmpty(SiblingHash));                    
                    Assert(SiblingHash->Index == LowestResUsed*2 || 
                            SiblingHash->Index == LowestResUsed);
                    SiblingHash->Index = LowestResUsed;
                    UpdateLowerBlocksMapping(Terrain, SiblingP, LowestResUsed);
                }
                
                DeleteFromTerrainModel(Terrain, &BiggerP);
                while(BiggerP.Resolution <= Terrain->FixedResolution[0])
                {
                    block_hash *BiggerHash = GetHash(Terrain->ResolutionMapping, &BiggerP);
                    Assert(!HashIsEmpty(BiggerHash));
                    BiggerHash->Index = LowestResUsed;
                    BiggerP = GetBiggerResBlockPosition(&BiggerP);
                }
                
                // NOTE: We need to render the siblings
                for(uint32 SiblingIndex = 0;
                    SiblingIndex < ArrayCount(Siblings.Pos);
                    SiblingIndex++)
                {
                    world_block_pos *SiblingP = Siblings.Pos + SiblingIndex;
                    // NOTE: We want to rerender its neighbours too if we can
                    block_lower_neighbours SiblingNeighbours;
                    GetNeighbourBlockPositionsOnLowerRes(&SiblingNeighbours, SiblingP);
                    for(uint32 NIndex = 0;
                        NIndex < ArrayCount(SiblingNeighbours.Pos);
                        NIndex++)
                    {
                        world_block_pos *NPos = SiblingNeighbours.Pos + NIndex;
                        block_hash *NHash = GetHash(Terrain->ResolutionMapping, NPos);
                        if(HashIsEmpty(NHash))
                        {
                            NHash = MapBlockPositionAfterParent(Terrain, NPos);
                        }
                        Assert(!HashIsEmpty(NHash));
                        
                        //NOTE: Have to examine the densities of neighbour's neighbours too
                        world_block_pos MappedNPos = GetAndSetBiggerMappedPosition(Terrain, NPos);
                        Assert(MappedNPos.Resolution == NHash->Index);
                        QueueBlockToRender(Terrain, &MappedNPos, 
                            &MaxRenderBlocksToGenerateInFrame, 
                            &BlocksToRender);
                    }
                    QueueBlockToRender(Terrain, SiblingP,
                        &MaxRenderBlocksToGenerateInFrame, 
                        &BlocksToRender);
                }
            }
        }
    }
    // LODClock.PrintMiliSeconds("Upgrade Resoltuion:");
    
    for(int32 DelIndex = 0; DelIndex < DeleteRenderBlockCount; DelIndex++)
    {
        world_block_pos *BlockP = DeleteRenderBlockQueue + DelIndex;
        DeleteFromTerrainModel(Terrain, BlockP);
    }
    // LODClock.PrintMiliSeconds("Delete render blocks:");
    
    // NOTE: Remove duplicates from BlocksToRender
    for(uint32 RenderIndex = 0;
        RenderIndex < BlocksToRender.Count;
        RenderIndex++)
    {
        world_block_pos *BlockP = BlocksToRender.Pos + RenderIndex;
        for(uint32 RenderInnerIndex = RenderIndex+1;
            RenderInnerIndex < BlocksToRender.Count;
            RenderInnerIndex++)
        {
            world_block_pos *InnerBlockP = BlocksToRender.Pos + RenderInnerIndex;
            if(WorldPosEquals(BlockP, InnerBlockP))
            {
                world_block_pos LastP = BlocksToRender.Pos[BlocksToRender.Count-1];
                BlocksToRender.Pos[RenderInnerIndex] = LastP;
                --BlocksToRender.Count;
                --RenderInnerIndex;
            }
        }
    }
    // LODClock.PrintMiliSeconds("Remove duplicates:");
    
    // NOTE: Before rendering, check for every block that their neighbours are mapped right 
    // and their densities are loaded.
    // This is basically a big assert for resolutions, and loading in dynamic blocks
    for(uint32 RenderIndex = 0;
        RenderIndex < BlocksToRender.Count;
        RenderIndex++)
    {
        world_block_pos *BlockP = BlocksToRender.Pos + RenderIndex;
        
        block_lower_neighbours NPositions;
        GetNeighbourBlockPositionsOnLowerRes(&NPositions, BlockP);
        
        terrain_density_block *Neighbours[ArrayCount(NPositions.Pos)];
        terrain_density_block *DynNeighbours[ArrayCount(NPositions.Pos)];
        for(uint32 NeighbourIndex = 0;
            NeighbourIndex < ArrayCount(NPositions.Pos);
            NeighbourIndex++)
        {
            world_block_pos *NeighbourP = NPositions.Pos + NeighbourIndex;
            world_block_pos MappedP = GetAndSetBiggerMappedPosition(Terrain, NeighbourP);
            Assert((MappedP.Resolution >= BlockP->Resolution/2) &&
                   (MappedP.Resolution <= BlockP->Resolution*2));
            
            block_hash *NeighbourHash = GetHash(Terrain->DensityHash, &MappedP);
            Assert(!HashIsEmpty(NeighbourHash));
            Neighbours[NeighbourIndex] = Terrain->DensityBlocks + NeighbourHash->Index;
            
            DynNeighbours[NeighbourIndex] = GetDynamicBlock(Arena, Terrain, &MappedP, Session);
        }
    }
    // LODClock.PrintMiliSeconds("NeighbourCheck:");
    
    
    // logger::PerfPrint("Block count: %f", BlocksToRender.Count);
    // timer PoligoniseClock;
    timer FullPoligoniseClock;
    // NOTE: These blocks may have been rendered once, but now they have to be generated again
    for(uint32 RenderIndex = 0;
        RenderIndex < BlocksToRender.Count;
        RenderIndex++)
    {
        world_block_pos *BlockP = BlocksToRender.Pos + RenderIndex;
        DeleteFromTerrainModel(Terrain, BlockP);
        
        Assert(HashIsEmpty(GetHash(Terrain->RenderHash, BlockP)));
        Assert(HashIsEmpty(GetZeroHash(Terrain, BlockP)));
        
        {
            // PoligoniseClock.Reset();
            //terrain_render_block *RenderBlock = Terrain->PoligonisedBlocks + Terrain->PoligonisedBlockCount;
            terrain_render_block RenderBlock;
            CreateRenderBlock(Terrain, &RenderBlock, BlockP);
            // PoligoniseClock.CalculateAverageTime(&Terrain->AvgPoligoniseTime);
            
            if(RenderBlock.VertexCount != 0)
            {
                terrain_block_model *Model = Terrain->PoligonisedBlocks + Terrain->PoligonisedBlockCount;
                
                Model->Pos = RenderBlock.Pos;
                Model->WPos = RenderBlock.WPos;
                Model->VertexCount = RenderBlock.VertexCount;
                DXResources->CreateVertexBufferImmutable(&(Model->Buffer), 
                                                         Model->VertexCount, RenderBlock.Vertices);                
                
                WriteHash(Terrain->RenderHash, BlockP, Terrain->PoligonisedBlockCount++);
                Assert(Terrain->PoligonisedBlockCount < ArrayCount(Terrain->PoligonisedBlocks));
                
                // PoligoniseClock.PrintMiliSeconds("Gen render_block:");
            }
            else
            {
                WriteZeroHash(Terrain, BlockP);
                Terrain->ZeroBlockCount++;
            }
            
        }
    }
    // FullPoligoniseClock.PrintMiliSeconds("Full gen render_block:");
    
    Terrain->RenderBlockCount = 0;
    for(uint32 ResolutionIndex = 0;
        ResolutionIndex < Terrain->StoreResolutionCount;
        ResolutionIndex++)
    {
        block_pos_array *BlockPositions = Terrain->RenderPositionStore + ResolutionIndex;
        for(size_t PosIndex = 0; 
            (PosIndex < BlockPositions->Count);
            ++PosIndex)
        {
            world_block_pos *BlockP = BlockPositions->Pos + PosIndex;
            // NOTE: Siblings may be too far from camera, so we have to add them here to render
            world_block_pos BiggerP = GetBiggerResBlockPosition(BlockP);
            lower_blocks Siblings;
            GetLowerResBlockPositions(&Siblings, &BiggerP);
            for(uint32 SiblingIndex = 0;
                SiblingIndex < ArrayCount(Siblings.Pos);
                SiblingIndex++)
            {
                world_block_pos *SiblingP = Siblings.Pos + SiblingIndex;
                block_hash *ZeroHash = GetZeroHash(Terrain, SiblingP);
                if(HashIsEmpty(ZeroHash))
                {
                    block_hash *Hash = GetHash(Terrain->RenderHash, SiblingP);
                    if(!HashIsEmpty(Hash))
                    {
                        QueueForRendering(Terrain, Terrain->PoligonisedBlocks + Hash->Index, 
                                          CameraP, CamDir);
                    }
                }
            }
        }
    }
}













