/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen_terrain.h"

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
                     GridPos.Z + (Scale * (real32)GridY), 
                     Color};
    return Result;
}

void terrain::UpdateAndDrawWireframe(dx_resource &DXResources, uint32 Seed, real32 Persistence)
{
    if(LastSeed != Seed || Persistence != LastPersistence)
    {
        GenerateTerrain(Seed, Persistence);
        LastSeed = Seed;
        LastPersistence = Persistence;
    }
    DrawWireframe(DXResources);
}

void terrain::DrawWireframe(dx_resource &DXResources)
{
    vertex *Vertices = new vertex[FinalVertexCount];
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
                Vertices[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX, GridY, Color);
                Vertices[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX, GridY+1, Color);
            }                                                        
            if((GridX + 1) < TerrainGrid.Dimension)                            
            {                                                        
                Vertices[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX, GridY, Color);
                Vertices[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX+1, GridY, Color);
            }
            if(((GridX + 1) < TerrainGrid.Dimension) && ((GridY + 1) < TerrainGrid.Dimension))
            {
                Vertices[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX, GridY+1, Color);
                Vertices[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX+1, GridY+1, Color);
                Vertices[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX+1, GridY+1, Color);
                Vertices[VertexCount++] = GetGridVertex(&TerrainGrid, GridPos, GridX+1, GridY, Color);
            }
        }
    }
    Assert(FinalVertexCount == VertexCount);
       
    DXResources.LoadResource(ObjectConstantBuffer, &ObjectConstants, sizeof(ObjectConstants));
       
    uint32 stride = sizeof(vertex);
    uint32 offset = 0;
    DXResources.LoadResource(VertexBuffer, Vertices, sizeof(vertex) * FinalVertexCount);    
    
    DXResources.DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    DXResources.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    DXResources.DeviceContext->Draw(FinalVertexCount, 0);
    
    delete[] Vertices;
}

void terrain::Initialize(dx_resource &DXResources, uint32 Seed, real32 Persistence)
{    
    Released = false;
    ObjectConstants.WorldMatrix = XMFLOAT4X4(1, 0, 0, 0,
                                             0, 1, 0, 0,
                                             0, 0, 1, 0,
                                             0, 0, 0, 1);
                                           
    D3D11_BUFFER_DESC ObjectCBDesc = {};
    ObjectCBDesc.ByteWidth = sizeof( object_constants );
    ObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
    ObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ObjectCBDesc.MiscFlags = 0;
    ObjectCBDesc.StructureByteStride = 0;
    
    D3D11_SUBRESOURCE_DATA ObjCBufferData;
    ObjCBufferData.pSysMem = &ObjectConstants;
    ObjCBufferData.SysMemPitch = 0;
    ObjCBufferData.SysMemSlicePitch = 0;
    
    DXResources.Device->CreateBuffer(&ObjectCBDesc, &ObjCBufferData, 
                               &ObjectConstantBuffer);
    DXResources.DeviceContext->VSSetConstantBuffers(1, 1, &ObjectConstantBuffer);  
    
    TerrainDimension = 128;
    TerrainGrid = grid2D{TerrainDimension};
    
    uint32 RowCount = TerrainDimension;
    uint32 ColumnCount = TerrainDimension;
    FinalVertexCount = 2 * (4 * (RowCount-1) * (ColumnCount-1) + RowCount + ColumnCount - 2);
    
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(BufferDesc));
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;            
    BufferDesc.ByteWidth = sizeof(vertex) * FinalVertexCount;        
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;   
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    DXResources.Device->CreateBuffer(&BufferDesc, NULL, &VertexBuffer);
    
    GenerateTerrain(Seed, Persistence);
    LastSeed = Seed;
    LastPersistence = Persistence;
}

void terrain::Release()
{
    ObjectConstantBuffer->Release();
    VertexBuffer->Release();
    Released = true;
}
   
terrain::~terrain()
{
    if(!Released) Release();
}




// Terrain 3D


