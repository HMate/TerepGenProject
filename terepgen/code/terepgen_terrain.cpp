/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_terrain.h"
#include "terepgen_marching_cubes.cpp"


// Terrain 3D

void terrain3D::Initialize(uint32 BlockDimension, uint32 Seed, real32 Persistence, v3 WorldPos, uint32 CubeSize)
{      
    Loaded = false;
    LastSeed = Seed + 1;
    TerrainDimension = BlockDimension;
    TerrainGrid = grid3D{TerrainDimension};
    GridPos = WorldPos;
    RenderPos = WorldPos;
    
    // NOTE: above 120MB vx buffer size dx11 crashes
    MaxVertexCount = TerrainDimension*TerrainDimension*TerrainDimension*6;
    
    Update(Seed, Persistence, terrain_render_mode::Triangles, CubeSize);
}

void terrain3D::Update(uint32 Seed, real32 Persistence, terrain_render_mode RenderMode, uint32 CubeSize)
{
    if(LastSeed != Seed || Persistence != LastPersistence)
    {
        Loaded = false;
        GenerateTerrain(Seed, Persistence);
        LastSeed = Seed;
        LastPersistence = Persistence;
        LastRenderMode = RenderMode;
        if(RenderMode == terrain_render_mode::Points)  
            Vertices = CreateVerticesForPointRendering();
        else //if(RenderMode == terrain_render_mode::Triangles)
            Vertices = CreateRenderVertices(CubeSize);
        Loaded = true;
    } 
    else if(LastRenderMode != RenderMode)
    {
        Loaded = false;
        if(RenderMode == terrain_render_mode::Points)  
            Vertices = CreateVerticesForPointRendering();
        else if(LastRenderMode == terrain_render_mode::Points) //TODO : at intialization this can fail
            Vertices = CreateRenderVertices(CubeSize);
        LastRenderMode = RenderMode;
        Loaded = true;
    }
}

void terrain3D::GenerateTerrain(uint32 Seed, real32 Persistence)
{
    TerrainGrid.ZeroOutGridPoints();
    RandomGenerator Rng(Seed);
        
    uint32 FirstOctave = 0;
    uint32 OctaveCount = Log2(TerrainDimension);
    
    for(uint32 Octaves = FirstOctave;
        Octaves < OctaveCount;
        ++Octaves)
    {
        // float freq = 2^i;
        // float amplitude = persistence^i;
        
        real32 Weight = Pow(Persistence, OctaveCount-Octaves-1);
        int32 WaveLength = (int32)Pow2(Octaves);
        real32 Frequency = 1.0f / (WaveLength + 1);
        uint32 PGDimension = ((TerrainDimension-1)/WaveLength) + 1;
        
        grid3D PerlinGrid = {PGDimension};
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
                    // NOTE: Only works properly, if GridPoses are apart from each other 
                    //      with value TerrainDimension. To correct this PerlinGrid dimension 
                    //      should be 1 bigger, and during streching we need to check 
                    //      which octave we are in.
                    int32 WorldX = Plane + FloorInt32(GridPos.X/WaveLength);
                    int32 WorldY = Row + FloorInt32(GridPos.Y/WaveLength);
                    int32 WorldZ = Column + FloorInt32(GridPos.Z/WaveLength);
                    real32 RandomVal = Rng.RandomFloat(WorldX * WaveLength,
                                                       WorldY * WaveLength,
                                                       WorldZ * WaveLength); 
                    RandomVal = RandomVal * Weight;
                    real32 Dampening = (WorldY)*(1.0f/(4.0f * TerrainDimension));
                    PerlinGrid[Plane][Row][Column] = RandomVal + Dampening;
                        
                }
            }
        }
        
        // NOTE: Have to stretch out the little grids to match the size of the terrain
        //      During strech, we get the inner points by interpolating between the Perlin grid values 
        grid3D StrechedPerlinGrid = {TerrainDimension};
        for(uint32 Plane = 0;
            Plane < StrechedPerlinGrid.Dimension;
            ++Plane)
        {  
            real32 PlaneRatio = (real32)Plane * (PGDimension-1) / (TerrainDimension - 1);
            uint32 PGPlane = FloorUint32(PlaneRatio);
            PlaneRatio = PlaneRatio - (real32)PGPlane;
            
            for(uint32 Row = 0;
                Row < StrechedPerlinGrid.Dimension;
                ++Row)
            {
                real32 RowRatio = (real32)Row * (PGDimension-1) / (TerrainDimension - 1);
                uint32 PGRow = FloorUint32(RowRatio);
                RowRatio = RowRatio - (real32)PGRow;
                
                for(uint32 Column = 0;
                    Column < StrechedPerlinGrid.Dimension;
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
                        (((1.0f-RowRatio) * 
                            ((1.0f-ColumnRatio) * PerlinGrid[PGPlane][PGRow][PGColumn] + 
                            (ColumnRatio) * PerlinGrid[PGPlane][PGRow][PGColumn+1])) +
                        ( (RowRatio) * 
                            (((1.0f-ColumnRatio) * PerlinGrid[PGPlane][PGRow+1][PGColumn]) + 
                            ( ColumnRatio * PerlinGrid[PGPlane][PGRow+1][PGColumn+1]) ))) +
                        PlaneRatio * 
                        ((1.0f-RowRatio) * 
                            ((1.0f-ColumnRatio) * PerlinGrid[PGPlane+1][PGRow][PGColumn] + 
                            (ColumnRatio) * PerlinGrid[PGPlane+1][PGRow][PGColumn+1]) +
                        ( (RowRatio) * 
                            (((1.0f-ColumnRatio) * PerlinGrid[PGPlane+1][PGRow+1][PGColumn]) + 
                            ( ColumnRatio * PerlinGrid[PGPlane+1][PGRow+1][PGColumn+1]) )));
                    StrechedPerlinGrid[Plane][Row][Column] = InnerValue;
                }
            }
        }
        
        TerrainGrid += StrechedPerlinGrid;
    }
}

