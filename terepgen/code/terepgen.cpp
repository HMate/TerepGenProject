/*
    Terep generátor by Hidvégi Máté @2015

*/

#include "terepgen.h"

#include "terepgen_random.cpp"

internal int32 
getGridSize(const int32 n)
{
    return ((int32)(((4.0*n*n*n) + n*8.0 )/3.0 ) + (2*n*n) + 1);
}

vertex Vertex(v3 Pos, v3 Norm, v4 Color)
{
    vertex Result;
    Result.X = Pos.X;
    Result.Y = Pos.Y;
    Result.Z = Pos.Z;
    Result.NX = Norm.X;
    Result.NY = Norm.Y;
    Result.NZ = Norm.Z;
    Result.Color = Color;
    
    return Result;
}

internal void
AddCubeWireframe(cube_frame *Cube, v3 Pos, real32 Size, v4 Color)
{
    const v3 Corner0 = Pos + v3{-0.5f, -0.5f, -0.5f}*Size;
    const v3 Corner1 = Pos + v3{-0.5f, 0.5f, -0.5f}*Size;
    const v3 Corner2 = Pos + v3{0.5f, -0.5f, -0.5f}*Size;
    const v3 Corner3 = Pos + v3{0.5f, 0.5f, -0.5f}*Size;
    const v3 Corner4 = Pos + v3{-0.5f, -0.5f, 0.5f}*Size;
    const v3 Corner5 = Pos + v3{-0.5f, 0.5f, 0.5f}*Size;
    const v3 Corner6 = Pos + v3{0.5f, -0.5f, 0.5f}*Size;
    const v3 Corner7 = Pos + v3{0.5f, 0.5f, 0.5f}*Size;
    
    const v3 Normal = v3{0.0f, 0.0f, 1.0f};
    
    Cube->Vertices[0 ] = Vertex(Corner0, Normal, Color);
    Cube->Vertices[1 ] = Vertex(Corner1, Normal, Color);
    Cube->Vertices[2 ] = Vertex(Corner0, Normal, Color);
    Cube->Vertices[3 ] = Vertex(Corner2, Normal, Color);
    Cube->Vertices[4 ] = Vertex(Corner2, Normal, Color);
    Cube->Vertices[5 ] = Vertex(Corner3, Normal, Color);
    Cube->Vertices[6 ] = Vertex(Corner1, Normal, Color);
    Cube->Vertices[7 ] = Vertex(Corner3, Normal, Color);
                   
    Cube->Vertices[8 ] = Vertex(Corner4, Normal, Color);
    Cube->Vertices[9 ] = Vertex(Corner5, Normal, Color);
    Cube->Vertices[10] = Vertex(Corner4, Normal, Color);
    Cube->Vertices[11] = Vertex(Corner6, Normal, Color);
    Cube->Vertices[12] = Vertex(Corner6, Normal, Color);
    Cube->Vertices[13] = Vertex(Corner7, Normal, Color);
    Cube->Vertices[14] = Vertex(Corner5, Normal, Color);
    Cube->Vertices[15] = Vertex(Corner7, Normal, Color);
                   
    Cube->Vertices[16] = Vertex(Corner0, Normal, Color);
    Cube->Vertices[17] = Vertex(Corner4, Normal, Color);
    Cube->Vertices[18] = Vertex(Corner1, Normal, Color);
    Cube->Vertices[19] = Vertex(Corner5, Normal, Color);
    Cube->Vertices[20] = Vertex(Corner2, Normal, Color);
    Cube->Vertices[21] = Vertex(Corner6, Normal, Color);
    Cube->Vertices[22] = Vertex(Corner3, Normal, Color);
    Cube->Vertices[23] = Vertex(Corner7, Normal, Color);
}

