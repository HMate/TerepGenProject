/*
    Terep generátor by Hidvégi Máté @2016
*/
bool32 WorldPosEquals(world_block_pos *A, world_block_pos *B)
{
    bool32 Result = (A->BlockX == B->BlockX) && 
                    (A->BlockY == B->BlockY) && 
                    (A->BlockZ == B->BlockZ) && 
                    (A->Resolution == B->Resolution);
    return Result;
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

block_node
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

block_node
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

v3
ConvertBlockNodeToRenderPos(block_node *Node)
{
    v3 Result = V3FromWorldPos(Node->BlockP);
    Result = Result + v3{(real32)Node->X, (real32)Node->Y, (real32)Node->Z} 
                        * (real32)Node->BlockP.Resolution * RENDER_SPACE_UNIT;
    
    return Result;
}