/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_terrain.h"
#include "terepgen_marching_cubes.cpp"

inline real32 pow(real32 A, uint32 N)
{
    Assert(N>=0);
    if(N==0) return 1; 
    return A * pow(A, N-1);
}

inline uint32 pow2(uint32 N)
{
    Assert(N>=0);
    if(N==0) return 1; 
    return 2 * pow2(N-1);
}

inline uint32 log2(uint32 N)
{
    Assert(N>=1);
    if(N==1) return 0; 
    return 1 + log2(N/2);
}

internal real32
SmoothGridPointLinearly(grid2D &RoughGrid, uint32 Row, uint32 Column)
{
    real32 Corners, Sides, Center;
    
    Corners = (RoughGrid[Row-1][Column-1] +
               RoughGrid[Row+1][Column-1] +
               RoughGrid[Row+1][Column+1] +
               RoughGrid[Row-1][Column+1]) / 16.0f;
    Sides = (RoughGrid[Row-1][Column] +
             RoughGrid[Row][Column-1] +
             RoughGrid[Row+1][Column] +
             RoughGrid[Row][Column+1]) / 8.0f;
    Center = RoughGrid[Row][Column] / 4.0f;
    
    return Corners + Sides + Center; 
}

void terrain::GenerateTerrain(uint32 Seed, real32 Persistence)
{
    TerrainGrid.ZeroOutGridPoints();
    RandomGenerator Rng(Seed);
        
    uint32 OctaveCount = log2(TerrainDimension);
    
    for(uint32 Octaves = 0;
        Octaves < OctaveCount;
        ++Octaves)
    {
        // float freq = 2^i;
        // float amplitude = persistence^i;
        
        real32 Weight = pow(Persistence, OctaveCount-Octaves-1);
        uint32 WaveLength = pow2(Octaves);
        uint32 PGDimension = (TerrainDimension/WaveLength);
        
        grid2D PerlinGrid = {PGDimension};
        for(uint32 Row = 0;
            Row < PerlinGrid.Dimension;
            ++Row)
        {
            for(uint32 Column = 0;
                Column < PerlinGrid.Dimension;
                ++Column)
            {
                PerlinGrid[Row][Column] = 
                    Rng.RandomFloat(Row*WaveLength, Column*WaveLength) * Weight;
                Assert(PerlinGrid.GetXY(Row,Column) == PerlinGrid[Row][Column]);
            }
        }
        
        grid2D StrechedPerlinGrid = {TerrainDimension};
        for(uint32 Row = 0;
            Row < StrechedPerlinGrid.Dimension;
            ++Row)
        {
                
            real32 RowRatio = (real32)Row * (PGDimension-1) / (TerrainGrid.Dimension - 1);
            uint32 PGRow = (uint32)RowRatio;
            RowRatio = RowRatio - (real32)PGRow;
            
            for(uint32 Column = 0;
                Column < StrechedPerlinGrid.Dimension;
                ++Column)
            {                
                real32 ColumnRatio = (real32)Column * (PGDimension-1) / (TerrainGrid.Dimension - 1);
                uint32 PGColumn = (uint32)ColumnRatio;
                ColumnRatio = ColumnRatio - (real32)PGColumn;
                
                Assert(((RowRatio+1) < PGDimension) && ((ColumnRatio+1) < PGDimension));
                StrechedPerlinGrid[Row][Column] = 
                    (((1.0f-ColumnRatio) * PerlinGrid[PGRow][PGColumn] + 
                     (ColumnRatio) * PerlinGrid[PGRow][PGColumn+1]) * (1.0f-RowRatio)) +
                    ((((1.0f-ColumnRatio) * PerlinGrid[PGRow+1][PGColumn]) + 
                     ((ColumnRatio) * PerlinGrid[PGRow+1][PGColumn+1])) * (RowRatio));
            }
        }
        
        TerrainGrid += StrechedPerlinGrid;
    }
    
    /*grid2D SmoothGrid = {TerrainDimension};
    for(uint32 Row = 0;
        Row < SmoothGrid.Dimension;
        ++Row)
    {
        for(uint32 Column = 0;
            Column < SmoothGrid.Dimension;
            ++Column)
        {
            SmoothGrid[Row][Column] = SmoothGridPointLinearly(TerrainGrid, Row, Column);
        }
    }
    TerrainGrid = SmoothGrid;*/
}

