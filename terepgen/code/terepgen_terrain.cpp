/*
    Terep generátor by Hidvégi Máté @2015

*/


// NOTE: Block Resolution gives how many density values are skipped
// This way a bigger area can be stored in the same block, 
// if at rendering we only use every BlockResolution'th value too.
internal void 
GenerateDensityGrid(terrain_density_block *DensityBlock, perlin_noise_generator *Rng, uint32 BlockResolution)
{
    int32 TerrainDimension = DensityBlock->Grid.Dimension;
    for(int32 Plane = 0;
        Plane < TerrainDimension;
        ++Plane) 
    {
        for(int32 Row = 0;
            Row < TerrainDimension;
            ++Row)
        {
            for(int32 Column = 0;
                Column < TerrainDimension;
                ++Column)
            {
                real32 DensityValue = DensityBlock->Pos.Y + (real32)((Row-2) * (real32)BlockResolution);
                
                real32 WorldX = DensityBlock->Pos.X + ((Plane-2) * (real32)BlockResolution);
                real32 WorldY = DensityBlock->Pos.Y + ((Row-2) * (real32)BlockResolution);
                real32 WorldZ = DensityBlock->Pos.Z + ((Column-2) * (real32)BlockResolution);
                
                v3 WorldPos = {WorldX, WorldY, WorldZ};
             
                DensityValue += Rng->RandomFloat(WorldPos) * 1.0f;
                // DensityValue += Rng->RandomFloat(WorldPos * 0.51f) * 2.0f;
                DensityValue += Rng->RandomFloat(WorldPos * 0.248f) * 4.0f;
                // DensityValue += Rng->RandomFloat(WorldPos * 0.128f) * 8.0f;
                DensityValue += Rng->RandomFloat(WorldPos * 0.0621f) * 16.0f;
                //DensityValue += Rng->RandomFloat(WorldPos * 0.03127f) * 32.0f;
                DensityValue += Rng->RandomFloat(WorldPos * 0.015622f) * 64.0f;
                
                DensityValue += Rng->RandomFloat(WorldPos * 0.00392f) * 256.0f;
                DensityValue += Rng->RandomFloat(WorldPos * 0.00192f) * 512.0f;
                
                // DensityValue += Rng->RandomFloat(WorldPos * 0.00098f) * 1024.0f;
                
                // DensityValue += Rng->RandomFloat(WorldPos * 0.00024f) * 4096.0f;
                                                                   
                SetGridPRC(&DensityBlock->Grid, Plane, Row, Column, DensityValue);
            }
        }
    }
}

