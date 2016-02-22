/*
    Terep generátor by Hidvégi Máté @2015

*/


// NOTE: Block Resolution gives how many density values are skipped
// This way a bigger area can be stored in the same block, 
// if at rendering we only use every BlockResolution'th value too.
internal void 
GenerateDensityGrid(terrain_density_block *DensityBlock, perlin_noise_array *PNArray, uint32 BlockResolution)
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
                // real32 DensityValue = 0;
                
                real32 WorldX = DensityBlock->Pos.X + ((Plane-2) * (real32)BlockResolution);
                real32 WorldY = DensityBlock->Pos.Y + ((Row-2) * (real32)BlockResolution);
                real32 WorldZ = DensityBlock->Pos.Z + ((Column-2) * (real32)BlockResolution);
                
                v3 WorldPos = v3{WorldX, WorldY, WorldZ} / 32.0f;
                real32 Scale = 50.0f;
                             
                // DensityValue += RandomFloat(&PNArray->Noise[0], WorldPos * 256.03f) * Scale * 0.0036025f;
                // DensityValue += RandomFloat(&PNArray->Noise[1], WorldPos * 128.96f) * Scale * 0.0078125f;
                //win32_clock Clock;
                DensityValue += RandomFloat(&PNArray->Noise[2], WorldPos * 64.01f)  * Scale * 0.015625f;
                //Clock.PrintMiliSeconds("Perlin gen time:");
                
                DensityValue += RandomFloat(&PNArray->Noise[0], WorldPos * 32.03f) * Scale * 0.03125f;
                DensityValue += RandomFloat(&PNArray->Noise[1], WorldPos * 16.16f) * Scale * 0.0625f;
                // DensityValue += RandomFloat(&PNArray->Noise[2], WorldPos * 7.91f)  * Scale * 0.125f;
                
                // DensityValue += RandomFloat(&PNArray->Noise[0], WorldPos * 4.03f) * Scale * 0.25f;
                DensityValue += RandomFloat(&PNArray->Noise[1], WorldPos * 1.96f) * Scale * 0.5f;
                // DensityValue += RandomFloat(&PNArray->Noise[2], WorldPos * 1.01f) * Scale * 1.0f;
                DensityValue += RandomFloat(&PNArray->Noise[2], WorldPos * 0.491f) * Scale * 1.0f;
                
                DensityValue += RandomFloat(&PNArray->Noise[0], WorldPos * 0.023f) * Scale * 4.0f;
                DensityValue += RandomFloat(&PNArray->Noise[1], WorldPos * 0.00646f) * Scale * 16.0f;
                
                SetGrid(&DensityBlock->Grid, Plane, Row, Column, DensityValue);
            }
        }
    }
}

internal vertex
Get3DVertex(v3 LocalPos, v3 Normal, v4 Color)
{
    vertex Result = {LocalPos.X, LocalPos.Y, LocalPos.Z, 
                     Normal.X, Normal.Y, Normal.Z,
                     Color};
    return Result;
}

internal v3
GetPointNormal(terrain_density_block *DensityBlock, v3 Point)
{    
    // TODO: Compute normals from adjecent density blocks, 
    // instead of generating adjecent blocks points in current block
    real32 Diff = 0.5f;
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
    
    real32 XP = GetGridWithInterpolate(&DensityBlock->Grid, DiffXMax, Point.Y, Point.Z);
    real32 XM = GetGridWithInterpolate(&DensityBlock->Grid, DiffXMin, Point.Y, Point.Z);
    real32 NormalX = XP - XM;
    real32 YP = GetGridWithInterpolate(&DensityBlock->Grid, Point.X, DiffYMax, Point.Z);
    real32 YM = GetGridWithInterpolate(&DensityBlock->Grid, Point.X, DiffYMin, Point.Z);
    real32 NormalY = YP - YM;
    real32 ZP = GetGridWithInterpolate(&DensityBlock->Grid, Point.X, Point.Y, DiffZMax);
    real32 ZM = GetGridWithInterpolate(&DensityBlock->Grid, Point.X, Point.Y, DiffZMin);
    real32 NormalZ = ZP - ZM;
        
    v3 Result = v3{NormalX, NormalY, NormalZ};
    Result = Normalize(Result);
    return Result;
}