internal vertex
GetGridVertex(grid2D *Grid, v3 GridPos, uint32 GridX, uint32 GridY, color Color)
{
    real32 Scale = 1.0f;
    real32 Height = Scale * 15.0f;
    vertex Result = {GridPos.X + (Scale * (real32)GridX), 
                     GridPos.Y + Height * Grid->GetXY(GridY, GridX), 
                     GridPos.Z + (Scale * (real32)GridY), 1.0f, 
                     0.0f, 0.0f, 0.0f, 1.0f,
                     Color};
    return Result;
}

std::shared_ptr<vertex> terrain::CreateRenderVertices()
{
    std::shared_ptr<vertex> Vertices = std::shared_ptr<vertex>(new vertex[FinalVertexCount]);
    uint32 VertexCount = 0;
    v3 GridPos = v3{10.0f, -10.0f, 10.0f};
    for(uint32 GridY = 0;
        GridY < TerrainGrid.Dimension;
        ++GridY)
    {
        for(uint32 GridX = 0;
            GridX < TerrainGrid.Dimension;
            ++GridX)
        {
            if((GridY + 1) < TerrainGrid.Dimension)
            {
                Vertices.get()[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX, GridY, Color);
                Vertices.get()[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX, GridY+1, Color);
            }                                                        
            if((GridX + 1) < TerrainGrid.Dimension)                            
            {                                                        
                Vertices.get()[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX, GridY, Color);
                Vertices.get()[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX+1, GridY, Color);
            }
            if(((GridX + 1) < TerrainGrid.Dimension) && ((GridY + 1) < TerrainGrid.Dimension))
            {
                Vertices.get()[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX, GridY+1, Color);
                Vertices.get()[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX+1, GridY+1, Color);
                Vertices.get()[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX+1, GridY+1, Color);
                Vertices.get()[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX+1, GridY, Color);
            }
        }
    }
    Assert(FinalVertexCount == VertexCount);
    
    return Vertices;
}

void terrain::Initialize(uint32 Seed, real32 Persistence)
{        
    TerrainDimension = 128;
    TerrainGrid = grid2D{TerrainDimension};
    
    uint32 RowCount = TerrainDimension;
    uint32 ColumnCount = TerrainDimension;
    FinalVertexCount = 2 * (4 * (RowCount-1) * (ColumnCount-1) + RowCount + ColumnCount - 2);
    
    GenerateTerrain(Seed, Persistence);
    LastSeed = Seed;
    LastPersistence = Persistence;
    Vertices = CreateRenderVertices();
}

void terrain::Update(uint32 Seed, real32 Persistence)
{
    if(LastSeed != Seed || Persistence != LastPersistence)
    {
        GenerateTerrain(Seed, Persistence);
        LastSeed = Seed;
        LastPersistence = Persistence;
        Vertices = CreateRenderVertices();
    }
}


// Terrain 3D

void terrain3D::Initialize(uint32 Seed, real32 Persistence)
{      
    TerrainDimension = 64;
    TerrainGrid = grid3D{TerrainDimension};
    
    // NOTE: above 120MB vx buffer size dx11 crashes
    FinalVertexCount = TerrainDimension*TerrainDimension*TerrainDimension*6;
    
    Update(Seed, Persistence, terrain_render_mode::Triangles);
}

