/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_terrain.h"
#include "terepgen_marching_cubes.cpp"


// Terrain 3D

internal void 
GenerateDensityGrid(terrain_density_block *DensityBlock, RandomGenerator *Rng, uint32 BlockResolution)
{
    uint32 TerrainDimension = DensityBlock->Grid.Dimension;
    for(uint32 Plane = 0;
        Plane < TerrainDimension;
        ++Plane)
    {
        for(uint32 Row = 0;
            Row < TerrainDimension;
            ++Row)
        {
            for(uint32 Column = 0;
                Column < TerrainDimension;
                ++Column)
            {
                real32 DensityValue = DensityBlock->Pos.Y + (real32)(((int32)Row-2) * (real32)BlockResolution);
                
                real32 WorldX = DensityBlock->Pos.X + ((((int32)Plane)-2) * (real32)BlockResolution);
                real32 WorldY = DensityBlock->Pos.Y + ((((int32)Row)-2) * (real32)BlockResolution);
                real32 WorldZ = DensityBlock->Pos.Z + ((((int32)Column)-2) * (real32)BlockResolution);
                
                v3 WorldPos = {WorldX, WorldY, WorldZ};
             
                DensityValue += Rng->RandomFloat(WorldPos) * 1.0f;
                DensityValue += Rng->RandomFloat(WorldPos * 0.51f) * 2.0f;
                DensityValue += Rng->RandomFloat(WorldPos * 0.248f) * 4.0f;
                DensityValue += Rng->RandomFloat(WorldPos * 0.128f) * 8.0f;
                DensityValue += Rng->RandomFloat(WorldPos * 0.0621f) * 16.0f;
                DensityValue += Rng->RandomFloat(WorldPos * 0.03127f) * 32.0f;
                DensityValue += Rng->RandomFloat(WorldPos * 0.015622f) * 64.0f;
                                                                   
                GetGridPRC(&DensityBlock->Grid, Plane, Row, Column) = DensityValue;
            }
        }
    }
}