void terrain3D::Initialize(dx_resource &DXResources, uint32 Seed, real32 Persistence)
{    
    
    TerrainDimension = 64;
    TerrainGrid = grid3D{TerrainDimension};
    
    // uint32 RowCount = TerrainDimension;
    // uint32 ColumnCount = TerrainDimension;
    // FinalVertexCount = 2 * (4 * (RowCount-1) * (ColumnCount-1) + RowCount + ColumnCount - 2);
    FinalVertexCount = TerrainDimension*TerrainDimension*TerrainDimension*18;
    
    GenerateTerrain(Seed, Persistence);
    LastSeed = Seed;
    LastPersistence = Persistence;
    Vertices = CreateRenderVertices();

    DXReleased = false;
    ObjectConstants.WorldMatrix = XMFLOAT4X4(1, 0, 0, 0,
                                             0, 1, 0, 0,
                                             0, 0, 1, 0,
                                             0, 0, 0, 1);
                                           
    D3D11_BUFFER_DESC ObjectCBDesc = {};
    ObjectCBDesc.ByteWidth = sizeof( object_constants );
    ObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
    ObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ObjectCBDesc.MiscFlags = 0;
    ObjectCBDesc.StructureByteStride = 0;
    
    D3D11_SUBRESOURCE_DATA ObjCBufferData;
    ObjCBufferData.pSysMem = &ObjectConstants;
    ObjCBufferData.SysMemPitch = 0;
    ObjCBufferData.SysMemSlicePitch = 0;
    
    DXResources.Device->CreateBuffer(&ObjectCBDesc, &ObjCBufferData, 
                               &ObjectConstantBuffer);
    DXResources.DeviceContext->VSSetConstantBuffers(1, 1, &ObjectConstantBuffer);  
    
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof(BufferDesc));
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;            
    BufferDesc.ByteWidth = sizeof(vertex) * FinalVertexCount;        
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;   
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    DXResources.Device->CreateBuffer(&BufferDesc, NULL, &VertexBuffer);
}

bool32 internal
IsBoundaryPoint(grid3D TerrainGrid, uint32 Plane, uint32 Row, uint32 Column)
{
    bool32 Result = false;
    if(TerrainGrid.GetPRC(Plane, Row, Column) < 0.05f)
    {
        // for(int32 InnerPlane = ((int32)Plane)-1;
            // (InnerPlane < (int32)Plane+2)&&(Result == false);
            // ++InnerPlane)
        // {
            // for(int32 InnerRow = ((int32)Row)-1;
                // (InnerRow < (int32)Row+2)&&(Result == false);
                // ++InnerRow)
            // {
                // for(int32 InnerColumn = ((int32)Column)-1;
                    // (InnerColumn < (int32)Column+2)&&(Result == false);
                    // ++InnerColumn)
                // {
                    // if(TerrainGrid.GetPRC(InnerPlane, InnerRow, InnerColumn) >= 0.05f)
                    // {
                        // Result = true;
                    // }
                // }
            // }
        // }
        int32 InnerPlane = Plane;
        int32 InnerRow = Row;
        int32 InnerColumn = Column;
        if(TerrainGrid.GetPRC(InnerPlane+1, InnerRow, InnerColumn) >= 0.05f)
        {
            Result = true;
        }
        else if(TerrainGrid.GetPRC(InnerPlane-1, InnerRow, InnerColumn) >= 0.05f)
        {
            Result = true;
        }
        else if(TerrainGrid.GetPRC(InnerPlane, InnerRow+1, InnerColumn) >= 0.05f)
        {
            Result = true;
        }
        else if(TerrainGrid.GetPRC(InnerPlane, InnerRow-1, InnerColumn) >= 0.05f)
        {
            Result = true;
        }
        else if(TerrainGrid.GetPRC(InnerPlane, InnerRow, InnerColumn+1) >= 0.05f)
        {
            Result = true;
        }
        else if(TerrainGrid.GetPRC(InnerPlane, InnerRow, InnerColumn-1) >= 0.05f)
        {
            Result = true;
        }
    }
    return Result;
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
        
    // TODO: FinalVertexCount can be 0 here!
    VertexLocations = std::shared_ptr<v3>(new v3[FinalVertexCount]);
    uint32 VertexCount = 0;
    for(int32 Plane = 0;
            Plane < TerrainGrid.Dimension;
            ++Plane)
    {
        for(int32 Row = 0;
            Row < TerrainGrid.Dimension;
            ++Row)
        {
            for(int32 Column = 0;
                Column < TerrainGrid.Dimension;
                ++Column)
            {                
                if(IsBoundaryPoint(TerrainGrid, Plane, Row, Column))
                {
                    VertexLocations.get()[VertexCount++] = v3{Column, Plane, Row};
                }
                
            }
        }
    }
    VertexLocationCount = VertexCount;
}