internal vertex
Get3DGridVertex(v3 LocalPos, v3 Normal, color Color)
{
    real32 Scale = 1.0f;
    vertex Result = {(Scale * LocalPos.X), 
                     (Scale * LocalPos.Y), 
                     (Scale * LocalPos.Z), 1.0f, 
                     Normal.X, Normal.Y, Normal.Z, 1.0f,
                     Color};
    return Result;
}

std::shared_ptr<vertex> terrain3D::CreateVerticesForPointRendering()
{
    std::shared_ptr<vertex> Vertices = std::shared_ptr<vertex>(new vertex[MaxVertexCount],
        [](vertex *V){delete[] V;});
    color PointColor0 = color{0.0, 1.0f, 0.0f, 1.0f};
    v3 Normal = v3{0.0f, 1.0f, 0.0f};
    
    uint32 VertexCount = 0;
    // NOTE: No marching cubes, indexing to full dimension.
    for(uint32 Plane = 0;
        Plane < TerrainGrid.Dimension;
        ++Plane)
    {
        for(uint32 Row = 0;
            Row < TerrainGrid.Dimension;
            ++Row)
        {
            for(uint32 Column = 0;
                Column < TerrainGrid.Dimension;
                ++Column)
            {    
                v3 Pos = v3{(real32)Plane, (real32)Row, (real32)Column};
                v3 dPosPlane = v3{0.1f, 0.0f, 0.0f};
                v3 dPosRow = v3{0.0f, 0.1f, 0.0f};
                v3 dPosColumn = v3{0.0f, 0.0f, 0.1f};
                real32 GridValue = TerrainGrid.GetPRC(Plane, Row, Column);
                color PointColor;
                if(GridValue < 0.05f)
                    PointColor = color{-GridValue, 1.0f, 0.0f, 1.0f};
                else
                    PointColor = color{GridValue, GridValue, 1.0f, 1.0f};
                Vertices.get()[VertexCount++] = Get3DGridVertex(Pos + dPosPlane,  Normal ,PointColor);
                Vertices.get()[VertexCount++] = Get3DGridVertex(Pos - dPosPlane,  Normal ,PointColor);
                Vertices.get()[VertexCount++] = Get3DGridVertex(Pos + dPosRow,    Normal ,PointColor);
                Vertices.get()[VertexCount++] = Get3DGridVertex(Pos - dPosRow,    Normal ,PointColor);
                Vertices.get()[VertexCount++] = Get3DGridVertex(Pos + dPosColumn, Normal ,PointColor);
                Vertices.get()[VertexCount++] = Get3DGridVertex(Pos - dPosColumn, Normal ,PointColor);
                    
            }
        }
    }
    CurrentVertexCount = VertexCount;
    
#if TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Current Vertex Count: %d\n", CurrentVertexCount);
    OutputDebugStringA(DebugBuffer);
#endif
    return Vertices;
}

// TODO: At the edges of a grid, the normals are distorted.
//  to fix this, the generated grids should contain at least +1 cells in every direction
inline v3
GetPointNormal(grid3D TerrainGrid, v3 Point)
{    
    real32 Diff = 1.0f;
    real32 DimensionBound = (real32)TerrainGrid.Dimension-1.0f;
    
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
        TerrainGrid.GetPRCWithInterpolate(DiffXMax, Point.Y, Point.Z) -
        TerrainGrid.GetPRCWithInterpolate(DiffXMin, Point.Y, Point.Z);
    real32 NormalY = 
        TerrainGrid.GetPRCWithInterpolate(Point.X, DiffYMax, Point.Z) -
        TerrainGrid.GetPRCWithInterpolate(Point.X, DiffYMin, Point.Z);
    real32 NormalZ = 
        TerrainGrid.GetPRCWithInterpolate(Point.X, Point.Y, DiffZMax) -
        TerrainGrid.GetPRCWithInterpolate(Point.X, Point.Y, DiffZMin);
        
    v3 Result = v3{NormalX, NormalY, NormalZ};
    
    return Result;
}