#define DENSITY_ISO_LEVEL 0.0f
/*
    NOTE: The dimension of the grids required to be 5 units bigger
        than the size of the final terrain
        +1 in each dimension for marching cubes
        +2 at each side to calculate proper normals
*/
internal void
PoligoniseBlock(terrain_render_block *RenderBlock, terrain_density_block *DensityBlock,
                     uint32 BlockResolution)
{
    Assert(BlockResolution > 0);
    real32 CellDiff = (real32)BlockResolution;
    v4 GreenColor = v4{0.0, 1.0f, 0.0f, 1.0f};
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
                Cell.p[0] = v3{Planef     , Rowf+1.0f, Columnf     };
                Cell.p[1] = v3{Planef     , Rowf+1.0f, Columnf+1.0f};
                Cell.p[2] = v3{Planef     , Rowf     , Columnf+1.0f};
                Cell.p[3] = v3{Planef     , Rowf     , Columnf     };
                Cell.p[4] = v3{Planef+1.0f, Rowf+1.0f, Columnf     };
                Cell.p[5] = v3{Planef+1.0f, Rowf+1.0f, Columnf+1.0f};
                Cell.p[6] = v3{Planef+1.0f, Rowf     , Columnf+1.0f};
                Cell.p[7] = v3{Planef+1.0f, Rowf     , Columnf     };
                Cell.val[0] = GetGrid(&DensityBlock->Grid, Plane  , Row+1, Column  );
                Cell.val[1] = GetGrid(&DensityBlock->Grid, Plane  , Row+1, Column+1);
                Cell.val[2] = GetGrid(&DensityBlock->Grid, Plane  , Row  , Column+1);
                Cell.val[3] = GetGrid(&DensityBlock->Grid, Plane  , Row  , Column  );
                Cell.val[4] = GetGrid(&DensityBlock->Grid, Plane+1, Row+1, Column  );
                Cell.val[5] = GetGrid(&DensityBlock->Grid, Plane+1, Row+1, Column+1);
                Cell.val[6] = GetGrid(&DensityBlock->Grid, Plane+1, Row  , Column+1);
                Cell.val[7] = GetGrid(&DensityBlock->Grid, Plane+1, Row  , Column  );
                TRIANGLE Triangles[5];
                uint32 TriangleCount = Polygonise(Cell, DENSITY_ISO_LEVEL, Triangles);
                
                for(uint32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
                {
                    v3 Point0 = Triangles[TriangleIndex].p[0];
                    v3 Point1 = Triangles[TriangleIndex].p[1];
                    v3 Point2 = Triangles[TriangleIndex].p[2];
                                        
                    v3 Normal0 = GetPointNormal(DensityBlock, Point0);
                    v3 Normal1 = GetPointNormal(DensityBlock, Point1);
                    v3 Normal2 = GetPointNormal(DensityBlock, Point2);
                    
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex((Point0-PosDiff) * CellDiff, Normal0, GreenColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex((Point1-PosDiff) * CellDiff, Normal1, GreenColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex((Point2-PosDiff) * CellDiff, Normal2, GreenColor);
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
                    v3 Pos = (Point0-PosDiff);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex(Pos * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex((Pos + Normal0) * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex((Pos + 0.9f*Normal0 + 0.1f*NormalPerpend) * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex(Pos * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex((Pos + 0.9f*Normal0 + 0.1f*NormalPerpend) * CellDiff, NormalNormal, BlueColor);
                    RenderBlock->Vertices[VertexCount++] = 
                        Get3DVertex((Pos + Normal0) * CellDiff, NormalNormal, BlueColor);
#endif
                }
            }
        }
    }
    RenderBlock->VertexCount = VertexCount;
    
#if 0 //TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Current Vertex Count: %d\n", VertexCount);
    OutputDebugStringA(DebugBuffer);
#endif
}