internal vertex
Get3DGridVertex(v3 GridPos, uint32 GridX, uint32 GridY, uint32 GridZ, color Color)
{
    real32 Scale = 1.0f;
    vertex Result = {GridPos.X + (Scale * (real32)GridX), 
                     GridPos.Y + (Scale * (real32)GridZ), 
                     GridPos.Z + (Scale * (real32)GridY), 
                     Color};
    return Result;
}

internal vertex
Get3DGridVertex(v3 GridPos, v3 LocalPos, color Color)
{
    real32 Scale = 1.0f;
    vertex Result = {GridPos.X + (Scale * (real32)LocalPos.X), 
                     GridPos.Y + (Scale * (real32)LocalPos.Y), 
                     GridPos.Z + (Scale * (real32)LocalPos.Z), 
                     Color};
    return Result;
}

internal bool32
IsInVertices(grid3D TerrainGrid,
             uint32 X, uint32 Y, uint32 Z)
{
    return IsBoundaryPoint(TerrainGrid, Y, Z, X);
}

std::shared_ptr<vertex> terrain3D::CreateRenderVertices()
{
    // vertex *Vertices = new vertex[FinalVertexCount];
    std::shared_ptr<vertex> Vertices = std::shared_ptr<vertex>(new vertex[FinalVertexCount]);
    uint32 VertexCount = 0;
    v3 GridPos = v3{-10.0f, -10.0f, -10.0f};
    for(uint32 VertexLocationIndex = 0;
        VertexLocationIndex < VertexLocationCount;
        ++VertexLocationIndex)
    {
        v3 LocalPos = VertexLocations.get()[VertexLocationIndex];
        color PointColor = color{0.0, 1.0f, 0.0f, 1.0f};
        // if(VertexLocationIndex % 13 == 0) PointColor = color{1.0f, 0.0f, 0.0f, 1.0f};
        
        for(int32 NeighbourX = LocalPos.X;
            NeighbourX < LocalPos.X+2;
            ++NeighbourX)
        {
            // NOTE: We have to skip cooridnates that go out from the grid
            // less than 0 or bigger than the grid size.
            if(NeighbourX < 0 || NeighbourX >= TerrainDimension) continue;
            for(int32 NeighbourY = LocalPos.Y-1;
                NeighbourY < LocalPos.Y+2;
                ++NeighbourY)
            {
                if(NeighbourY < 0 || NeighbourY >= TerrainDimension) continue;
                for(int32 NeighbourZ = LocalPos.Z-1;
                    NeighbourZ < LocalPos.Z+2;
                    ++NeighbourZ)
                {
                    if(NeighbourZ < 0 || NeighbourZ >= TerrainDimension) continue;
                    // NOTE: Skip body diagonals
                    if(NeighbourX != LocalPos.X && NeighbourY != LocalPos.Y && NeighbourZ != LocalPos.Z)
                        continue;
                    if(IsInVertices(TerrainGrid, NeighbourX, NeighbourY, NeighbourZ))
                    {
                        Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, LocalPos, PointColor); 
                        Vertices.get()[VertexCount++] = Get3DGridVertex(GridPos, v3{NeighbourX, NeighbourY, NeighbourZ}, PointColor); 
                    }
                }
            }
        }
            
        // int32 TopX = LocalPos.X+1;
        // int32 TopY = LocalPos.Y+1;
        // int32 TopZ = LocalPos.Z+1;
        // for(int32 NeighbourX = LocalPos.X;
            // NeighbourX <= TopX;
            // ++NeighbourX)
        // {
            // if(NeighbourX < 0 || NeighbourX >= TerrainDimension) continue;
            // for(int32 NeighbourY = LocalPos.Y;
                // NeighbourY <= TopY;
                // ++NeighbourY)
            // {
                // if(NeighbourY < 0 || NeighbourY >= TerrainDimension) continue;
                // for(int32 NeighbourZ = LocalPos.Z;
                    // NeighbourZ <= TopZ;
                    // ++NeighbourZ)
                // {
                    // if(NeighbourZ < 0 || NeighbourZ >= TerrainDimension) continue;
                    // if(IsInVertices(TerrainGrid, NeighbourX, NeighbourY, NeighbourZ))
                    // {
                        // if(NeighbourX == TopX && NeighbourY == TopY && NeighbourZ == TopZ)
                        // {
                            // if(IsInVertices(TerrainGrid, NeighbourX-1, NeighbourY, NeighbourZ)) continue;
                            // if(IsInVertices(TerrainGrid, NeighbourX, NeighbourY-1, NeighbourZ-1)) continue;
                        // }
                        // Vertices[VertexCount++] = Get3DGridVertex(GridPos, LocalPos, PointColor); 
                        // Vertices[VertexCount++] = Get3DGridVertex(GridPos, v3{NeighbourX, NeighbourY, NeighbourZ}, PointColor); 
                    // }
                // }
            // }
        // }
        
        // PointColor = color{0.0f, 0.0f, 1.0f, 1.0f};
        
        // int32 NeighbourX = LocalPos.X-1;
        // int32 NeighbourY = LocalPos.Y;
        // int32 NeighbourZ = LocalPos.Z+1;
        // if(NeighbourX >=0 && NeighbourZ < TerrainDimension &&
           // IsInVertices(TerrainGrid, NeighbourX, NeighbourY, NeighbourZ) &&
           // !IsInVertices(TerrainGrid, NeighbourX, NeighbourY, NeighbourZ - 1))
        // {
            // Vertices[VertexCount++] = Get3DGridVertex(GridPos, LocalPos, PointColor); 
            // Vertices[VertexCount++] = Get3DGridVertex(GridPos, v3{NeighbourX, NeighbourY, NeighbourZ}, PointColor); 
        // }
        // NeighbourX = LocalPos.X;
        // NeighbourY = LocalPos.Y-1;
        // NeighbourZ = LocalPos.Z+1;
        // if(NeighbourY >=0 && NeighbourZ < TerrainDimension &&
           // IsInVertices(TerrainGrid, NeighbourX, NeighbourY, NeighbourZ) &&
           // (IsInVertices(TerrainGrid, NeighbourX, NeighbourY + 1, NeighbourZ)^
            // IsInVertices(TerrainGrid, NeighbourX, NeighbourY, NeighbourZ - 1)) )
        // {
            // Vertices[VertexCount++] = Get3DGridVertex(GridPos, LocalPos, PointColor); 
            // Vertices[VertexCount++] = Get3DGridVertex(GridPos, v3{NeighbourX, NeighbourY, NeighbourZ}, PointColor); 
        // }
        // NeighbourX = LocalPos.X-1;
        // NeighbourY = LocalPos.Y+1;
        // NeighbourZ = LocalPos.Z;
        // if(NeighbourX >=0 && NeighbourY < TerrainDimension &&
           // IsInVertices(TerrainGrid, NeighbourX, NeighbourY, NeighbourZ) &&
           // !IsInVertices(TerrainGrid, NeighbourX, NeighbourY - 1, NeighbourZ))
        // {
            // Vertices[VertexCount++] = Get3DGridVertex(GridPos, LocalPos, PointColor); 
            // Vertices[VertexCount++] = Get3DGridVertex(GridPos, v3{NeighbourX, NeighbourY, NeighbourZ}, PointColor); 
        // }
        // NeighbourX = LocalPos.X-1;
        // NeighbourY = LocalPos.Y+1;
        // NeighbourZ = LocalPos.Z+1;
        // if(NeighbourX >=0 && NeighbourY < TerrainDimension && NeighbourZ < TerrainDimension &&
           // IsInVertices(TerrainGrid, NeighbourX, NeighbourY, NeighbourZ) &&
           // !IsInVertices(TerrainGrid, NeighbourX, NeighbourY - 1, NeighbourZ) &&
           // !IsInVertices(TerrainGrid, NeighbourX + 1, NeighbourY, NeighbourZ - 1))
        // {
            // Vertices[VertexCount++] = Get3DGridVertex(GridPos, LocalPos, PointColor); 
            // Vertices[VertexCount++] = Get3DGridVertex(GridPos, v3{NeighbourX, NeighbourY, NeighbourZ}, PointColor); 
        // }
        // NeighbourX = LocalPos.X+1;
        // NeighbourY = LocalPos.Y-1;
        // NeighbourZ = LocalPos.Z+1;
        // if(NeighbourY >=0 && NeighbourX < TerrainDimension && NeighbourZ < TerrainDimension &&
           // IsInVertices(TerrainGrid, NeighbourX, NeighbourY, NeighbourZ) &&
           // !IsInVertices(TerrainGrid, NeighbourX, NeighbourY + 1, NeighbourZ) &&
           // !IsInVertices(TerrainGrid, NeighbourX - 1, NeighbourY, NeighbourZ - 1))
        // {
            // Vertices[VertexCount++] = Get3DGridVertex(GridPos, LocalPos, PointColor); 
            // Vertices[VertexCount++] = Get3DGridVertex(GridPos, v3{NeighbourX, NeighbourY, NeighbourZ}, PointColor); 
        // }
        // NeighbourX = LocalPos.X+1;
        // NeighbourY = LocalPos.Y+1;
        // NeighbourZ = LocalPos.Z-1;
        // if(NeighbourZ >=0 && NeighbourX < TerrainDimension && NeighbourY < TerrainDimension &&
           // IsInVertices(TerrainGrid, NeighbourX, NeighbourY, NeighbourZ) &&
           // !IsInVertices(TerrainGrid, NeighbourX, NeighbourY - 1, NeighbourZ) &&
           // !IsInVertices(TerrainGrid, NeighbourX - 1, NeighbourY, NeighbourZ + 1))
        // {
            // Vertices[VertexCount++] = Get3DGridVertex(GridPos, LocalPos, PointColor); 
            // Vertices[VertexCount++] = Get3DGridVertex(GridPos, v3{NeighbourX, NeighbourY, NeighbourZ}, PointColor); 
        // }
        
    }
    //Assert(FinalVertexCount == VertexCount);
    
    return Vertices;
}