std::shared_ptr<vertex> terrain3D::CreateRenderVertices(uint32 CubeSize)
{
    Assert(CubeSize > 0);
    real32 CellDiff = (real32)CubeSize;
    color GreenColor = color{0.0, 1.0f, 0.0f, 1.0f};
    
    std::shared_ptr<vertex> Vertices = std::shared_ptr<vertex>(new vertex[MaxVertexCount],
        [](vertex *V){delete[] V;});
    
    uint32 VertexCount = 0;
    // NOTE: Using marching cubes, so we index to dim-1, to make cubes
    for(uint32 Plane = 0;
        Plane < TerrainGrid.Dimension-1;
        Plane += CubeSize)
    {
        for(uint32 Row = 0;
            Row < TerrainGrid.Dimension-1;
            Row += CubeSize)
        {
            for(uint32 Column = 0;
                Column < TerrainGrid.Dimension-1;
                Column += CubeSize)
            {                
                GRIDCELL Cell;
                real32 Planef = (real32)Plane;
                real32 Rowf = (real32)Row;
                real32 Columnf = (real32)Column;
                Cell.p[0] = v3{Planef         , Rowf+CellDiff, Columnf         };
                Cell.p[1] = v3{Planef         , Rowf+CellDiff, Columnf+CellDiff};
                Cell.p[2] = v3{Planef         , Rowf         , Columnf+CellDiff};
                Cell.p[3] = v3{Planef         , Rowf         , Columnf         };
                Cell.p[4] = v3{Planef+CellDiff, Rowf+CellDiff, Columnf         };
                Cell.p[5] = v3{Planef+CellDiff, Rowf+CellDiff, Columnf+CellDiff};
                Cell.p[6] = v3{Planef+CellDiff, Rowf         , Columnf+CellDiff};
                Cell.p[7] = v3{Planef+CellDiff, Rowf         , Columnf         };
                Cell.val[0] = TerrainGrid.GetPRC(Plane         , Row+CubeSize, Column         );
                Cell.val[1] = TerrainGrid.GetPRC(Plane         , Row+CubeSize, Column+CubeSize);
                Cell.val[2] = TerrainGrid.GetPRC(Plane         , Row         , Column+CubeSize);
                Cell.val[3] = TerrainGrid.GetPRC(Plane         , Row         , Column         );
                Cell.val[4] = TerrainGrid.GetPRC(Plane+CubeSize, Row+CubeSize, Column         );
                Cell.val[5] = TerrainGrid.GetPRC(Plane+CubeSize, Row+CubeSize, Column+CubeSize);
                Cell.val[6] = TerrainGrid.GetPRC(Plane+CubeSize, Row         , Column+CubeSize);
                Cell.val[7] = TerrainGrid.GetPRC(Plane+CubeSize, Row         , Column         );
                TRIANGLE Triangles[5];
                uint32 TriangleCount = Polygonise(Cell, 0.05f, Triangles);
                
                for(uint32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
                {
                    v3 Point0 = Triangles[TriangleIndex].p[0];
                    v3 Point1 = Triangles[TriangleIndex].p[1];
                    v3 Point2 = Triangles[TriangleIndex].p[2];
                                        
                    v3 Normal0 = GetPointNormal(TerrainGrid, Point0);
                    v3 Normal1 = GetPointNormal(TerrainGrid, Point1);
                    v3 Normal2 = GetPointNormal(TerrainGrid, Point2);
                    
                    Vertices.get()[VertexCount++] = Get3DGridVertex(Point0, Normal0, GreenColor);
                    Vertices.get()[VertexCount++] = Get3DGridVertex(Point1, Normal1, GreenColor);
                    Vertices.get()[VertexCount++] = Get3DGridVertex(Point2, Normal2, GreenColor);
                }
            }
        }
    }
    CurrentVertexCount = VertexCount;
    
#if TEREPGEN_DEBUG
    char DebugBuffer[256];
    sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Current Vertex Count: %d\n", CurrentVertexCount);
    OutputDebugStringA(DebugBuffer);
#endif
    return Vertices;
}

void terrain3D::Draw(terrainRenderer *Renderer)
{
    Renderer->SetTransformations(RenderPos);
    if(LastRenderMode != terrain_render_mode::Triangles) 
        Renderer->DrawWireframe(Vertices.get(), CurrentVertexCount);
    else 
        Renderer->DrawTriangles(Vertices.get(), CurrentVertexCount);
    Renderer->SetTransformations(v3{});
}