internal void 
GenerateDensityGrid2(terrain_density_block *DensityBlock, RandomGenerator *Rng, real32 Persistence)
{
    ZeroOutGridPoints(&DensityBlock->Grid);
        
    uint32 TerrainDimension = DensityBlock->Grid.Dimension;
    uint32 FirstOctave = 0;
    uint32 OctaveCount = Log2(TerrainDimension);
    
    for(uint32 Octaves = FirstOctave;
        Octaves < OctaveCount;
        ++Octaves)
    {
        /*
        float freq = 2^i;
        float amplitude = persistence^i;
        */
        real32 Weight = Pow(Persistence, OctaveCount-Octaves-1);
        int32 WaveLength = (int32)Pow2(Octaves);
        real32 Frequency = 1.0f / (WaveLength + 1);
        uint32 PGDimension = ((TerrainDimension-1)/WaveLength) + 1;
        
        dynamic_grid3D PerlinGrid{PGDimension};
        /* NOTE: Plane is axis X
                 Row is axis Y
                 Column is axis Z
        */
        for(int32 Plane = 0;
            Plane < PerlinGrid.Dimension;
            ++Plane)
        {
            for(int32 Row = 0;
                Row < PerlinGrid.Dimension;
                ++Row)
            {
                for(int32 Column = 0;
                    Column < PerlinGrid.Dimension;
                    ++Column)
                {
                    /*NOTE: Only works properly, if GridPoses are apart from each other 
                         with value TerrainDimension. To correct this PerlinGrid dimension 
                         should be 1 bigger, and during streching we need to check 
                         which octave we are in.*/
                    int32 WorldX = Plane + FloorInt32(DensityBlock->Pos.X/WaveLength);
                    int32 WorldY = Row + FloorInt32(DensityBlock->Pos.Y/WaveLength);
                    int32 WorldZ = Column + FloorInt32(DensityBlock->Pos.Z/WaveLength);
                    real32 RandomVal = Rng->RandomFloat(WorldX * WaveLength,
                                                        WorldY * WaveLength,
                                                        WorldZ * WaveLength); 
                    RandomVal = RandomVal * Weight;
                    real32 Dampening = (WorldY)*(1.0f/(4.0f * TerrainDimension));
                    PerlinGrid.GetPRC(Plane, Row, Column) = RandomVal + Dampening;
                }
            }
        }
        
        /*NOTE: Have to stretch out the little grids to match the size of the terrain
             During strech, we get the inner points by interpolating between the Perlin grid values
        */
        for(uint32 Plane = 0;
            Plane < TerrainDimension;
            ++Plane)
        {  
            real32 PlaneRatio = (real32)Plane * (PGDimension-1) / (TerrainDimension - 1);
            uint32 PGPlane = FloorUint32(PlaneRatio);
            PlaneRatio = PlaneRatio - (real32)PGPlane;
            
            for(uint32 Row = 0;
                Row < TerrainDimension;
                ++Row)
            {
                real32 RowRatio = (real32)Row * (PGDimension-1) / (TerrainDimension - 1);
                uint32 PGRow = FloorUint32(RowRatio);
                RowRatio = RowRatio - (real32)PGRow;
                
                for(uint32 Column = 0;
                    Column < TerrainDimension;
                    ++Column)
                {                
                    real32 ColumnRatio = (real32)Column * (PGDimension-1) / (TerrainDimension - 1);
                    uint32 PGColumn = FloorUint32(ColumnRatio);
                    ColumnRatio = ColumnRatio - (real32)PGColumn;
                    
                    Assert(((PGColumn+1) <= PGDimension) &&
                           ((PGRow+1) <= PGDimension) &&
                           ((PGPlane+1) <= PGDimension));
                    
                    real32 InnerValue = 
                        (1.0f - PlaneRatio) *
                        ((1.0f-RowRatio) * 
                            ((1.0f-ColumnRatio) * PerlinGrid.GetPRC(PGPlane, PGRow, PGColumn) + 
                            (ColumnRatio) * PerlinGrid.GetPRC(PGPlane, PGRow, PGColumn+1)) +
                        ( RowRatio * 
                            ((1.0f-ColumnRatio) * PerlinGrid.GetPRC(PGPlane, PGRow+1, PGColumn) + 
                             ColumnRatio * PerlinGrid.GetPRC(PGPlane, PGRow+1, PGColumn+1) )
                        )) +
                        PlaneRatio * 
                        ((1.0f-RowRatio) * 
                            ((1.0f-ColumnRatio) * PerlinGrid.GetPRC(PGPlane+1, PGRow, PGColumn) + 
                              ColumnRatio * PerlinGrid.GetPRC(PGPlane+1, PGRow, PGColumn+1)) +
                        ( RowRatio * 
                            ((1.0f-ColumnRatio) * PerlinGrid.GetPRC(PGPlane+1, PGRow+1, PGColumn) + 
                            ( ColumnRatio * PerlinGrid.GetPRC(PGPlane+1, PGRow+1, PGColumn+1)) )
                        ));
                    GetGridPRC(&DensityBlock->Grid, Plane, Row, Column) += InnerValue;
                }
            }
        }
    }
}