void terrain3D::UpdateAndDrawWireframe(dx_resource &DXResources, uint32 Seed, real32 Persistence)
{
    if(LastSeed != Seed || Persistence != LastPersistence)
    {
        GenerateTerrain(Seed, Persistence);
        LastSeed = Seed;
        LastPersistence = Persistence;
        Vertices = CreateRenderVertices();
    }
    DrawWireframe(DXResources);
}

void terrain3D::DrawWireframe(dx_resource &DXResources)
{
    
    DXResources.LoadResource(ObjectConstantBuffer, &ObjectConstants, sizeof(ObjectConstants));
       
    uint32 stride = sizeof(vertex);
    uint32 offset = 0;
    DXResources.LoadResource(VertexBuffer, Vertices.get(), sizeof(vertex) * FinalVertexCount);    
    
    DXResources.DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    DXResources.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    DXResources.DeviceContext->Draw(FinalVertexCount, 0);
    
    //delete[] Vertices;
}

void terrain3D::UpdateAndDrawPoints(dx_resource &DXResources, uint32 Seed, real32 Persistence)
{
    if(LastSeed != Seed || Persistence != LastPersistence)
    {
        GenerateTerrain(Seed, Persistence);
        LastSeed = Seed;
        LastPersistence = Persistence;
    }
    DrawPoints(DXResources);
}

