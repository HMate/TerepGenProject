/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen.h"

    
internal void 
CalculateBlockPositions(game_state *GameState, world_block_pos CentralBlockPos)
{    
    int32 CubeRoot = Cbrt(ArrayCount(GameState->BlockPositions));
    int32 IndexDelta = CubeRoot/2;
    int32 Start = -IndexDelta;
    int32 End = CubeRoot - IndexDelta;
    
    uint32 PosIndex = 0;
    for(int32 XIndex = Start; XIndex < End; ++XIndex)
    {
        for(int32 YIndex = Start; YIndex < End; ++YIndex)
        {
            for(int32 ZIndex = Start; ZIndex < End; ++ZIndex)
            {
                GameState->BlockPositions[PosIndex].BlockX = CentralBlockPos.BlockX + XIndex;
                GameState->BlockPositions[PosIndex].BlockY = CentralBlockPos.BlockY + YIndex;
                GameState->BlockPositions[PosIndex].BlockZ = CentralBlockPos.BlockZ + ZIndex;
                PosIndex++;
            }
        }
    }
    Assert(PosIndex == ArrayCount(GameState->BlockPositions));
}

inline world_block_pos
GetWorldPosFromV3(v3 Pos, real32 BlockSize)
{
    world_block_pos Result = {};
    
    v3 CentralBlockPos = Pos / BlockSize;
    Result.BlockX = FloorInt32(CentralBlockPos.X); 
    Result.BlockY = FloorInt32(CentralBlockPos.Y);
    Result.BlockZ = FloorInt32(CentralBlockPos.Z);
     
    return Result;
}

inline v3
GetV3FromWorldPos(world_block_pos Pos)
{
    v3 Result = {};
    Result.X = (real32)Pos.BlockX;
    Result.Y = (real32)Pos.BlockY;
    Result.Z = (real32)Pos.BlockZ;
    
    return Result;
}

internal void
GenerateTerrain(game_state *GameState)
{
    real32 BlockSize = real32(TERRAIN_BLOCK_SIZE);
    uint32 BlockResolution = 8;
    
    world_block_pos CentralBlockPos = GetWorldPosFromV3(GameState->CameraPos, BlockSize);
    CalculateBlockPositions(GameState, CentralBlockPos);
                         
    terrain_density_block DensityBlock;
    RandomGenerator Rng(GameState->Seed);
    Rng.SetSeed(1000);
    for(size_t BlockIndex = 0; 
        BlockIndex < ArrayCount(GameState->RenderBlocks);
        BlockIndex++)
    {
        DensityBlock.Pos = GetV3FromWorldPos(GameState->BlockPositions[BlockIndex]) * BlockSize * (real32)BlockResolution;
        GenerateDensityGrid(&DensityBlock, &Rng, BlockResolution);
        CreateRenderVertices(&(GameState->RenderBlocks[BlockIndex]), &DensityBlock, BlockResolution);
    }
}

internal void
UpdateGameState(game_state *GameState)
{
    if(GameState->Initialized == false)
    {
        GenerateTerrain(GameState);
        GameState->Initialized = true;
    }

}

internal void
RenderGame(terrain_renderer *Renderer, game_state *GameState)
{
    for(size_t RenderBlockIndex = 0; 
        RenderBlockIndex < ArrayCount(GameState->RenderBlocks); 
        RenderBlockIndex++)
    {
        Renderer->SetTransformations(GameState->RenderBlocks[RenderBlockIndex].Pos);
        Renderer->DrawTriangles(
            GameState->RenderBlocks[RenderBlockIndex].Vertices,
            GameState->RenderBlocks[RenderBlockIndex].VertexCount);
        Renderer->SetTransformations(v3{});
    }
}