internal v3
GetPointNormal(terrain_density_block *DensityBlock, v3 Point)
{    
    real32 Diff = 1.0f;
    real32 DimensionBound = (real32)DensityBlock->Grid.Dimension-1.0f;
    
    real32 DiffXMin = Point.X - Diff;
    DiffXMin = ClampReal32(DiffXMin, 0.0f, DimensionBound);
    real32 DiffXMax = Point.X + Diff;
    DiffXMax = ClampReal32(DiffXMax, 0.0f, DimensionBound);
    
    real32 DiffYMin = Point.Y - Diff;
    DiffYMin = ClampReal32(DiffYMin, 0.0f, DimensionBound);
    real32 DiffYMax = Point.Y + Diff;
    DiffYMax = ClampReal32(DiffYMax, 0.0f, DimensionBound);
    
    real32 DiffZMin = Point.Z - Diff;
    DiffZMin = ClampReal32(DiffZMin, 0.0f, DimensionBound);
    real32 DiffZMax = Point.Z + Diff;
    DiffZMax = ClampReal32(DiffZMax, 0.0f, DimensionBound);
    
    real32 NormalX = 
        GetGridPRCWithInterpolate(&DensityBlock->Grid, DiffXMax, Point.Y, Point.Z) -
        GetGridPRCWithInterpolate(&DensityBlock->Grid, DiffXMin, Point.Y, Point.Z);
    real32 NormalY = 
        GetGridPRCWithInterpolate(&DensityBlock->Grid, Point.X, DiffYMax, Point.Z) -
        GetGridPRCWithInterpolate(&DensityBlock->Grid, Point.X, DiffYMin, Point.Z);
    real32 NormalZ = 
        GetGridPRCWithInterpolate(&DensityBlock->Grid, Point.X, Point.Y, DiffZMax) -
        GetGridPRCWithInterpolate(&DensityBlock->Grid, Point.X, Point.Y, DiffZMin);
        
    v3 Result = v3{NormalX, NormalY, NormalZ};
    
    return Result;
}

internal vertex
Get3DGridVertex(v3 LocalPos, v3 Normal, color Color)
{
    vertex Result = {LocalPos.X, LocalPos.Y, LocalPos.Z, 1.0f, 
                     Normal.X, Normal.Y, Normal.Z, 1.0f,
                     Color};
    return Result;
}