void terrain3D::DrawPoints(dx_resource &DXResources)
{
    vertex *Vertices = new vertex[FinalVertexCount];
    uint32 VertexCount = 0;
    v3 GridPos = v3{-10.0f, -10.0f, -10.0f};
    v3 GridPosdX = GridPos + v3{0.2f, 0.0f, 0.0f};
    v3 GridPosdX2 = GridPos - v3{0.2f, 0.0f, 0.0f};
    v3 GridPosdY = GridPos + v3{0.0f, 0.2f, 0.0f};
    v3 GridPosdY2 = GridPos - v3{0.0f, 0.2f, 0.0f};
    v3 GridPosdZ = GridPos + v3{0.0f, 0.0f, 0.2f};
    v3 GridPosdZ2 = GridPos - v3{0.0f, 0.0f, 0.2f};
    for(uint32 GridZ = 0;
        GridZ < TerrainGrid.Dimension;
        ++GridZ)
    {
        for(uint32 GridY = 0;
            GridY < TerrainGrid.Dimension;
            ++GridY)
        {
            for(uint32 GridX = 0;
                GridX < TerrainGrid.Dimension;
                ++GridX)
            {
                real32 TerrainValue = TerrainGrid[GridZ][GridY][GridX];
                color PointColor = {TerrainValue, TerrainValue, TerrainValue, 1.0f};
                if(TerrainValue < 0.05f)
                {
                    PointColor = color{0.0, 1.0f, 0.0f, 1.0f};
                }
                Vertices[VertexCount++] = Get3DGridVertex(GridPosdX, GridX, GridY, GridZ, PointColor); 
                Vertices[VertexCount++] = Get3DGridVertex(GridPosdX2, GridX, GridY, GridZ, PointColor); 
                Vertices[VertexCount++] = Get3DGridVertex(GridPosdY, GridX, GridY, GridZ, PointColor); 
                Vertices[VertexCount++] = Get3DGridVertex(GridPosdY2, GridX, GridY, GridZ, PointColor); 
                Vertices[VertexCount++] = Get3DGridVertex(GridPosdZ, GridX, GridY, GridZ, PointColor); 
                Vertices[VertexCount++] = Get3DGridVertex(GridPosdZ2, GridX, GridY, GridZ, PointColor);             
            }
        }
    }
    //Assert(FinalVertexCount == VertexCount);
       
    DXResources.LoadResource(ObjectConstantBuffer, &ObjectConstants, sizeof(ObjectConstants));
       
    uint32 stride = sizeof(vertex);
    uint32 offset = 0;
    DXResources.LoadResource(VertexBuffer, Vertices, sizeof(vertex) * FinalVertexCount);    
    
    DXResources.DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    DXResources.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    DXResources.DeviceContext->Draw(FinalVertexCount, 0);
    
    delete[] Vertices;
}

