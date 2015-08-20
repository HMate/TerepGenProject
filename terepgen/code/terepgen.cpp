/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen.h"

    
internal void 
CalculateBlockPositions(game_state *GameState, v3 CentralBlockPos)
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
                GameState->BlockPositions[PosIndex++] = CentralBlockPos +
                    v3{(real32)XIndex, (real32)YIndex, (real32)ZIndex};
            }
        }
    }
    Assert(PosIndex == ArrayCount(GameState->BlockPositions));
}

internal void
GenerateTerrain(game_state *GameState)
{
    real32 BlockSize = real32(TERRAIN_BLOCK_SIZE);
    uint32 BlockResolution = 8;
    
    v3 CentralBlockPos = GameState->CameraPos / BlockSize;
    CentralBlockPos = v3{FloorReal32(CentralBlockPos.X), 
                         FloorReal32(CentralBlockPos.Y),
                         FloorReal32(CentralBlockPos.Z)};
    CalculateBlockPositions(GameState, CentralBlockPos);
                         
    terrain_density_block DensityBlock;
    RandomGenerator Rng(GameState->Seed);
    Rng.SetSeed(1000);
    for(size_t BlockIndex = 0; 
        BlockIndex < ArrayCount(GameState->RenderBlocks);
        BlockIndex++)
    {
        DensityBlock.Pos = GameState->BlockPositions[BlockIndex] * BlockSize * (real32)BlockResolution;
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