internal void
LoadSessionDesc(game_state *GameState, uint32 SessionID)
{
    char *SessionFile = "sessions.txt";
    FileHandle Handle = PlatformOpenOrCreateFileForWrite(SessionFile);
    
    //NOTE: Read blocks until we find the one we need
    bool32 NotFound = true;
    bool32 EndOfFile = false;
    
    session_description ReadSession;
    const uint32 SessionDescSizeInBytes = sizeof(session_description);
    while(NotFound && !EndOfFile)
    {
        uint32 BytesRead = PlatformReadFile(Handle, &ReadSession, SessionDescSizeInBytes);
        EndOfFile = (BytesRead == 0);
        Assert(BytesRead == SessionDescSizeInBytes || EndOfFile);
        if(ReadSession.Id == SessionID)
        {
            NotFound = false;
            // NOTE: Set the file pointer to the begining of the block, to overwrite it
            GameState->Session = ReadSession;
        }
    }
    
    if(NotFound)
    {
        GameState->Session.Id = SessionID;
        
        char SessionStorePath[MAX_PATH];
        sprintf_s(SessionStorePath, "dynamicStore%d.txt", SessionID);
        CopyString((char*)&GameState->Session.DynamicStore, SessionStorePath);
        
        uint32 BytesWritten = PlatformWriteFile(Handle, &GameState->Session, SessionDescSizeInBytes);
        Assert(BytesWritten == SessionDescSizeInBytes);
    }
    CloseHandle(Handle);
}