void terrain3D::Release()
{
    ObjectConstantBuffer->Release();
    VertexBuffer->Release();
    DXReleased = true;
}
   
terrain3D::~terrain3D()
{
    if(!DXReleased) Release();
}



//Functional Terrain
    
functional_terrain::functional_terrain()
{
    // NOTE: Default Random generation. Not used currently
    // TODO: Rng now gets destroyed, when this callback is used.
    //       Fix RandomGenerator, so that its passed into the lambda properly.
    RandomGenerator Rng(0);
    RandomGeneratorFunction = [&Rng](RandomGeneratorVariables& Vars)
    {
        Rng.Seed = Vars.Seed;
        return Rng.RandomFloat(Vars.Row, Vars.Column);
    };
    // NOTE: Default is linear interpolating between grid points
    InterpolatingFunction = [](InterpolatingFunctionVariables& Vars)
    {
        grid2D &Grid = *(Vars.PerlinGrid);
        uint32 TerrainDimension = Vars.TerrainGrid->Dimension;
        
        real32 RowRatio = (real32)Vars.Row * (Grid.Dimension-1) / (TerrainDimension - 1);
        uint32 PGRow = (uint32)RowRatio;
        RowRatio = RowRatio - (real32)PGRow;
        real32 ColumnRatio = (real32)Vars.Column * (Grid.Dimension-1) / (TerrainDimension - 1);
        uint32 PGColumn = (uint32)ColumnRatio;
        ColumnRatio = ColumnRatio - (real32)PGColumn;
        
        real32 Result =
            (((1.0f-ColumnRatio) * Grid.GetXY(PGRow, PGColumn) + 
             (ColumnRatio) * Grid.GetXY(PGRow, PGColumn+1)) * (1.0f-RowRatio)) +
            ((((1.0f-ColumnRatio) * Grid.GetXY(PGRow+1, PGColumn)) + 
             ((ColumnRatio) * Grid.GetXY(PGRow+1, PGColumn+1))) * (RowRatio));
        return Result;
    };
    // NOTE: Default is linear smoothing between grid points
    SmoothingFunction = [](SmoothingFunctionVariables &Vars)
    {
        grid2D &Grid = *(Vars.StrechedGrid);
        uint32 Row = Vars.Row;
        uint32 Column = Vars.Column;
        real32 Corners, Sides, Center;
        Corners = (Grid.GetXY(Row-1, Column-1) +
                   Grid.GetXY(Row+1, Column-1) +
                   Grid.GetXY(Row+1, Column+1) +
                   Grid.GetXY(Row-1, Column+1)) / 16.0f;
        Sides = (Grid.GetXY(Row-1, Column) +
                 Grid.GetXY(Row, Column-1) +
                 Grid.GetXY(Row+1, Column) +
                 Grid.GetXY(Row, Column+1)) / 8.0f;
        Center = Grid.GetXY(Row, Column) / 4.0f;
        
        return Corners + Sides + Center;
    }; 
}