internal void 
GenerateDensityGrid2(terrain_density_block *DensityBlock, value_noise_generator *Rng, uint32 BlockResolution,
                     real32 Persistence = 0.5f)
{
    ZeroOutGridPoints(&DensityBlock->Grid);
        
    int32 TerrainDimension = (int32)DensityBlock->Grid.Dimension;
    uint32 FirstOctave = 0;//Log2(BlockResolution);
    uint32 OctaveCount = Log2(TerrainDimension);
    uint32 OctaveStep = 2;
    
    for(uint32 Octaves = FirstOctave;
        Octaves < OctaveCount;
        Octaves += OctaveStep)
    {
        /*
        float freq = 2^i;
        float amplitude = persistence^i;
        */
        real32 Weight = Pow(Persistence, OctaveCount-Octaves-1);
        int32 WaveLength = (int32)Pow2(Octaves);
        int32 EdgePlus = Pow2(BlockResolution/WaveLength);
        int32 PGDimension = ((TerrainDimension - 5)*BlockResolution/WaveLength) + 1 + 2*EdgePlus;
        
        real32 Step = (real32)(PGDimension-1-(2*EdgePlus)) / (TerrainDimension - 5);
        int32 IntStep = (Step >= 1.0f) ? (int32)Step : 1;
        
        dynamic_grid3D PerlinGrid{PGDimension};
        /* NOTE: Plane is axis X
                 Row is axis Y
                 Column is axis Z
        */
        for(int32 Plane = 0;
            Plane < PerlinGrid.Dimension;
            Plane += IntStep)
        {
            for(int32 Row = 0;
                Row < PerlinGrid.Dimension;
                Row += IntStep)
            {
                for(int32 Column = 0;
                    Column < PerlinGrid.Dimension;
                    Column += IntStep)
                {
                    int32 WorldX = Plane - EdgePlus + FloorInt32(DensityBlock->Pos.X/WaveLength);
                    int32 WorldY = Row - EdgePlus + FloorInt32(DensityBlock->Pos.Y/WaveLength);
                    int32 WorldZ = Column - EdgePlus + FloorInt32(DensityBlock->Pos.Z/WaveLength);
                    // TODO: Should I multiply these with WaveLength?
                    real32 RandomVal = Rng->RandomFloat((real32)WorldX,
                                                        (real32)WorldY,
                                                        (real32)WorldZ);
                    RandomVal = RandomVal * Weight;
                    real32 Dampening = (WorldY)*((real32)WaveLength/(4.0f * TerrainDimension));
                    PerlinGrid.GetPRC(Plane, Row, Column) = RandomVal + Dampening;
                }
            }
        }
        
        // NOTE: Have to stretch out the little grids to match the size of the terrain
        // During strech, we get the inner points by interpolating between the Perlin grid values
        
        for(int32 Plane = 0;
            Plane < TerrainDimension;
            ++Plane)
        {  
            real32 PlaneRatio = (real32)(Plane-2)*Step + EdgePlus;
            int32 PGPlane = FloorInt32(PlaneRatio);
            PlaneRatio = PlaneRatio - (real32)PGPlane;
            
            for(int32 Row = 0;
                Row < TerrainDimension;
                ++Row)
            {
                real32 RowRatio = (real32)(Row-2)*Step + EdgePlus;
                int32 PGRow = FloorInt32(RowRatio);
                RowRatio = RowRatio - (real32)PGRow;
                
                for(int32 Column = 0;
                    Column < TerrainDimension;
                    ++Column)
                { 
                    real32 ColumnRatio = (real32)(Column-2)*Step + EdgePlus;
                    int32 PGColumn = FloorInt32(ColumnRatio);
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
                    real32 GridVal = GetGridPRC(&DensityBlock->Grid, Plane, Row, Column);
                    SetGridPRC(&DensityBlock->Grid, Plane, Row, Column, GridVal+InnerValue);
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
                     uint32 BlockResolution)
{
    Assert(BlockResolution > 0);
    real32 CellDiff = (real32)BlockResolution;
    color GreenColor = color{0.0, 1.0f, 0.0f, 1.0f};
    v3 PosDiff = {2.0f, 2.0f, 2.0f};
    
    RenderBlock->Pos = DensityBlock->Pos;
    int32 TerrainDimension = DensityBlock->Grid.Dimension;
    uint32 VertexCount = 0;
    for(int32 Plane = 2;
        Plane < TerrainDimension-3;
        Plane += 1)
    {
        for(int32 Row = 2;
            Row < TerrainDimension-3;
            Row += 1)
        {
            for(int32 Column = 2;
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
                Cell.val[0] = GetGridPRC(&DensityBlock->Grid, Plane  , Row+1, Column  );
                Cell.val[1] = GetGridPRC(&DensityBlock->Grid, Plane  , Row+1, Column+1);
                Cell.val[2] = GetGridPRC(&DensityBlock->Grid, Plane  , Row  , Column+1);
                Cell.val[3] = GetGridPRC(&DensityBlock->Grid, Plane  , Row  , Column  );
                Cell.val[4] = GetGridPRC(&DensityBlock->Grid, Plane+1, Row+1, Column  );
                Cell.val[5] = GetGridPRC(&DensityBlock->Grid, Plane+1, Row+1, Column+1);
                Cell.val[6] = GetGridPRC(&DensityBlock->Grid, Plane+1, Row  , Column+1);
                Cell.val[7] = GetGridPRC(&DensityBlock->Grid, Plane+1, Row  , Column  );
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
                        
                    // RenderBlock->Vertices[VertexCount++] = 
                        // Get3DGridVertex((Point0-PosDiff), Normal0, GreenColor);
                    // RenderBlock->Vertices[VertexCount++] = 
                        // Get3DGridVertex((Point1-PosDiff), Normal1, GreenColor);
                    // RenderBlock->Vertices[VertexCount++] = 
                        // Get3DGridVertex((Point2-PosDiff), Normal2, GreenColor);
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