void terrain3D::GenerateTerrain(uint32 Seed, real32 Persistence)
{
    TerrainGrid.ZeroOutGridPoints();
    RandomGenerator Rng(Seed);
        
    uint32 OctaveCount = log2(TerrainDimension);
    
    for(uint32 Octaves = 0;
        Octaves < OctaveCount;
        ++Octaves)
    {
        // float freq = 2^i;
        // float amplitude = persistence^i;
        
        real32 Weight = pow(Persistence, OctaveCount-Octaves-1);
        uint32 WaveLength = pow2(Octaves);
        uint32 PGDimension = (TerrainDimension/WaveLength);
        
        grid3D PerlinGrid = {PGDimension};
        for(uint32 Plane = 0;
            Plane < PerlinGrid.Dimension;
            ++Plane)
        {
            for(uint32 Row = 0;
                Row < PerlinGrid.Dimension;
                ++Row)
            {
                for(uint32 Column = 0;
                    Column < PerlinGrid.Dimension;
                    ++Column)
                {
                    PerlinGrid[Plane][Row][Column] = 
                        Rng.RandomFloat(Plane*WaveLength, Row*WaveLength, Column*WaveLength) * Weight 
                        + /*(real32)*/((int32)Plane-1)*(1.0f/PerlinGrid.Dimension/PerlinGrid.Dimension) ;
                }
            }
        }
        
        grid3D StrechedPerlinGrid = {TerrainDimension};
        for(uint32 Plane = 0;
            Plane < StrechedPerlinGrid.Dimension;
            ++Plane)
        {  
            real32 PlaneRatio = (real32)Plane * (PGDimension-1) / (TerrainGrid.Dimension - 1);
            uint32 PGPlane = (uint32)PlaneRatio;
            PlaneRatio = PlaneRatio - (real32)PGPlane;
            
            for(uint32 Row = 0;
                Row < StrechedPerlinGrid.Dimension;
                ++Row)
            {
                real32 RowRatio = (real32)Row * (PGDimension-1) / (TerrainGrid.Dimension - 1);
                uint32 PGRow = (uint32)RowRatio;
                RowRatio = RowRatio - (real32)PGRow;
                
                for(uint32 Column = 0;
                    Column < StrechedPerlinGrid.Dimension;
                    ++Column)
                {                
                    real32 ColumnRatio = (real32)Column * (PGDimension-1) / (TerrainGrid.Dimension - 1);
                    uint32 PGColumn = (uint32)ColumnRatio;
                    ColumnRatio = ColumnRatio - (real32)PGColumn;
                    
                    Assert(((RowRatio+1) < PGDimension) && ((ColumnRatio+1) < PGDimension));
                    StrechedPerlinGrid[Plane][Row][Column] = 
                        ( (1.0f - PlaneRatio) *
                        ((((1.0f-ColumnRatio) * PerlinGrid[PGPlane][PGRow][PGColumn] + 
                         (ColumnRatio) * PerlinGrid[PGPlane][PGRow][PGColumn+1]) * (1.0f-RowRatio)) +
                        ((((1.0f-ColumnRatio) * PerlinGrid[PGPlane][PGRow+1][PGColumn]) + 
                         ((ColumnRatio) * PerlinGrid[PGPlane][PGRow+1][PGColumn+1])) * (RowRatio)))) +
                        (PlaneRatio * 
                        ((((1.0f-ColumnRatio) * PerlinGrid[PGPlane+1][PGRow][PGColumn] + 
                         (ColumnRatio) * PerlinGrid[PGPlane+1][PGRow][PGColumn+1]) * (1.0f-RowRatio)) +
                        ((((1.0f-ColumnRatio) * PerlinGrid[PGPlane+1][PGRow+1][PGColumn]) + 
                         ((ColumnRatio) * PerlinGrid[PGPlane+1][PGRow+1][PGColumn+1])) * (RowRatio))));
                }
            }
        }
        
        TerrainGrid += StrechedPerlinGrid;
    }
}

internal vertex
Get3DGridVertex(v3 GridPos, v3 LocalPos, v3 Normal, color Color)
{
    real32 Scale = 1.0f;
    vertex Result = {GridPos.X + (Scale * LocalPos.X), 
                     GridPos.Y + (Scale * LocalPos.Y), 
                     GridPos.Z + (Scale * LocalPos.Z), 1.0f, 
                     Normal.X, Normal.Y, Normal.Z, 1.0f,
                     Color};
    return Result;
}