void functional_terrain::GenerateTerrain(uint32 Seed, real32 Persistence)
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
        SmoothingFunctionVariables Variables;
        Variables.Seed = Seed;
        Variables.Persistence = Persistence;
        Variables.OctaveCount = OctaveCount;
        Variables.OctaveIndex = Octaves;
        Variables.Weight = Weight;
        Variables.WaveLength = WaveLength;
        Variables.PerlinGrid = &PerlinGrid;
        Variables.TerrainGrid = &TerrainGrid;
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
            }
        }
        
        grid2D StrechedPerlinGrid = {TerrainDimension};
        Variables.StrechedGrid = &StrechedPerlinGrid;
        for(uint32 Row = 0;
            Row < TerrainGrid.Dimension;
            ++Row)
        {
            for(uint32 Column = 0;
                Column < TerrainGrid.Dimension;
                ++Column)
            {
                Variables.Row = Row;
                Variables.Column = Column;
                StrechedPerlinGrid[Row][Column] = InterpolatingFunction(Variables);
            }
        }
        
        grid2D SmoothGrid = {TerrainDimension};
        Variables.SmoothGrid = &SmoothGrid;
        for(uint32 Row = 0;
            Row < SmoothGrid.Dimension;
            ++Row)
        {
            for(uint32 Column = 0;
                Column < SmoothGrid.Dimension;
                ++Column)
            {
                Variables.Row = Row;
                Variables.Column = Column;
                SmoothGrid[Row][Column] = SmoothingFunction(Variables);
            }
        }
        
        TerrainGrid += SmoothGrid;
    }
}