void
UpdateAndRenderGame(game_memory *Memory, game_input *Input, screen_info ScreenInfo, bool32 Resize)
{
    Assert(sizeof(game_state) < Memory->PermanentStorageSize);
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    render_state *RenderState = &GameState->RenderState;
    terrain *Terrain = &GameState->Terrain;
    
    if(GameState->Initialized == false)
    {
        // TODO: WorldDensity should be in WordArena! -> lets put it in there!
        InitializeArena(&GameState->WorldArena, (uint8 *)Memory->PermanentStorage + sizeof(game_state), 
                        Memory->PermanentStorageSize - sizeof(game_state));
        
        Terrain->FixedResolution[0] = 8;
        Terrain->FixedResolution[1] = 4;
        Terrain->FixedResolution[2] = 2;
        Terrain->StoreResolutionCount = RESOLUTION_COUNT-2;
        Terrain->MaxResolutionToRender = RESOLUTION_COUNT-2;
        
        LoadSessionDesc(GameState, 321421);
        
        Terrain->Seed = 1000;
        Terrain->BlockSize = real32(TERRAIN_BLOCK_SIZE);
        SetSeed(&Terrain->PerlinArray.Noise[0], Terrain->Seed);
        SetSeed(&Terrain->PerlinArray.Noise[1], Terrain->Seed+1);
        SetSeed(&Terrain->PerlinArray.Noise[2], Terrain->Seed+2);
        InitializeTerrain(Terrain);
        
        GameState->Initialized = true;
    }
    
    // NOTE: Initialize transient state
    Assert(sizeof(transient_state) < Memory->TransientStorageSize);
    transient_state *TranState = (transient_state *)Memory->TransientStorage;
    memory_arena *TranArena = &TranState->TranArena;
    if(TranState->Initialized == false)
    {
        InitializeArena(TranArena, (uint8 *)Memory->TransientStorage + sizeof(transient_state), 
                        Memory->TransientStorageSize - sizeof(transient_state));
        TranState->Initialized = true;
    }
    
    // NOTE: Initialize render state
    camera *Camera = &RenderState->Camera;
    if(RenderState->Initialized == false)
    {
        temporary_memory RenderMemory = BeginTemporaryMemory(TranArena);
        HRESULT HResult = RenderState->DXResources.Initialize(TranArena, ScreenInfo.Width, ScreenInfo.Height);
        if(FAILED(HResult))
        {
            char* ErrMsg = RenderState->DXResources.GetDebugMessage(HResult);
            logger::DebugPrint("Initialize error: %s", ErrMsg);
#if TEREPGEN_DEBUG
            char DebugBuffer[256];
            sprintf_s(DebugBuffer, "[TEREPGEN_DEBUG] Initialize error: %s\n", ErrMsg);
            MessageBox(NULL, DebugBuffer, NULL, MB_OK);
#endif
            RenderState->DXResources.Release();
            TerminateGame();
            return;
        }
        
        RenderState->Camera.Initialize(&RenderState->DXResources, ScreenInfo.Width, ScreenInfo.Height, 20.0f);
        
        EndTemporaryMemory(&RenderMemory);
        RenderState->Initialized = true;
    }
    
    if(Resize)
    {
        HRESULT HResult = RenderState->DXResources.Resize(ScreenInfo.Width, ScreenInfo.Height);
        if(FAILED(HResult)) 
        {
            char* ErrMsg = RenderState->DXResources.GetDebugMessage(HResult);
            logger::DebugPrint("Resize error: %s", ErrMsg);
            
            TerminateGame();
            return;
        }
        Camera->Resize(ScreenInfo.Width, ScreenInfo.Height);
    }
    
    GameState->RenderMode = Input->RenderMode;
    
    Camera->Update(Input, GameState->dtForFrame);
    
    v3 CameraP = Camera->GetPos();
    v3 CamDir = Camera->GetLookDirection();
    
    //NOTE: calculate mouse position in game space
    v2 MouseInPixel = v2{(real32)Input->MouseX, (real32)Input->MouseY};
    real32 WorldScreenSizeY = 2.0f*Tan(Camera->Fov/2.0f);
    real32 WorldScreenSizeX = WorldScreenSizeY*(real32)ScreenInfo.Width/ScreenInfo.Height;
    v3 UpDir = Camera->GetUpDirection();
    v3 RightDir = Normalize(Cross(UpDir, CamDir));
    v2 NormalizedMouse = MouseInPixel - v2{(real32)ScreenInfo.Width/2, (real32)ScreenInfo.Height/2};
    NormalizedMouse.X = NormalizedMouse.X / ScreenInfo.Width;
    NormalizedMouse.Y = -NormalizedMouse.Y / ScreenInfo.Height;
    
    v3 WorldMousePos = CameraP + (UpDir*NormalizedMouse.Y*WorldScreenSizeY) 
        + (RightDir*NormalizedMouse.X*WorldScreenSizeX);
        
    RenderState->CameraOrigo = CameraP + Normalize(Cross(UpDir, RightDir));
    
    
    temporary_memory GeneratorMemory = BeginTemporaryMemory(TranArena);
    
    timer Clock;
    generator_position GeneratorPosition = CalculateTerrainGeneratorPositon(Terrain, CameraP);
    ClearFarawayBlocks(TranArena, Terrain, &GameState->Session, &GeneratorPosition);
    Clock.Reset();
        
    dx_resource *DXResources = &RenderState->DXResources;
    GenerateTerrainBlocks(TranArena, Terrain, Input, &GameState->Session, &GeneratorPosition,
                          WorldMousePos, RenderState->CameraOrigo, &GameState->Cube,
                          CameraP, CamDir, DXResources);
        
    real64 TimeAddToRender = Clock.GetSecondsElapsed();
    
    //
    // RENDER
    //
    
    Clock.Reset();
    
    DXResources->LoadResource(Camera->SceneConstantBuffer,
                  &Camera->SceneConstants, sizeof(Camera->SceneConstants));
    
    DXResources->ClearViews();
    
    // NOTE: Background rendering
    DXResources->SetDrawModeDefault();
    DXResources->DeviceContext->IASetInputLayout(DXResources->BackgroundInputLayout);
    DXResources->DeviceContext->VSSetShader(DXResources->BackgroundVS, 0, 0);
    DXResources->DeviceContext->PSSetShader(DXResources->BackgroundPS, 0, 0);
    DXResources->DeviceContext->PSSetSamplers(0, 1, &DXResources->CubeTexSamplerState);
    
    v3 BGVertices[6] = {{-1.0, -1.0, 0.99f},
                        {-1.0,  1.0, 0.99f},
                        { 1.0, -1.0, 0.99f},
                        { 1.0, -1.0, 0.99f},
                        {-1.0,  1.0, 0.99f},
                        { 1.0,  1.0, 0.99f}};
    DXResources->SetTransformations(v3{});
    DXResources->ObjectConstants.CameraDir = DirectX::XMFLOAT4(CamDir.X, CamDir.Y, CamDir.Z, 0.0f);
    DXResources->DrawBackground(BGVertices, 6);
    
    //DXResources->DeviceContext->PSSetSamplers(0, 1, &DXResources->TexSamplerState);
    DXResources->DeviceContext->IASetInputLayout(DXResources->TerrainInputLayout);
    if(Input->ShowDebugAxis)
    {
        // NOTE: Render axis
        DXResources->SetDrawModeDefault();
        DXResources->DeviceContext->VSSetShader(DXResources->TerrainVS, 0, 0);
        DXResources->DeviceContext->PSSetShader(DXResources->LinePS, 0, 0);
        DXResources->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        
        const real32 AxisSize = 256;
        const uint32 VertCount = 6;
        const v4 Red{1.0f, 0.0f, 0.0f, 1.0f};
        const v4 Green{0.0f, 1.0f, 0.0f, 1.0f}; 
        const v4 Blue{0.0f, 0.0f, 1.0f, 1.0f};
        const v3 Normal{0.0f, 1.0f, 0.0f};
        vertex AxisVertices[VertCount]={Vertex(v3{ 1.0f*AxisSize,  0.0f,  0.0f}, Normal, Red),
                                        Vertex(v3{-1.0f*AxisSize,  0.0f,  0.0f}, Normal, Red),
                                        Vertex(v3{ 0.0f,  1.0f*AxisSize,  0.0f}, Normal, Green),
                                        Vertex(v3{ 0.0f, -1.0f*AxisSize,  0.0f}, Normal, Green),
                                        Vertex(v3{ 0.0f,  0.0f,  1.0f*AxisSize}, Normal, Blue),
                                        Vertex(v3{ 0.0f,  0.0f, -1.0f*AxisSize}, Normal, Blue)};
        DXResources->DrawLines(AxisVertices, VertCount);
        // DXResources->DrawDebugTriangle();
    }
    
    if(GameState->RenderMode)
    {
        DXResources->SetDrawModeWireframe();
    }
    else
    {
        DXResources->SetDrawModeDefault();
    }
    
    DXResources->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DXResources->DeviceContext->VSSetShader(DXResources->TerrainVS, 0, 0);
    DXResources->DeviceContext->PSSetShader(DXResources->TerrainPS, 0, 0);
    DXResources->DeviceContext->PSSetSamplers(0, 1, &DXResources->TexSamplerState);
    
    for(size_t RenderBlockIndex = 0; 
        RenderBlockIndex < Terrain->RenderBlockCount; 
        RenderBlockIndex++)
    {
        terrain_block_model *Block = Terrain->RenderBlocks[RenderBlockIndex];
        DXResources->SetTransformations(Block->Pos);
        DXResources->DrawVertices(&(Block->Buffer), Block->VertexCount);
        // DXResources->DrawTriangles(
            // Terrain->RenderBlocks[RenderBlockIndex]->Vertices,
            // Terrain->RenderBlocks[RenderBlockIndex]->VertexCount);
    }
    DXResources->SetTransformations(v3{});
    
    // NOTE: Draw cube
    DXResources->DeviceContext->PSSetShader(DXResources->LinePS, 0, 0);
    
    // DXResources->SetTransformations(GameState->CubePos);
    DXResources->DrawTriangles(GameState->Cube.Vertices, CubeVertexCount);
    DXResources->SetTransformations(v3{});
    
    if(Input->ShowDebugGrid)
    {
        // NOTE: Draw debug resolution blocks
        GameState->DebugBlockFrameCount = 0;
        DXResources->SetDrawModeWireframe();
        DXResources->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        
        for(uint32 ResolutionIndex = 0;
            ResolutionIndex < Terrain->StoreResolutionCount;
            ResolutionIndex++)
        {
            density_block_pos_array *BlockPositions = Terrain->DensityPositionStore + ResolutionIndex;
            for(size_t BlockPosIndex = 0; 
                (BlockPosIndex < BlockPositions->Count);
                ++BlockPosIndex)
            {
                world_block_pos *BlockP = BlockPositions->Pos + BlockPosIndex;
                int32 MappedRes = GetBlockMappedResolution(Terrain, BlockP);
                if(MappedRes == BlockP->Resolution)
                {
                    v3 RenderPos = V3FromWorldPos(*BlockP);
                    real32 Size = (RENDER_SPACE_UNIT*TERRAIN_BLOCK_SIZE*BlockP->Resolution);
                    RenderPos += v3{0.5f, 0.5f, 0.5f}*Size;
                    
                    v4 Color{1.0f, 1.0f, 0.0f, 1.0f};
                    if(BlockP->Resolution > 4)
                    {
                        Color = v4{0.0f, 0.3f, 0.0f, 1.0f};
                    }
                    
                    cube_frame *BlockCube = GameState->DebugBlockFrames + GameState->DebugBlockFrameCount++;
                    AddCubeWireframe(BlockCube, RenderPos, Size-0.1f, Color);
                }
            }
        }
        
        DXResources->DrawTriangles(GameState->DebugBlockFrames->Vertices, 
                                    CubeFrameVertexCount*GameState->DebugBlockFrameCount);
        DXResources->SetTransformations(v3{});
    }
    
    DXResources->SwapChain->Present(0, 0);
    
    EndTemporaryMemory(&GeneratorMemory);
    CheckMemoryArena(&GameState->WorldArena);
    CheckMemoryArena(&TranState->TranArena);
    
    
    real64 TimeToRender = Clock.GetSecondsElapsed();
    
    
    // logger::PerfPrint("Avg poligonise: %f", GameState->AvgPoligoniseTime * 1000.0);
    
    // logger::PerfPrint("Generate density time: %f", TimeGenerateDensity * 1000.0);
    // logger::PerfPrint("Right click time: %f", TimeRightClick * 1000.0);
    // logger::PerfPrint("Generate render: %f for %d blocks", TimeGenerateRender * 1000.0, RenderCount);
    // logger::PerfPrint("Add to render time: %f", TimeAddToRender * 1000.0);
    // logger::PerfPrint("Render time: %f", TimeToRender * 1000.0);
    
}

void
FreeGameState(game_memory *Memory)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    terrain *Terrain = &GameState->Terrain;
    transient_state *TranState = (transient_state *)Memory->TransientStorage;
    memory_arena *TranArena = &TranState->TranArena;
    
    temporary_memory ClearupMemory = BeginTemporaryMemory(TranArena);
    
    if(GameState->Initialized)
    {
        SaveTerrain(TranArena, Terrain, &GameState->Session);
        for(size_t BlockIndex = 0; 
            BlockIndex < Terrain->PoligonisedBlockCount; 
            BlockIndex++)
        {
            terrain_block_model Block = Terrain->PoligonisedBlocks[BlockIndex];
            if(Block.Buffer)
            {
                Block.Buffer->Release();
            }
        }
    }
    
    GameState->RenderState.Camera.Release();
    GameState->RenderState.DXResources.Release();
    
    
    EndTemporaryMemory(&ClearupMemory);
    CheckMemoryArena(&TranState->TranArena);
}