std::shared_ptr<vertex> terrain3D::CreateVerticesForPointRendering()
{
    std::shared_ptr<vertex> Vertices = std::shared_ptr<vertex>(new vertex[FinalVertexCount]);
    color PointColor0 = color{0.0, 1.0f, 0.0f, 1.0f};
    v3 Normal = v3{0.0f, 1.0f, 0.0f};
    
    uint32 VertexCount = 0;
    for(uint32 Plane = 0;
        Plane < TerrainGrid.Dimension-1;
        ++Plane)
    {
        for(uint32 Row = 0;
            Row < TerrainGrid.Dimension-1;
            ++Row)
        {
            for(uint32 Column = 0;
                Column < TerrainGrid.Dimension-1;
                ++Column)
            {    
                v3 Pos = v3{Plane, Row, Column};
                v3 dPosPlane = v3{0.1f, 0.0f, 0.0f};
                v3 dPosRow = v3{0.0f, 0.1f, 0.0f};
                v3 dPosColumn = v3{0.0f, 0.0f, 0.1f};
                real32 GridValue = TerrainGrid.GetPRC(Plane, Row, Column);
                color PointColor;
                if(GridValue < 0.05f)
                    PointColor = color{0.0f, 1.0f, 0.0f, 1.0f};
                else
                    PointColor = color{1.0f, 1.0f, 1.0f, 1.0f};
                Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Pos + dPosPlane,  Normal ,PointColor);
                Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Pos - dPosPlane,  Normal ,PointColor);
                Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Pos + dPosRow,    Normal ,PointColor);
                Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Pos - dPosRow,    Normal ,PointColor);
                Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Pos + dPosColumn, Normal ,PointColor);
                Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Pos - dPosColumn, Normal ,PointColor);
                    
            }
        }
    }
    
    return Vertices;
}

std::shared_ptr<vertex> terrain3D::CreateVerticesForWireframeRendering()
{
    std::shared_ptr<vertex> Vertices = std::shared_ptr<vertex>(new vertex[FinalVertexCount]);
    color PointColor = color{0.0, 1.0f, 0.0f, 1.0f};
    color PointColor0 = color{1.0, 0.0f, 0.0f, 1.0f};
    color PointColor1 = color{0.0, 1.0f, 0.0f, 1.0f};
    color PointColor2 = color{0.0, 0.0f, 1.0f, 1.0f};
    
    uint32 VertexCount = 0;
    for(uint32 Plane = 0;
        Plane < TerrainGrid.Dimension-1;
        ++Plane)
    {
        for(uint32 Row = 0;
            Row < TerrainGrid.Dimension-1;
            ++Row)
        {
            for(uint32 Column = 0;
                Column < TerrainGrid.Dimension-1;
                ++Column)
            {                
                GRIDCELL Cell;
                Cell.p[0] = v3{Plane ,  Row+1, Column  };
                Cell.p[1] = v3{Plane ,  Row+1, Column+1};
                Cell.p[2] = v3{Plane ,  Row  , Column+1};
                Cell.p[3] = v3{Plane ,  Row  , Column  };
                Cell.p[4] = v3{Plane+1, Row+1, Column  };
                Cell.p[5] = v3{Plane+1, Row+1, Column+1};
                Cell.p[6] = v3{Plane+1, Row  , Column+1};
                Cell.p[7] = v3{Plane+1, Row  , Column  };
                Cell.val[0] = TerrainGrid.GetPRC(Plane  , Row+1, Column   );
                Cell.val[1] = TerrainGrid.GetPRC(Plane  , Row+1, Column+1 );
                Cell.val[2] = TerrainGrid.GetPRC(Plane  , Row  , Column+1 );
                Cell.val[3] = TerrainGrid.GetPRC(Plane  , Row  , Column   );
                Cell.val[4] = TerrainGrid.GetPRC(Plane+1, Row+1, Column   );
                Cell.val[5] = TerrainGrid.GetPRC(Plane+1, Row+1, Column+1 );
                Cell.val[6] = TerrainGrid.GetPRC(Plane+1, Row  , Column+1 );
                Cell.val[7] = TerrainGrid.GetPRC(Plane+1, Row  , Column   );
                TRIANGLE Triangles[5];
                uint32 TriangleCount = Polygonise(Cell, 0.05f, Triangles);
                
                for(uint32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
                {
                    v3 Point0 = Triangles[TriangleIndex].p[0];
                    v3 Point1 = Triangles[TriangleIndex].p[1];
                    v3 Point2 = Triangles[TriangleIndex].p[2];
                    v3 Normal = Cross(Point2 - Point1, Point0 - Point1);
                    Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Point0, Normal, PointColor);
                    Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Point1, Normal, PointColor);
                    Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Point1, Normal, PointColor);
                    Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Point2, Normal, PointColor);
                    Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Point2, Normal, PointColor);
                    Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Point0, Normal, PointColor);
                }
            }
        }
    }
    
    return Vertices;
}