/*
    NOTE: The dimension of the grids required to be 5 units bigger
        than the size of the final terrain
        +1 in each dimension for marching cubes
        +2 at each side to calculate proper normals
*/
internal void
CreateRenderVertices(terrain_render_block *RenderBlock, terrain_density_block *DensityBlock,
                     uint32 CubeSize)
{
    Assert(CubeSize > 0);
    real32 CellDiff = (real32)CubeSize;
    color GreenColor = color{0.0, 1.0f, 0.0f, 1.0f};
    v3 PosDiff = {2.0f, 2.0f, 2.0f};
    
    RenderBlock->Pos = DensityBlock->Pos;
    uint32 TerrainDimension = DensityBlock->Grid.Dimension;
    uint32 VertexCount = 0;
    for(uint32 Plane = 2;
        Plane < TerrainDimension-3;
        Plane += 1)
    {
        for(uint32 Row = 2;
            Row < TerrainDimension-3;
            Row += 1)
        {
            for(uint32 Column = 2;
                Column < TerrainDimension-3;
                Column += 1)
            {                
                GRIDCELL Cell;
                real32 Planef = (real32)Plane;
                real32 Rowf = (real32)Row;
                real32 Columnf = (real32)Column;
                Cell.p[0] = v3{Planef     , Rowf+1.0f, Columnf         };
                Cell.p[1] = v3{Planef     , Rowf+1.0f, Columnf+1.0f};
                Cell.p[2] = v3{Planef     , Rowf     , Columnf+1.0f};
                Cell.p[3] = v3{Planef     , Rowf     , Columnf         };
                Cell.p[4] = v3{Planef+1.0f, Rowf+1.0f, Columnf         };
                Cell.p[5] = v3{Planef+1.0f, Rowf+1.0f, Columnf+1.0f};
                Cell.p[6] = v3{Planef+1.0f, Rowf     , Columnf+1.0f};
                Cell.p[7] = v3{Planef+1.0f, Rowf     , Columnf         };
                Cell.val[0] = GetGridPRC(&DensityBlock->Grid, Plane  , Row+1, Column         );
                Cell.val[1] = GetGridPRC(&DensityBlock->Grid, Plane  , Row+1, Column+1);
                Cell.val[2] = GetGridPRC(&DensityBlock->Grid, Plane  , Row  , Column+1);
                Cell.val[3] = GetGridPRC(&DensityBlock->Grid, Plane  , Row  , Column         );
                Cell.val[4] = GetGridPRC(&DensityBlock->Grid, Plane+1, Row+1, Column         );
                Cell.val[5] = GetGridPRC(&DensityBlock->Grid, Plane+1, Row+1, Column+1);
                Cell.val[6] = GetGridPRC(&DensityBlock->Grid, Plane+1, Row  , Column+1);
                Cell.val[7] = GetGridPRC(&DensityBlock->Grid, Plane+1, Row  , Column         );
                TRIANGLE Triangles[5];
                uint32 TriangleCount = Polygonise(Cell, 0.05f, Triangles);
                
                for(uint32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
                {
                    v3 Point0 = Triangles[TriangleIndex].p[0];
                    v3 Point1 = Triangles[TriangleIndex].p[1];
                    v3 Point2 = Triangles[TriangleIndex].p[2];
                                        
                    v3 Normal0 = GetPointNormal(DensityBlock, Point0);
                    v3 Normal1 = GetPointNormal(DensityBlock, Point1);
                    v3 Normal2 = GetPointNormal(DensityBlock, Point2);
                    
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DGridVertex((Point0-PosDiff) * CellDiff, Normal0, GreenColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DGridVertex((Point1-PosDiff) * CellDiff, Normal1, GreenColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DGridVertex((Point2-PosDiff) * CellDiff, Normal2, GreenColor);
                }
            }
        }
    }
    RenderBlock->VertexCount = VertexCount;
    
#if TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Current Vertex Count: %d\n", VertexCount);
    OutputDebugStringA(DebugBuffer);
#endif
}
    
internal void 
CalculateBlockPositions(v3 *BlockPositions, uint32 BlockCount, v3 CentralBlockPos)
{    
    uint32 CubeRoot = Cbrt(BlockCount);
    uint32 IndexDelta = CubeRoot/2;
    int32 Start = -IndexDelta;
    int32 End = CubeRoot - IndexDelta;
    
    uint32 PosIndex = 0;
    for(int32 XIndex = Start; XIndex < End; ++XIndex)
    {
        for(int32 YIndex = Start; YIndex < End; ++YIndex)
        {
            for(int32 ZIndex = Start; ZIndex < End; ++ZIndex)
            {
                BlockPositions[PosIndex++] = CentralBlockPos +
                    v3{(real32)XIndex, (real32)YIndex, (real32)ZIndex};
            }
        }
    }
    Assert(PosIndex == BlockCount);
}

internal void
GenerateTerrain(terrain_render_block *RenderBlocks, v3 *BlockPositions, uint32 BlockCount,
                v3 CameraPos, uint32 Seed)
{
    real32 BlockSize = real32(TERRAIN_BLOCK_SIZE);
    uint32 BlockResolution = 8;
    
    v3 CentralBlockPos = CameraPos / BlockSize;
    CentralBlockPos = v3{FloorReal32(CentralBlockPos.X), 
                         FloorReal32(CentralBlockPos.Y),
                         FloorReal32(CentralBlockPos.Z)};
    CalculateBlockPositions(BlockPositions, BlockCount, CentralBlockPos);
                         
    terrain_density_block DensityBlock;
    RandomGenerator Rng(Seed);
    Rng.SetSeed(1000);
    for(size_t BlockIndex = 0; BlockIndex < BlockCount; BlockIndex++)
    {
        DensityBlock.Pos = BlockPositions[BlockIndex] * BlockSize * BlockResolution;
        GenerateDensityGrid(&DensityBlock, &Rng, BlockResolution);
        CreateRenderVertices(&(RenderBlocks[BlockIndex]), &DensityBlock, BlockResolution);
    }
}

internal void
RenderTerrain(terrainRenderer *Renderer, terrain_render_block *RenderBlocks, uint32 BlockCount)
{
    for(size_t BlockIndex = 0; BlockIndex < BlockCount; BlockIndex++)
    {
        Renderer->SetTransformations(RenderBlocks[BlockIndex].Pos);
        Renderer->DrawTriangles(RenderBlocks[BlockIndex].Vertices, RenderBlocks[BlockIndex].VertexCount);
        Renderer->SetTransformations(v3{});
    }
}