std::shared_ptr<vertex> terrain3D::CreateRenderVertices()
{
    std::shared_ptr<vertex> Vertices = std::shared_ptr<vertex>(new vertex[FinalVertexCount]);
    color PointColor = color{0.0, 1.0f, 0.0f, 1.0f};
    color PointColor0 = color{1.0, 0.0f, 0.0f, 1.0f};
    color PointColor1 = color{0.0, 1.0f, 0.0f, 1.0f};
    color PointColor2 = color{0.0, 0.0f, 1.0f, 1.0f};
    
    uint32 VertexCount = 0;
    for(uint32 Plane = 0;
        Plane < TerrainGrid.Dimension-1;
        ++Plane)
    {
        for(uint32 Row = 0;
            Row < TerrainGrid.Dimension-1;
            ++Row)
        {
            for(uint32 Column = 0;
                Column < TerrainGrid.Dimension-1;
                ++Column)
            {                
                GRIDCELL Cell;
                Cell.p[0] = v3{Plane ,  Row+1, Column  };
                Cell.p[1] = v3{Plane ,  Row+1, Column+1};
                Cell.p[2] = v3{Plane ,  Row  , Column+1};
                Cell.p[3] = v3{Plane ,  Row  , Column  };
                Cell.p[4] = v3{Plane+1, Row+1, Column  };
                Cell.p[5] = v3{Plane+1, Row+1, Column+1};
                Cell.p[6] = v3{Plane+1, Row  , Column+1};
                Cell.p[7] = v3{Plane+1, Row  , Column  };
                Cell.val[0] = TerrainGrid.GetPRC(Plane  , Row+1, Column   );
                Cell.val[1] = TerrainGrid.GetPRC(Plane  , Row+1, Column+1 );
                Cell.val[2] = TerrainGrid.GetPRC(Plane  , Row  , Column+1 );
                Cell.val[3] = TerrainGrid.GetPRC(Plane  , Row  , Column   );
                Cell.val[4] = TerrainGrid.GetPRC(Plane+1, Row+1, Column   );
                Cell.val[5] = TerrainGrid.GetPRC(Plane+1, Row+1, Column+1 );
                Cell.val[6] = TerrainGrid.GetPRC(Plane+1, Row  , Column+1 );
                Cell.val[7] = TerrainGrid.GetPRC(Plane+1, Row  , Column   );
                TRIANGLE Triangles[5];
                uint32 TriangleCount = Polygonise(Cell, 0.05f, Triangles);
                
                for(uint32 TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
                {
                    v3 Point0 = Triangles[TriangleIndex].p[0];
                    v3 Point1 = Triangles[TriangleIndex].p[1];
                    v3 Point2 = Triangles[TriangleIndex].p[2];
                    // TODO: Ths is the inverse of the real normal but this works properly, why?
                    v3 Normal = Cross(Point1 - Point0, Point2 - Point0);
                    Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Point0, Normal, PointColor1);
                    Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Point1, Normal, PointColor1);
                    Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, Point2, Normal, PointColor1);
                }
            }
        }
    }
    
    return Vertices;
}

void terrain3D::Update(uint32 Seed, real32 Persistence, terrain_render_mode RenderMode)
{
    if(LastSeed != Seed || Persistence != LastPersistence || LastRenderMode != RenderMode)
    {
        GenerateTerrain(Seed, Persistence);
        LastSeed = Seed;
        LastPersistence = Persistence;
        LastRenderMode = RenderMode;
        if(RenderMode == terrain_render_mode::Triangles) 
            Vertices = CreateRenderVertices();
        else if(RenderMode == terrain_render_mode::Points) 
            Vertices = CreateVerticesForPointRendering();
        else 
            Vertices = CreateVerticesForWireframeRendering();
        
    }
}

void terrain3D::Draw(terrainRenderer &Renderer)
{
    if(LastRenderMode != terrain_render_mode::Triangles) 
        Renderer.DrawWireframe(Vertices);
    else 
        Renderer.DrawTriangles(Vertices);
}
